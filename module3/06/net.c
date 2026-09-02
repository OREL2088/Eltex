#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "chat.h"

#define PROTOCOL_MAGIC 0x454c5458U
#define PROTOCOL_VERSION 1U
#define HEADER_LENGTH 17U

static void put_u16(unsigned char *buffer, uint16_t value)
{
    buffer[0] = (unsigned char)(value >> 8);
    buffer[1] = (unsigned char)value;
}

static void put_u32(unsigned char *buffer, uint32_t value)
{
    buffer[0] = (unsigned char)(value >> 24);
    buffer[1] = (unsigned char)(value >> 16);
    buffer[2] = (unsigned char)(value >> 8);
    buffer[3] = (unsigned char)value;
}

static void put_u64(unsigned char *buffer, uint64_t value)
{
    for (unsigned int i = 0; i < 8U; ++i)
        buffer[i] = (unsigned char)(value >> (56U - i * 8U));
}

static uint16_t get_u16(const unsigned char *buffer)
{
    return (uint16_t)((uint16_t)buffer[0] << 8 | buffer[1]);
}

static uint32_t get_u32(const unsigned char *buffer)
{
    return (uint32_t)buffer[0] << 24 |
           (uint32_t)buffer[1] << 16 |
           (uint32_t)buffer[2] << 8 |
           buffer[3];
}

static uint64_t get_u64(const unsigned char *buffer)
{
    uint64_t value = 0;

    for (unsigned int i = 0; i < 8U; ++i)
        value = value << 8 | buffer[i];
    return value;
}

int create_socket(unsigned int port)
{
    struct sockaddr_in local_address;
    int enabled = 1;
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_fd == -1) {
        perror("socket");
        return -1;
    }
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,
                   &enabled, sizeof(enabled)) == -1) {
        perror("setsockopt(SO_REUSEADDR)");
        close(socket_fd);
        return -1;
    }
    if (setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST,
                   &enabled, sizeof(enabled)) == -1) {
        perror("setsockopt(SO_BROADCAST)");
        close(socket_fd);
        return -1;
    }

    memset(&local_address, 0, sizeof(local_address));
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons((uint16_t)port);
    if (bind(socket_fd, (struct sockaddr *)&local_address,
             sizeof(local_address)) == -1) {
        perror("bind");
        close(socket_fd);
        return -1;
    }
    return socket_fd;
}

int make_destination(const char *address, unsigned int port,
                     struct sockaddr_in *destination)
{
    memset(destination, 0, sizeof(*destination));
    destination->sin_family = AF_INET;
    destination->sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, address, &destination->sin_addr) != 1) {
        fprintf(stderr, "Invalid IPv4 broadcast address: %s\n", address);
        return -1;
    }
    return 0;
}

uint64_t make_client_id(void)
{
    struct timespec realtime = {0, 0};
    struct timespec monotonic = {0, 0};
    uint64_t id;

    clock_gettime(CLOCK_REALTIME, &realtime);
    clock_gettime(CLOCK_MONOTONIC, &monotonic);
    id = (uint64_t)realtime.tv_sec << 32;
    id ^= (uint64_t)realtime.tv_nsec << 1;
    id ^= (uint64_t)monotonic.tv_nsec << 17;
    id ^= (uint64_t)(unsigned long)getpid();
    return id;
}

int send_message(int socket_fd, const struct sockaddr_in *destination,
                 enum message_type type, uint64_t client_id,
                 const char *name, const char *text, size_t text_length)
{
    unsigned char packet[MAX_PACKET_LENGTH];
    size_t name_length = strlen(name);
    size_t packet_length;
    ssize_t sent;

    if (name_length == 0U || name_length > MAX_NAME_LENGTH ||
        text_length > MAX_MESSAGE_LENGTH)
        return -1;

    put_u32(packet, PROTOCOL_MAGIC);
    packet[4] = PROTOCOL_VERSION;
    packet[5] = (unsigned char)type;
    put_u64(packet + 6, client_id);
    packet[14] = (unsigned char)name_length;
    put_u16(packet + 15, (uint16_t)text_length);
    memcpy(packet + HEADER_LENGTH, name, name_length);
    if (text_length != 0U)
        memcpy(packet + HEADER_LENGTH + name_length, text, text_length);
    packet_length = HEADER_LENGTH + name_length + text_length;

    sent = sendto(socket_fd, packet, packet_length, 0,
                  (const struct sockaddr *)destination, sizeof(*destination));
    if (sent == -1) {
        perror("sendto");
        return -1;
    }
    if ((size_t)sent != packet_length) {
        fprintf(stderr, "Incomplete UDP datagram sent\n");
        return -1;
    }
    return 0;
}

int receive_message(int socket_fd, struct message *message)
{
    unsigned char packet[MAX_PACKET_LENGTH + 1U];
    size_t name_length;
    size_t text_length;
    size_t expected_length;
    ssize_t received = recvfrom(socket_fd, packet, sizeof(packet), 0, NULL, NULL);

    if (received == -1) {
        if (errno != EINTR)
            perror("recvfrom");
        return -1;
    }
    if ((size_t)received < HEADER_LENGTH ||
        get_u32(packet) != PROTOCOL_MAGIC ||
        packet[4] != PROTOCOL_VERSION ||
        packet[5] < MESSAGE_JOIN || packet[5] > MESSAGE_LEAVE)
        return 0;

    name_length = packet[14];
    text_length = get_u16(packet + 15);
    expected_length = HEADER_LENGTH + name_length + text_length;
    if (name_length == 0U || name_length > MAX_NAME_LENGTH ||
        text_length > MAX_MESSAGE_LENGTH ||
        expected_length != (size_t)received)
        return 0;
    if (packet[5] != MESSAGE_CHAT && text_length != 0U)
        return 0;

    message->type = (enum message_type)packet[5];
    message->client_id = get_u64(packet + 6);
    memcpy(message->name, packet + HEADER_LENGTH, name_length);
    message->name[name_length] = '\0';
    if (text_length != 0U)
        memcpy(message->text, packet + HEADER_LENGTH + name_length,
               text_length);
    message->text[text_length] = '\0';
    message->text_length = text_length;
    return 1;
}
