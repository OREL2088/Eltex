#define _POSIX_C_SOURCE 200809L

#include "protocol.h"

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_TRANSFERS 128

struct transfer {
    char sender[MAX_NAME_LENGTH + 1U];
    char path[512];
    FILE *file;
};

struct outgoing_file {
    char path[512];
    FILE *file;
};

static volatile sig_atomic_t stopped;
static struct transfer transfers[MAX_TRANSFERS];
static unsigned char input[MAX_FRAME_SIZE];
static size_t input_length;

static void on_signal(int number)
{
    (void)number;
    stopped = 1;
}

static int send_frame(int fd, enum frame_type type,
                      const void *name, uint16_t name_length,
                      const void *payload, uint32_t payload_length)
{
    unsigned char data[MAX_FRAME_SIZE];
    size_t length;

    if (encode_frame(data, sizeof(data), type, name, name_length,
                     payload, payload_length, &length) == -1)
        return -1;
    return write_all(fd, data, length);
}

static struct transfer *find_transfer(const struct frame_view *frame)
{
    for (size_t i = 0; i < MAX_TRANSFERS; ++i) {
        if (transfers[i].file != NULL &&
            strlen(transfers[i].sender) == frame->name_length &&
            memcmp(transfers[i].sender, frame->name, frame->name_length) == 0)
            return &transfers[i];
    }
    return NULL;
}

static int open_received_file(struct transfer *transfer,
                              const struct frame_view *frame)
{
    char file_name[MAX_FILE_NAME_LENGTH + 1U];
    char sender[MAX_NAME_LENGTH + 1U];

    memcpy(file_name, frame->payload, frame->payload_length);
    file_name[frame->payload_length] = '\0';
    strcpy(sender, transfer->sender);

    for (unsigned int suffix = 0; suffix < 10000U; ++suffix) {
        int count;
        int file_fd;

        if (suffix == 0U)
            count = snprintf(transfer->path, sizeof(transfer->path),
                             "received_%s_%s", sender, file_name);
        else
            count = snprintf(transfer->path, sizeof(transfer->path),
                             "received_%s_%u_%s", sender, suffix, file_name);
        if (count < 0 || (size_t)count >= sizeof(transfer->path))
            return -1;

        file_fd = open(transfer->path, O_WRONLY | O_CREAT | O_EXCL, 0644);
        if (file_fd != -1) {
            transfer->file = fdopen(file_fd, "wb");
            if (transfer->file == NULL) {
                close(file_fd);
                unlink(transfer->path);
                return -1;
            }
            return 0;
        }
        if (errno != EEXIST)
            return -1;
    }
    return -1;
}

static void abort_transfers(const struct frame_view *frame)
{
    for (size_t i = 0; i < MAX_TRANSFERS; ++i) {
        if (transfers[i].file != NULL &&
            strlen(transfers[i].sender) == frame->name_length &&
            memcmp(transfers[i].sender, frame->name, frame->name_length) == 0) {
            fclose(transfers[i].file);
            fprintf(stderr, "\nFile transfer interrupted: %s\n",
                    transfers[i].path);
            memset(&transfers[i], 0, sizeof(transfers[i]));
        }
    }
}

