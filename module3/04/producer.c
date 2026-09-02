#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

#include "shared.h"

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

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

static int create_resources(int *shared_memory_id, int *semaphore_id,
                            struct shared_header **header)
{
    key_t memory_key = make_ipc_key('M');
    key_t semaphore_key = make_ipc_key('S');
    union semun argument;

    *shared_memory_id = shmget(memory_key, SHARED_MEMORY_SIZE,
                               IPC_CREAT | IPC_EXCL | 0600);
    if (*shared_memory_id == -1) {
        if (errno == EEXIST)
            fprintf(stderr, "Shared memory already exists. Is another producer running?\n");
        else
            perror("shmget");
        return -1;
    }

    *semaphore_id = semget(semaphore_key, 1, IPC_CREAT | IPC_EXCL | 0600);
    if (*semaphore_id == -1) {
        if (errno == EEXIST)
            fprintf(stderr, "Semaphore already exists. Is another producer running?\n");
        else
            perror("semget");
        shmctl(*shared_memory_id, IPC_RMID, NULL);
        return -1;
    }

    *header = shmat(*shared_memory_id, NULL, 0);
    if (*header == (void *)-1) {
        perror("shmat");
        semctl(*semaphore_id, 0, IPC_RMID);
        shmctl(*shared_memory_id, IPC_RMID, NULL);
        return -1;
    }

    memset(*header, 0, SHARED_MEMORY_SIZE);
    (*header)->magic = SHARED_MAGIC;
    (*header)->version = SHARED_VERSION;
    (*header)->memory_size = SHARED_MEMORY_SIZE;
    (*header)->used_size = align_offset(sizeof(**header));

    argument.val = 1;
    if (semctl(*semaphore_id, 0, SETVAL, argument) == -1) {
        perror("semctl");
        shmdt(*header);
        semctl(*semaphore_id, 0, IPC_RMID);
        shmctl(*shared_memory_id, IPC_RMID, NULL);
        return -1;
    }
    return 0;
}

static int generate_blocks(struct shared_header *header, int semaphore_id)
{
    while (!stop_requested) {
        unsigned int number;
        unsigned int count;

        if (lock_semaphore(semaphore_id) == -1)
            return -1;
        if (!append_random_block(header)) {
            header->generation_finished = 1;
            count = header->total_blocks;
            if (unlock_semaphore(semaphore_id) == -1)
                return -1;
            printf("Shared memory is full: generated %u arrays.\n", count);
            fflush(stdout);
            return 0;
        }
        number = header->total_blocks;
        count = ((struct data_block *)((char *)header +
                                       header->last_offset))->count;
        if (unlock_semaphore(semaphore_id) == -1)
            return -1;

        printf("Generated array %u with %u elements.\n", number, count);
        fflush(stdout);
        sleep_milliseconds(PRODUCER_DELAY_MS);
    }
    return 0;
}

static int wait_for_consumers(struct shared_header *header, int semaphore_id)
{
    while (!stop_requested) {
        unsigned int processed;
        unsigned int total;
        unsigned int consumers;

        if (lock_semaphore(semaphore_id) == -1)
            return -1;
        if (header->processed_blocks == header->total_blocks)
            header->shutting_down = 1;
        processed = header->processed_blocks;
        total = header->total_blocks;
        consumers = header->active_consumers;
        if (unlock_semaphore(semaphore_id) == -1)
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
    int shared_memory_id = -1;
    int semaphore_id = -1;
    struct shared_header *header = (void *)-1;
    int result = EXIT_SUCCESS;

    if (argc != 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (install_signal_handlers() == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    if (create_resources(&shared_memory_id, &semaphore_id, &header) == -1)
        return EXIT_FAILURE;

    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
    printf("Producer started. Press Ctrl+C to stop.\n");
    if (generate_blocks(header, semaphore_id) == -1 ||
        wait_for_consumers(header, semaphore_id) == -1) {
        if (!stop_requested)
            perror("semaphore operation");
        result = EXIT_FAILURE;
    }

    if (shmdt(header) == -1) {
        perror("shmdt");
        result = EXIT_FAILURE;
    }
    if (shmctl(shared_memory_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        result = EXIT_FAILURE;
    }
    if (semctl(semaphore_id, 0, IPC_RMID) == -1) {
        perror("semctl");
        result = EXIT_FAILURE;
    }
    return result;
}
