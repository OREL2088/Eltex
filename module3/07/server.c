#define _POSIX_C_SOURCE 200809L

#include "protocol.h"

#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CLIENTS 128
#define MAX_EVENTS 64

struct client {
    int fd;
    int registered;
    int sending_file;
    char name[MAX_NAME_LENGTH + 1U];
    unsigned char input[MAX_FRAME_SIZE];
    size_t input_length;
};

static volatile sig_atomic_t stopped;
static struct client clients[MAX_CLIENTS];
static int epoll_fd;

static void on_signal(int number)
{
    (void)number;
    stopped = 1;
}

static int name_is_free(const unsigned char *name, size_t length)
{
    for (size_t i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].registered && strlen(clients[i].name) == length &&
            memcmp(clients[i].name, name, length) == 0)
            return 0;
    }
    return 1;
}

static void broadcast(enum frame_type type, const char *name,
                      const void *payload, uint32_t payload_length,
                      const struct client *except)
{
    unsigned char data[MAX_FRAME_SIZE];
    size_t length;

    if (encode_frame(data, sizeof(data), type, name,
                     name == NULL ? 0U : (uint16_t)strlen(name),
                     payload, payload_length, &length) == -1)
        return;
    for (size_t i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].registered && &clients[i] != except &&
            write_all(clients[i].fd, data, length) == -1)
            shutdown(clients[i].fd, SHUT_RDWR);
    }
}

static void disconnect_client(struct client *client)
{
    char name[MAX_NAME_LENGTH + 1U];
    int registered = client->registered;

    strcpy(name, client->name);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->fd, NULL);
    close(client->fd);
    memset(client, 0, sizeof(*client));
    client->fd = -1;
    if (registered) {
        printf("- %s\n", name);
        broadcast(FRAME_LEAVE, name, NULL, 0U, NULL);
    }
}

static int handle_frame(struct client *client, const struct frame_view *frame)
{
    if (!client->registered) {
        if (frame->type != FRAME_HELLO || frame->name_length == 0U ||
            frame->payload_length != 0U ||
            !name_is_free(frame->name, frame->name_length))
            return -1;
        memcpy(client->name, frame->name, frame->name_length);
        client->name[frame->name_length] = '\0';
        if (strlen(client->name) != frame->name_length ||
            !valid_user_name(client->name))
            return -1;
        client->registered = 1;
        printf("+ %s\n", client->name);
        broadcast(FRAME_JOIN, client->name, NULL, 0U, client);
        return 0;
    }

    if (frame->name_length != 0U)
        return -1;
    switch (frame->type) {
    case FRAME_CHAT:
        if (frame->payload_length == 0U ||
            frame->payload_length > MAX_TEXT_LENGTH)
            return -1;
        break;
    case FRAME_FILE_BEGIN:
        if (client->sending_file ||
            !valid_file_name(frame->payload, frame->payload_length))
            return -1;
        client->sending_file = 1;
        break;
    case FRAME_FILE_CHUNK:
        if (!client->sending_file || frame->payload_length == 0U ||
            frame->payload_length > FILE_CHUNK_SIZE)
            return -1;
        break;
    case FRAME_FILE_END:
        if (!client->sending_file || frame->payload_length != 0U)
            return -1;
        client->sending_file = 0;
        break;
    default:
        return -1;
    }
    broadcast(frame->type, client->name, frame->payload,
              frame->payload_length, NULL);
    return 0;
}

static int read_client(struct client *client)
{
    ssize_t count;

    do {
        count = recv(client->fd, client->input + client->input_length,
                     sizeof(client->input) - client->input_length, 0);
    } while (count == -1 && errno == EINTR);
    if (count <= 0)
        return -1;

    client->input_length += (size_t)count;
    while (client->input_length != 0U) {
        struct frame_view frame;
        size_t consumed;
        int status = decode_frame(client->input, client->input_length,
                                  &frame, &consumed);

        if (status == 0)
            break;
        if (status == -1 || handle_frame(client, &frame) == -1)
            return -1;
        memmove(client->input, client->input + consumed,
                client->input_length - consumed);
        client->input_length -= consumed;
    }
    return client->input_length == sizeof(client->input) ? -1 : 0;
}

static void accept_client(int listener)
{
    struct epoll_event event;
    struct client *slot = NULL;
    int fd = accept(listener, NULL, NULL);

    if (fd == -1) {
        if (errno != EINTR)
            perror("accept");
        return;
    }
    for (size_t i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].fd == -1) {
            slot = &clients[i];
            break;
        }
    }
    if (slot == NULL) {
        close(fd);
        return;
    }

    memset(slot, 0, sizeof(*slot));
    slot->fd = fd;
    memset(&event, 0, sizeof(event));
    event.events = EPOLLIN | EPOLLRDHUP;
    event.data.ptr = slot;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
        close(fd);
        slot->fd = -1;
    }
}

static int create_listener(unsigned int port)
{
    struct sockaddr_in address;
    int enabled = 1;
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1)
        return -1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enabled,
                   sizeof(enabled)) == -1) {
        close(fd);
        return -1;
    }
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons((uint16_t)port);
    if (bind(fd, (struct sockaddr *)&address, sizeof(address)) == -1 ||
        listen(fd, SOMAXCONN) == -1) {
        close(fd);
        return -1;
    }
    return fd;
}

int main(int argc, char **argv)
{
    unsigned int port = DEFAULT_PORT;
    struct epoll_event listener_event;
    struct epoll_event events[MAX_EVENTS];
    int listener;

    if (argc > 2 || (argc == 2 && parse_port(argv[1], &port) == -1)) {
        fprintf(stderr, "Usage: %s [PORT]\n", argv[0]);
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < MAX_CLIENTS; ++i)
        clients[i].fd = -1;
    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    signal(SIGPIPE, SIG_IGN);

    listener = create_listener(port);
    epoll_fd = epoll_create1(0);
    if (listener == -1 || epoll_fd == -1) {
        perror("server initialization");
        if (listener != -1)
            close(listener);
        return EXIT_FAILURE;
    }
    memset(&listener_event, 0, sizeof(listener_event));
    listener_event.events = EPOLLIN;
    listener_event.data.ptr = NULL;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listener, &listener_event) == -1) {
        perror("epoll_ctl");
        close(listener);
        close(epoll_fd);
        return EXIT_FAILURE;
    }

    printf("Server listens on port %u\n", port);
    while (!stopped) {
        int ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        if (ready == -1) {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < ready; ++i) {
            struct client *client = events[i].data.ptr;

            if (client == NULL)
                accept_client(listener);
            else if ((events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) ||
                     read_client(client) == -1)
                disconnect_client(client);
        }
    }
    for (size_t i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].fd != -1)
            disconnect_client(&clients[i]);
    }
    close(listener);
    close(epoll_fd);
    return EXIT_SUCCESS;
}
