#ifndef SHARED_H
#define SHARED_H

#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/ipc.h>

#define SHARED_MEMORY_SIZE 4096U
#define SHARED_MAGIC 0x454c5458U
#define SHARED_VERSION 1U
#define MAX_ARRAY_LENGTH 16U
#define PRODUCER_DELAY_MS 50L
#define CONSUMER_DELAY_MS 200L

struct shared_header {
    uint32_t magic;
    uint32_t version;
    size_t memory_size;
    size_t first_offset;
    size_t last_offset;
    size_t used_size;
    unsigned int total_blocks;
    unsigned int processed_blocks;
    unsigned int active_consumers;
    int generation_finished;
    int shutting_down;
};

struct data_block {
    size_t next_offset;
    unsigned int count;
    int values[];
};

extern volatile sig_atomic_t stop_requested;

int install_signal_handlers(void);
key_t make_ipc_key(int project_id);
int lock_semaphore(int semaphore_id);
int unlock_semaphore(int semaphore_id);
void sleep_milliseconds(long milliseconds);
size_t align_offset(size_t offset);
int validate_shared_memory(const struct shared_header *header);

#endif