static int handle_frame(const struct frame_view *frame)
{
    struct transfer *transfer;

    if (frame->name_length == 0U || frame->name_length > MAX_NAME_LENGTH)
        return -1;
    switch (frame->type) {
    case FRAME_JOIN:
        printf("\n*** %.*s joined the chat ***\n", frame->name_length,
               (const char *)frame->name);
        break;
    case FRAME_LEAVE:
        printf("\n*** %.*s left the chat ***\n", frame->name_length,
               (const char *)frame->name);
        abort_transfers(frame);
        break;
    case FRAME_CHAT:
        printf("\n%.*s: %.*s\n", frame->name_length,
               (const char *)frame->name, (int)frame->payload_length,
               (const char *)frame->payload);
        break;
    case FRAME_FILE_BEGIN:
        if (!valid_file_name(frame->payload, frame->payload_length) ||
            find_transfer(frame) != NULL)
            return -1;
        transfer = NULL;
        for (size_t i = 0; i < MAX_TRANSFERS; ++i) {
            if (transfers[i].file == NULL) {
                transfer = &transfers[i];
                break;
            }
        }
        if (transfer == NULL)
            return -1;
        memcpy(transfer->sender, frame->name, frame->name_length);
        transfer->sender[frame->name_length] = '\0';
        if (open_received_file(transfer, frame) == -1)
            return -1;
        printf("\nReceiving file from %s: %s\n",
               transfer->sender, transfer->path);
        break;
    case FRAME_FILE_CHUNK:
        transfer = find_transfer(frame);
        if (transfer == NULL || fwrite(frame->payload, 1,
                                      frame->payload_length,
                                      transfer->file) != frame->payload_length)
            return -1;
        break;
    case FRAME_FILE_END:
        transfer = find_transfer(frame);
        if (transfer == NULL || fclose(transfer->file) == EOF)
            return -1;
        printf("\nFile saved: %s\n", transfer->path);
        memset(transfer, 0, sizeof(*transfer));
        break;
    default:
        return -1;
    }
    printf("> ");
    fflush(stdout);
    return 0;
}

static int read_server(int fd)
{
    ssize_t count;

    do {
        count = recv(fd, input + input_length,
                     sizeof(input) - input_length, 0);
    } while (count == -1 && errno == EINTR);
    if (count <= 0)
        return -1;

    input_length += (size_t)count;
    while (input_length != 0U) {
        struct frame_view frame;
        size_t consumed;
        int status = decode_frame(input, input_length, &frame, &consumed);

        if (status == 0)
            break;
        if (status == -1 || handle_frame(&frame) == -1)
            return -1;
        memmove(input, input + consumed, input_length - consumed);
        input_length -= consumed;
    }
    return input_length == sizeof(input) ? -1 : 0;
}

static int start_file(int fd, struct outgoing_file *outgoing, const char *path)
{
    char path_copy[sizeof(outgoing->path)];
    const char *file_name;
    size_t name_length;

    if (outgoing->file != NULL) {
        fprintf(stderr, "A file is already being sent.\n");
        return 0;
    }
    if (strlen(path) >= sizeof(path_copy)) {
        fprintf(stderr, "File path is too long.\n");
        return 0;
    }
    outgoing->file = fopen(path, "rb");
    if (outgoing->file == NULL) {
        perror(path);
        return 0;
    }

    strcpy(path_copy, path);
    file_name = basename(path_copy);
    name_length = strlen(file_name);
    if (!valid_file_name((const unsigned char *)file_name, name_length)) {
        fprintf(stderr, "Invalid file name.\n");
        fclose(outgoing->file);
        outgoing->file = NULL;
        return 0;
    }
    strcpy(outgoing->path, path);
    if (send_frame(fd, FRAME_FILE_BEGIN, NULL, 0U,
                   file_name, (uint32_t)name_length) == -1)
        return -1;
    printf("Sending file: %s\n> ", path);
    fflush(stdout);
    return 0;
}

static int send_file_part(int fd, struct outgoing_file *outgoing)
{
    unsigned char chunk[FILE_CHUNK_SIZE];
    size_t length = fread(chunk, 1, sizeof(chunk), outgoing->file);

    if (length != 0U && send_frame(fd, FRAME_FILE_CHUNK, NULL, 0U,
                                  chunk, (uint32_t)length) == -1)
        return -1;
    if (length < sizeof(chunk)) {
        if (ferror(outgoing->file)) {
            perror(outgoing->path);
            return -1;
        }
        if (send_frame(fd, FRAME_FILE_END, NULL, 0U, NULL, 0U) == -1)
            return -1;
        fclose(outgoing->file);
        outgoing->file = NULL;
    }
    return 0;
}

