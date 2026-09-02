#define _POSIX_C_SOURCE 200809L

#include "protocol.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROTOCOL_MAGIC 0x454c5437U
#define PROTOCOL_VERSION 1U

static void put_u16(unsigned char *p, uint16_t value)
{
    p[0] = (unsigned char)(value >> 8);
    p[1] = (unsigned char)value;
}

static void put_u32(unsigned char *p, uint32_t value)
{
    p[0] = (unsigned char)(value >> 24);
    p[1] = (unsigned char)(value >> 16);
    p[2] = (unsigned char)(value >> 8);
    p[3] = (unsigned char)value;
}

static uint16_t get_u16(const unsigned char *p)
{
    return (uint16_t)((uint16_t)p[0] << 8 | p[1]);
}

static uint32_t get_u32(const unsigned char *p)
{
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 |
           (uint32_t)p[2] << 8 | p[3];
}

int parse_port(const char *text, unsigned int *port)
{
    char *end = NULL;
    unsigned long value;

    errno = 0;
    value = strtoul(text, &end, 10);
    if (errno != 0 || text[0] == '\0' || *end != '\0' ||
        value == 0UL || value > 65535UL)
        return -1;
    *port = (unsigned int)value;
    return 0;
}

int valid_user_name(const char *name)
{
    size_t length = strlen(name);

    if (length == 0U || length > MAX_NAME_LENGTH)
        return 0;
    for (size_t i = 0; i < length; ++i) {
        unsigned char c = (unsigned char)name[i];
        if (c < 33U || c == 127U)
            return 0;
    }
    return 1;
}

int valid_file_name(const unsigned char *name, size_t length)
{
    if (length == 0U || length > MAX_FILE_NAME_LENGTH)
        return 0;
    for (size_t i = 0; i < length; ++i) {
        if (name[i] < 32U || name[i] == 127U ||
            name[i] == '/' || name[i] == '\\')
            return 0;
    }
    return !(length == 1U && name[0] == '.') &&
           !(length == 2U && name[0] == '.' && name[1] == '.');
}

int encode_frame(unsigned char *destination, size_t capacity,
                 enum frame_type type, const void *name, uint16_t name_length,
                 const void *payload, uint32_t payload_length,
                 size_t *encoded_length)
{
    size_t total = FRAME_HEADER_SIZE + (size_t)name_length + payload_length;

    if (name_length > MAX_NAME_LENGTH || payload_length > MAX_PAYLOAD_SIZE ||
        total > capacity || (name_length != 0U && name == NULL) ||
        (payload_length != 0U && payload == NULL))
        return -1;
    put_u32(destination, PROTOCOL_MAGIC);
    destination[4] = PROTOCOL_VERSION;
    destination[5] = (unsigned char)type;
    put_u16(destination + 6, name_length);
    put_u32(destination + 8, payload_length);
    if (name_length != 0U)
        memcpy(destination + FRAME_HEADER_SIZE, name, name_length);
    if (payload_length != 0U)
        memcpy(destination + FRAME_HEADER_SIZE + name_length,
               payload, payload_length);
    *encoded_length = total;
    return 0;
}

int decode_frame(const unsigned char *data, size_t length,
                 struct frame_view *frame, size_t *consumed)
{
    size_t total;

    *consumed = 0U;
    if (length < FRAME_HEADER_SIZE)
        return 0;
    if (get_u32(data) != PROTOCOL_MAGIC || data[4] != PROTOCOL_VERSION ||
        data[5] < FRAME_HELLO || data[5] > FRAME_FILE_END)
        return -1;
    frame->name_length = get_u16(data + 6);
    frame->payload_length = get_u32(data + 8);
    if (frame->name_length > MAX_NAME_LENGTH ||
        frame->payload_length > MAX_PAYLOAD_SIZE)
        return -1;
    total = FRAME_HEADER_SIZE + (size_t)frame->name_length +
            frame->payload_length;
    if (length < total)
        return 0;
    frame->type = (enum frame_type)data[5];
    frame->name = data + FRAME_HEADER_SIZE;
    frame->payload = frame->name + frame->name_length;
    *consumed = total;
    return 1;
}

int write_all(int fd, const void *data, size_t length)
{
    const unsigned char *p = data;

    while (length != 0U) {
        ssize_t written = write(fd, p, length);
        if (written > 0) {
            p += written;
            length -= (size_t)written;
        } else if (written == -1 && errno == EINTR) {
            continue;
        } else {
            return -1;
        }
    }
    return 0;
}
