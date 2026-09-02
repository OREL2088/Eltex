#ifndef CHAT_H
#define CHAT_H

#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>

#define DEFAULT_BROADCAST_ADDRESS "255.255.255.255"
#define DEFAULT_PORT 5000U
#define MAX_NAME_LENGTH 31U
#define MAX_MESSAGE_LENGTH 512U
#define MAX_PACKET_LENGTH (17U + MAX_NAME_LENGTH + MAX_MESSAGE_LENGTH)

enum message_type {
    MESSAGE_JOIN = 1,
    MESSAGE_CHAT = 2,
    MESSAGE_LEAVE = 3
};

struct message {
    enum message_type type;
    uint64_t client_id;
    char name[MAX_NAME_LENGTH + 1U];
    char text[MAX_MESSAGE_LENGTH + 1U];
    size_t text_length;
};

extern volatile sig_atomic_t stop_requested;

int install_signal_handlers(void);
int create_socket(unsigned int port);
int make_destination(const char *address, unsigned int port,
                     struct sockaddr_in *destination);
uint64_t make_client_id(void);
int send_message(int socket_fd, const struct sockaddr_in *destination,
                 enum message_type type, uint64_t client_id,
                 const char *name, const char *text, size_t text_length);
int receive_message(int socket_fd, struct message *message);

#endif