static int handle_stdin(int fd, struct outgoing_file *outgoing)
{
    char line[MAX_TEXT_LENGTH + 2U];
    size_t length;

    if (fgets(line, sizeof(line), stdin) == NULL)
        return 1;
    length = strlen(line);
    if (length != 0U && line[length - 1U] == '\n')
        line[--length] = '\0';
    else if (length > MAX_TEXT_LENGTH) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
        fprintf(stderr, "Message is too long (maximum %u bytes).\n> ",
                MAX_TEXT_LENGTH);
        return 0;
    }

    if (strncmp(line, "/file ", 6) == 0 && line[6] != '\0')
        return start_file(fd, outgoing, line + 6);
    if (strcmp(line, "/quit") == 0)
        return 1;
    if (length != 0U && send_frame(fd, FRAME_CHAT, NULL, 0U,
                                  line, (uint32_t)length) == -1)
        return -1;
    printf("> ");
    fflush(stdout);
    return 0;
}

static int connect_server(const char *host, unsigned int port)
{
    struct addrinfo hints = {0};
    struct addrinfo *addresses = NULL;
    struct addrinfo *current;
    char service[6];
    int fd = -1;
    int status;

    snprintf(service, sizeof(service), "%u", port);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    status = getaddrinfo(host, service, &hints, &addresses);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }
    for (current = addresses; current != NULL; current = current->ai_next) {
        fd = socket(current->ai_family, current->ai_socktype,
                    current->ai_protocol);
        if (fd != -1 && connect(fd, current->ai_addr,
                                current->ai_addrlen) == 0)
            break;
        if (fd != -1)
            close(fd);
        fd = -1;
    }
    freeaddrinfo(addresses);
    return fd;
}

int main(int argc, char **argv)
{
    const char *host = DEFAULT_HOST;
    unsigned int port = DEFAULT_PORT;
    struct outgoing_file outgoing = {0};
    int fd;
    int result = EXIT_SUCCESS;

    if (argc < 2 || argc > 4 || !valid_user_name(argv[1]) ||
        (argc == 4 && parse_port(argv[3], &port) == -1)) {
        fprintf(stderr, "Usage: %s NAME [HOST [PORT]]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc >= 3)
        host = argv[2];
    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    signal(SIGPIPE, SIG_IGN);

    fd = connect_server(host, port);
    if (fd == -1) {
        perror("connect");
        return EXIT_FAILURE;
    }
    if (send_frame(fd, FRAME_HELLO, argv[1], (uint16_t)strlen(argv[1]),
                   NULL, 0U) == -1) {
        perror("send hello");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Connected as %s. Commands: /file PATH, /quit\n> ", argv[1]);
    fflush(stdout);
    while (!stopped) {
        struct pollfd descriptors[2] = {
            {.fd = fd, .events = POLLIN},
            {.fd = STDIN_FILENO, .events = POLLIN}
        };
        int ready;

        if (outgoing.file != NULL)
            descriptors[0].events |= POLLOUT;
        ready = poll(descriptors, 2, -1);
        if (ready == -1) {
            if (errno == EINTR)
                continue;
            perror("poll");
            result = EXIT_FAILURE;
            break;
        }
        if ((descriptors[0].revents & (POLLERR | POLLHUP | POLLNVAL)) ||
            ((descriptors[0].revents & POLLIN) && read_server(fd) == -1)) {
            fprintf(stderr, "\nServer disconnected.\n");
            result = EXIT_FAILURE;
            break;
        }
        if ((descriptors[0].revents & POLLOUT) &&
            send_file_part(fd, &outgoing) == -1) {
            result = EXIT_FAILURE;
            break;
        }
        if (descriptors[1].revents & (POLLIN | POLLHUP)) {
            int status = handle_stdin(fd, &outgoing);
            if (status != 0) {
                if (status < 0)
                    result = EXIT_FAILURE;
                break;
            }
        }
    }

    if (outgoing.file != NULL)
        fclose(outgoing.file);
    for (size_t i = 0; i < MAX_TRANSFERS; ++i) {
        if (transfers[i].file != NULL)
            fclose(transfers[i].file);
    }
    close(fd);
    return result;
}
