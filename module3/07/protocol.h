#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 5000U
#define MAX_NAME_LENGTH 31U
#define MAX_TEXT_LENGTH 4096U
#define MAX_FILE_NAME_LENGTH 255U
#define FILE_CHUNK_SIZE 32768U
#define FRAME_HEADER_SIZE 12U
#define MAX_PAYLOAD_SIZE 65536U
#define MAX_FRAME_SIZE (FRAME_HEADER_SIZE + MAX_NAME_LENGTH + MAX_PAYLOAD_SIZE)

enum frame_type {
    FRAME_HELLO = 1,
    FRAME_CHAT,
    FRAME_JOIN,
    FRAME_LEAVE,
    FRAME_FILE_BEGIN,
    FRAME_FILE_CHUNK,
    FRAME_FILE_END
};

struct frame_view {
    enum frame_type type;
    const unsigned char *name;
    uint16_t name_length;
    const unsigned char *payload;
    uint32_t payload_length;
};

int parse_port(const char *text, unsigned int *port);
int valid_user_name(const char *name);
int valid_file_name(const unsigned char *name, size_t length);
int encode_frame(unsigned char *destination, size_t capacity,
                 enum frame_type type, const void *name, uint16_t name_length,
                 const void *payload, uint32_t payload_length,
                 size_t *encoded_length);
int decode_frame(const unsigned char *data, size_t length,
                 struct frame_view *frame, size_t *consumed);
int write_all(int fd, const void *data, size_t length);

#endif
