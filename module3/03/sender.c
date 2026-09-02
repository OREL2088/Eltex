#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>

#include "chat.h"

void *send_messages(void *argument)
{
    struct sender_args *args = argument;
    char message[MESSAGE_SIZE];

    while (!stop_requested && fgets(message, sizeof(message), stdin)) {
        size_t length = strcspn(message, "\n");

        if (message[length] == '\n') {
            message[length] = '\0';
        } else if (!feof(stdin)) {
            int character;

            while ((character = getchar()) != '\n' && character != EOF) {
            }
            fprintf(stderr, "Message is too long\n");
            continue;
        }

        if (send_text(args->queue, message, NORMAL_PRIORITY) == -1)
            break;
    }

    stop_requested = 1;
    pthread_kill(args->main_thread, SIGUSR1);
    return NULL;
}
