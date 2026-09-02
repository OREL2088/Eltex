#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stddef.h>
#include <time.h>

#include "shared.h"

volatile sig_atomic_t stop_requested = 0;

static void handle_signal(int signal_number)
{
    (void)signal_number;
    stop_requested = 1;
}

int install_signal_handlers(void)
{
    struct sigaction action = {0};

    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);
    return sigaction(SIGINT, &action, NULL) == -1 ||
           sigaction(SIGTERM, &action, NULL) == -1 ? -1 : 0;
}

int lock_semaphore(sem_t *semaphore)
{
    while (sem_wait(semaphore) == -1) {
        if (errno != EINTR || stop_requested)
            return -1;
    }
    return 0;
}

int unlock_semaphore(sem_t *semaphore)
{
    return sem_post(semaphore);
}

void sleep_milliseconds(long milliseconds)
{
    struct timespec delay = {
        .tv_sec = milliseconds / 1000,
        .tv_nsec = milliseconds % 1000 * 1000000L
    };

    while (!stop_requested && nanosleep(&delay, &delay) == -1 &&
           errno == EINTR) {
    }
}

size_t align_offset(size_t offset)
{
    const size_t alignment = _Alignof(max_align_t);

    return (offset + alignment - 1U) / alignment * alignment;
}

int validate_shared_memory(const struct shared_header *header)
{
    return header->magic == SHARED_MAGIC &&
           header->version == SHARED_VERSION &&
           header->memory_size == SHARED_MEMORY_SIZE &&
           header->used_size >= align_offset(sizeof(*header)) &&
           header->used_size <= header->memory_size;
}
