#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "shared.h"

static int append_random_block(struct shared_header *header)
{
    char *memory = (char *)header;
    size_t offset = align_offset(header->used_size);
    size_t available;
    unsigned int limit;
    unsigned int count;
    struct data_block *block;

    if (offset > header->memory_size ||
        header->memory_size - offset < sizeof(struct data_block) + sizeof(int))
        return 0;

    available = header->memory_size - offset - sizeof(struct data_block);
    limit = (unsigned int)(available / sizeof(int));
    if (limit > MAX_ARRAY_LENGTH)
        limit = MAX_ARRAY_LENGTH;
    if (limit == 0U)
        return 0;

    count = 1U + (unsigned int)rand() % limit;
    block = (struct data_block *)(memory + offset);
    block->next_offset = 0U;
    block->count = count;
    for (unsigned int i = 0; i < count; ++i)
        block->values[i] = rand() % 2001 - 1000;

    if (header->last_offset == 0U)
        header->first_offset = offset;
    else
        ((struct data_block *)(memory + header->last_offset))->next_offset = offset;

    header->last_offset = offset;
    header->used_size = offset + sizeof(*block) + count * sizeof(int);
    ++header->total_blocks;
    return 1;
}

static int create_resources(int *memory_fd, sem_t **semaphore,
                            struct shared_header **header)
{
    *memory_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (*memory_fd == -1) {
        if (errno == EEXIST)
            fprintf(stderr, "Shared memory already exists. Is another producer running?\n");
        else
            perror("shm_open");
        return -1;
    }

    if (ftruncate(*memory_fd, (off_t)SHARED_MEMORY_SIZE) == -1) {
        perror("ftruncate");
        close(*memory_fd);
        shm_unlink(SHARED_MEMORY_NAME);
        return -1;
    }

    *header = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE,
                   MAP_SHARED, *memory_fd, 0);
    if (*header == MAP_FAILED) {
        perror("mmap");
        close(*memory_fd);
        shm_unlink(SHARED_MEMORY_NAME);
        return -1;
    }

    memset(*header, 0, SHARED_MEMORY_SIZE);
    (*header)->magic = SHARED_MAGIC;
    (*header)->version = SHARED_VERSION;
    (*header)->memory_size = SHARED_MEMORY_SIZE;
    (*header)->used_size = align_offset(sizeof(**header));

    *semaphore = sem_open(SEMAPHORE_NAME, O_CREAT | O_EXCL, 0600, 1U);
    if (*semaphore == SEM_FAILED) {
        if (errno == EEXIST)
            fprintf(stderr, "Semaphore already exists. Is another producer running?\n");
        else
            perror("sem_open");
        munmap(*header, SHARED_MEMORY_SIZE);
        close(*memory_fd);
        shm_unlink(SHARED_MEMORY_NAME);
        return -1;
    }
    return 0;
}

static int generate_blocks(struct shared_header *header, sem_t *semaphore)
{
    while (!stop_requested) {
        unsigned int number;
        unsigned int count;

        if (lock_semaphore(semaphore) == -1)
            return -1;
        if (!append_random_block(header)) {
            header->generation_finished = 1;
            count = header->total_blocks;
            if (unlock_semaphore(semaphore) == -1)
                return -1;
            printf("Shared memory is full: generated %u arrays.\n", count);
            fflush(stdout);
            return 0;
        }
        number = header->total_blocks;
        count = ((struct data_block *)((char *)header +
                                       header->last_offset))->count;
        if (unlock_semaphore(semaphore) == -1)
            return -1;

        printf("Generated array %u with %u elements.\n", number, count);
        fflush(stdout);
        sleep_milliseconds(PRODUCER_DELAY_MS);
    }
    return 0;
}

static int wait_for_consumers(struct shared_header *header, sem_t *semaphore)
{
    while (!stop_requested) {
        unsigned int processed;
        unsigned int total;
        unsigned int consumers;

        if (lock_semaphore(semaphore) == -1)
            return -1;
        if (header->generation_finished &&
            header->processed_blocks == header->total_blocks)
            header->shutting_down = 1;
        processed = header->processed_blocks;
        total = header->total_blocks;
        consumers = header->active_consumers;
        if (unlock_semaphore(semaphore) == -1)
            return -1;

        if (processed == total && consumers == 0U) {
            printf("All %u arrays have been processed.\n", total);
            return 0;
        }
        sleep_milliseconds(PRODUCER_DELAY_MS);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int memory_fd = -1;
    sem_t *semaphore = SEM_FAILED;
    struct shared_header *header = MAP_FAILED;
    int result = EXIT_SUCCESS;

    if (argc != 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (install_signal_handlers() == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    if (create_resources(&memory_fd, &semaphore, &header) == -1)
        return EXIT_FAILURE;

    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
    printf("Producer started. Press Ctrl+C to stop.\n");
    if (generate_blocks(header, semaphore) == -1 ||
        wait_for_consumers(header, semaphore) == -1) {
        if (!stop_requested)
            perror("semaphore operation");
        result = EXIT_FAILURE;
    }

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
    if (sem_unlink(SEMAPHORE_NAME) == -1) {
        perror("sem_unlink");
        result = EXIT_FAILURE;
    }
    if (shm_unlink(SHARED_MEMORY_NAME) == -1) {
        perror("shm_unlink");
        result = EXIT_FAILURE;
    }
    return result;
}
