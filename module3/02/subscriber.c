#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <unistd.h>

#include "pubsub.h"

static int send_command(int queue_id, const char *command, const char *topic,
                        int flags)
{
    char text[MAX_TEXT];

    snprintf(text, sizeof(text), "%s,%ld,%s", command, (long)getpid(), topic);
    return send_text(queue_id, BROKER_TYPE, text, flags);
}

static void unsubscribe_all(int queue_id, int count, char **topics)
{
    for (int i = 0; i < count; i++) {
        if (send_command(queue_id, "unsubscribe", topics[i], IPC_NOWAIT) == -1 &&
            queue_is_gone())
            return;
    }
}

int run_subscriber(int topic_count, char **topics)
{
    int queue_id = connect_to_queue();

    if (queue_id == -1)
        return EXIT_FAILURE;

    for (int i = 0; i < topic_count; i++) {
        if (!valid_topic(topics[i])) {
            fprintf(stderr, "Invalid topic: %s\n", topics[i]);
            unsubscribe_all(queue_id, i, topics);
            return EXIT_FAILURE;
        }
        if (send_command(queue_id, "subscribe", topics[i], 0) == -1) {
            if (!queue_is_gone())
                perror("msgsnd");
            unsubscribe_all(queue_id, i, topics);
            return EXIT_FAILURE;
        }
    }

    printf("Subscriber pid %ld is waiting for messages\n", (long)getpid());
    while (!stop_requested) {
        struct message message;
        ssize_t size = msgrcv(queue_id, &message, sizeof(message.mtext),
                              (long)getpid(), MSG_NOERROR);

        if (size == -1) {
            if (errno == EINTR)
                continue;
            if (!queue_is_gone())
                perror("msgrcv");
            break;
        }
        message.mtext[size < MAX_TEXT ? size : MAX_TEXT - 1] = '\0';
        printf("Received: %s\n", message.mtext);
        fflush(stdout);
    }

    unsubscribe_all(queue_id, topic_count, topics);
    printf("Subscriber finished\n");
    return EXIT_SUCCESS;
}
