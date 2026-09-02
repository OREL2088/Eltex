#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "shared.h"

static int open_resources(int *memory_fd, sem_t **semaphore,
                          struct shared_header **header)
{
    *memory_fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0600);
    if (*memory_fd == -1) {
        if (errno == ENOENT)
            fprintf(stderr, "Producer is not running.\n");
        else
            perror("shm_open");
        return -1;
    }

    *semaphore = sem_open(SEMAPHORE_NAME, 0);
    if (*semaphore == SEM_FAILED) {
        if (errno == ENOENT)
            fprintf(stderr, "Producer semaphore is not available.\n");
        else
            perror("sem_open");
        close(*memory_fd);
        return -1;
    }

    *header = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE,
                   MAP_SHARED, *memory_fd, 0);
    if (*header == MAP_FAILED) {
        perror("mmap");
        sem_close(*semaphore);
        close(*memory_fd);
        return -1;
    }
    return 0;
}

static struct data_block *find_unprocessed_block(struct shared_header *header)
{
    char *memory = (char *)header;
    size_t offset = header->first_offset;

    while (offset != 0U) {
        struct data_block *block;

        if (offset < align_offset(sizeof(*header)) ||
            offset > header->used_size ||
            header->used_size - offset < sizeof(struct data_block))
            return NULL;
        block = (struct data_block *)(memory + offset);
        if (block->count > MAX_ARRAY_LENGTH ||
            (size_t)block->count * sizeof(int) >
                header->used_size - offset - sizeof(*block))
            return NULL;
        if (block->count != 0U)
            return block;
        if (block->next_offset != 0U && block->next_offset <= offset)
            return NULL;
        offset = block->next_offset;
    }
    return NULL;
}

static int process_one_block(struct shared_header *header, sem_t *semaphore,
                             unsigned int *number, unsigned int *count,
                             int *minimum, int *maximum)
{
    struct data_block *block;

    if (lock_semaphore(semaphore) == -1)
        return -1;
    if (header->shutting_down) {
        if (header->active_consumers > 0U)
            --header->active_consumers;
        unlock_semaphore(semaphore);
        return 0;
    }

    block = find_unprocessed_block(header);
    if (block == NULL) {
        if (unlock_semaphore(semaphore) == -1)
            return -1;
        return 2;
    }

    *count = block->count;
    *minimum = block->values[0];
    *maximum = block->values[0];
    for (unsigned int i = 1; i < block->count; ++i) {
        if (block->values[i] < *minimum)
            *minimum = block->values[i];
        if (block->values[i] > *maximum)
            *maximum = block->values[i];
    }
    block->count = 0U;
    ++header->processed_blocks;
    *number = header->processed_blocks;
    if (unlock_semaphore(semaphore) == -1)
        return -1;
    return 1;
}

static void unregister_consumer(struct shared_header *header, sem_t *semaphore,
                                int *registered)
{
    if (!*registered)
        return;
    if (lock_semaphore(semaphore) == 0) {
        if (header->active_consumers > 0U)
            --header->active_consumers;
        unlock_semaphore(semaphore);
    }
    *registered = 0;
}

int main(int argc, char *argv[])
{
    int memory_fd = -1;
    sem_t *semaphore = SEM_FAILED;
    struct shared_header *header = MAP_FAILED;
    int registered = 0;
    int result = EXIT_SUCCESS;

    if (argc != 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (install_signal_handlers() == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    if (open_resources(&memory_fd, &semaphore, &header) == -1)
        return EXIT_FAILURE;

    if (lock_semaphore(semaphore) == -1) {
        perror("sem_wait");
        munmap(header, SHARED_MEMORY_SIZE);
        sem_close(semaphore);
        close(memory_fd);
        return EXIT_FAILURE;
    }
    if (!validate_shared_memory(header)) {
        fprintf(stderr, "Invalid shared memory contents.\n");
        unlock_semaphore(semaphore);
        munmap(header, SHARED_MEMORY_SIZE);
        sem_close(semaphore);
        close(memory_fd);
        return EXIT_FAILURE;
    }
    if (!header->shutting_down) {
        ++header->active_consumers;
        registered = 1;
    }
    if (unlock_semaphore(semaphore) == -1) {
        perror("sem_post");
        munmap(header, SHARED_MEMORY_SIZE);
        sem_close(semaphore);
        close(memory_fd);
        return EXIT_FAILURE;
    }

    printf("Consumer %ld started.\n", (long)getpid());
    while (!stop_requested && registered) {
        unsigned int number;
        unsigned int count;
        int minimum;
        int maximum;
        int status = process_one_block(header, semaphore, &number, &count,
                                       &minimum, &maximum);

        if (status == -1) {
            if (!stop_requested)
                perror("semaphore operation");
            result = EXIT_FAILURE;
            break;
        }
        if (status == 0) {
            registered = 0;
            break;
        }
        if (status == 1) {
            printf("Consumer %ld: array %u, elements %u, min = %d, max = %d\n",
                   (long)getpid(), number, count, minimum, maximum);
            fflush(stdout);
        }
        sleep_milliseconds(CONSUMER_DELAY_MS);
    }

    unregister_consumer(header, semaphore, &registered);
    if (munmap(header, SHARED_MEMORY_SIZE) == -1) {
        perror("munmap");
        result = EXIT_FAILURE;
    }
    if (sem_close(semaphore) == -1) {
        perror("sem_close");
        result = EXIT_FAILURE;
    }
    if (close(memory_fd) == -1) {
        perror("close");
        result = EXIT_FAILURE;
    }
    return result;
}
