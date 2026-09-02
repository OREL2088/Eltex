#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "chat.h"

volatile sig_atomic_t stop_requested = 0;

static void handle_signal(int signal_number)
{
    (void)signal_number;
    stop_requested = 1;
}

int install_signal_handlers(void)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);
    return sigaction(SIGINT, &action, NULL) == -1 ||
           sigaction(SIGTERM, &action, NULL) == -1 ? -1 : 0;
}

static int parse_port(const char *text, unsigned int *port)
{
    char *end;
    unsigned long value;

    errno = 0;
    value = strtoul(text, &end, 10);
    if (errno != 0 || *text == '\0' || *end != '\0' ||
        value == 0UL || value > 65535UL)
        return -1;
    *port = (unsigned int)value;
    return 0;
}

static int valid_name(const char *name)
{
    size_t length = strlen(name);

    if (length == 0U || length > MAX_NAME_LENGTH)
        return 0;
    for (size_t i = 0; i < length; ++i) {
        unsigned char character = (unsigned char)name[i];

        if (character < 32U || character == 127U)
            return 0;
    }
    return 1;
}

static void print_received(const struct message *message)
{
    if (message->type == MESSAGE_JOIN)
        printf("\n*** %s joined the chat ***\n", message->name);
    else if (message->type == MESSAGE_LEAVE)
        printf("\n*** %s left the chat ***\n", message->name);
    else {
        printf("\n%s: ", message->name);
        fwrite(message->text, 1, message->text_length, stdout);
        putchar('\n');
    }
    printf("> ");
    fflush(stdout);
}

static int run_chat(int socket_fd, const struct sockaddr_in *destination,
                    const char *name, uint64_t client_id)
{
    struct pollfd descriptors[2] = {
        {.fd = socket_fd, .events = POLLIN},
        {.fd = STDIN_FILENO, .events = POLLIN}
    };
    int result = 0;

    printf("Connected as %s. Enter a message; Ctrl+D or Ctrl+C exits.\n> ",
           name);
    fflush(stdout);

    while (!stop_requested) {
        int ready = poll(descriptors, 2, -1);

        if (ready == -1) {
            if (errno == EINTR)
                continue;
            perror("poll");
            result = -1;
            break;
        }
        if ((descriptors[0].revents & POLLIN) != 0) {
            struct message message;
            int status = receive_message(socket_fd, &message);

            if (status == -1) {
                if (!stop_requested)
                    result = -1;
                break;
            }
            if (status == 1 && message.client_id != client_id)
                print_received(&message);
        }
        if ((descriptors[1].revents & (POLLIN | POLLHUP)) != 0) {
            char line[MAX_MESSAGE_LENGTH + 2U];
            size_t length;

            if (fgets(line, sizeof(line), stdin) == NULL)
                break;
            length = strlen(line);
            if (length != 0U && line[length - 1U] == '\n') {
                line[--length] = '\0';
            } else if (length > MAX_MESSAGE_LENGTH) {
                int character;

                while ((character = getchar()) != '\n' && character != EOF)
                    ;
                fprintf(stderr, "Message is too long (maximum %u bytes).\n> ",
                        MAX_MESSAGE_LENGTH);
                fflush(stderr);
                continue;
            }
            if (length != 0U &&
                send_message(socket_fd, destination, MESSAGE_CHAT, client_id,
                             name, line, length) == -1) {
                result = -1;
                break;
            }
            printf("> ");
            fflush(stdout);
        }
    }
    return result;
}

int main(int argc, char *argv[])
{
    const char *broadcast_address = DEFAULT_BROADCAST_ADDRESS;
    unsigned int port = DEFAULT_PORT;
    struct sockaddr_in destination;
    uint64_t client_id;
    int socket_fd;
    int result;

    if (argc < 2 || argc > 4) {
        fprintf(stderr, "Usage: %s NAME [BROADCAST_ADDRESS [PORT]]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (!valid_name(argv[1])) {
        fprintf(stderr, "NAME must contain 1 to %u printable bytes.\n",
                MAX_NAME_LENGTH);
        return EXIT_FAILURE;
    }
    if (argc >= 3)
        broadcast_address = argv[2];
    if (argc == 4 && parse_port(argv[3], &port) == -1) {
        fprintf(stderr, "PORT must be an integer from 1 to 65535.\n");
        return EXIT_FAILURE;
    }
    if (install_signal_handlers() == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    if (make_destination(broadcast_address, port, &destination) == -1)
        return EXIT_FAILURE;
    socket_fd = create_socket(port);
    if (socket_fd == -1)
        return EXIT_FAILURE;

    client_id = make_client_id();
    if (send_message(socket_fd, &destination, MESSAGE_JOIN, client_id,
                     argv[1], "", 0U) == -1) {
        close(socket_fd);
        return EXIT_FAILURE;
    }

    result = run_chat(socket_fd, &destination, argv[1], client_id);
    if (send_message(socket_fd, &destination, MESSAGE_LEAVE, client_id,
                     argv[1], "", 0U) == -1)
        result = -1;
    if (close(socket_fd) == -1) {
        perror("close");
        result = -1;
    }
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
