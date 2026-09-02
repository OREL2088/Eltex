#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#include "shared.h"

static int open_resources(int *semaphore_id, struct shared_header **header)
{
    key_t memory_key = make_ipc_key('M');
    key_t semaphore_key = make_ipc_key('S');
    int shared_memory_id;
    shared_memory_id = shmget(memory_key, SHARED_MEMORY_SIZE, 0600);
    if (shared_memory_id == -1) {
        if (errno == ENOENT)
            fprintf(stderr, "Producer is not running.\n");
        else
            perror("shmget");
        return -1;
    }
    *semaphore_id = semget(semaphore_key, 1, 0600);
    if (*semaphore_id == -1) {
        perror("semget");
        return -1;
    }
    *header = shmat(shared_memory_id, NULL, 0);
    if (*header == (void *)-1) {
        perror("shmat");
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

static int process_one_block(struct shared_header *header, int semaphore_id,
                             unsigned int *number, unsigned int *count,
                             int *minimum, int *maximum)
{
    struct data_block *block;

    if (lock_semaphore(semaphore_id) == -1)
        return -1;
    if (header->shutting_down) {
        if (header->active_consumers > 0U)
            --header->active_consumers;
        unlock_semaphore(semaphore_id);
        return 0;
    }

    block = find_unprocessed_block(header);
    if (block == NULL) {
        if (unlock_semaphore(semaphore_id) == -1)
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
    if (unlock_semaphore(semaphore_id) == -1)
        return -1;
    return 1;
}

static void unregister_consumer(struct shared_header *header, int semaphore_id,
                                int *registered)
{
    if (!*registered)
        return;
    if (lock_semaphore(semaphore_id) == 0) {
        if (header->active_consumers > 0U)
            --header->active_consumers;
        unlock_semaphore(semaphore_id);
    }
    *registered = 0;
}

int main(int argc, char *argv[])
{
    int semaphore_id;
    struct shared_header *header = (void *)-1;
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
    if (open_resources(&semaphore_id, &header) == -1)
        return EXIT_FAILURE;

    if (lock_semaphore(semaphore_id) == -1) {
        perror("semop");
        shmdt(header);
        return EXIT_FAILURE;
    }
    if (!validate_shared_memory(header)) {
        fprintf(stderr, "Invalid shared memory contents.\n");
        unlock_semaphore(semaphore_id);
        shmdt(header);
        return EXIT_FAILURE;
    }
    if (!header->shutting_down) {
        ++header->active_consumers;
        registered = 1;
    }
    if (unlock_semaphore(semaphore_id) == -1) {
        perror("semop");
        shmdt(header);
        return EXIT_FAILURE;
    }

    printf("Consumer %ld started.\n", (long)getpid());
    while (!stop_requested && registered) {
        unsigned int number;
        unsigned int count;
        int minimum;
        int maximum;
        int status = process_one_block(header, semaphore_id, &number, &count,
                                       &minimum, &maximum);

        if (status == -1) {
            if (!stop_requested)
                perror("semop");
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

    unregister_consumer(header, semaphore_id, &registered);
    if (shmdt(header) == -1) {
        perror("shmdt");
        result = EXIT_FAILURE;
    }
    return result;
}
