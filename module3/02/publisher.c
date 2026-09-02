#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <unistd.h>

#include "pubsub.h"

int run_publisher(const char *topic)
{
    char payload[MAX_TEXT];
    char text[MAX_TEXT];
    int queue_id;

    if (!valid_topic(topic)) {
        fprintf(stderr, "Invalid topic\n");
        return EXIT_FAILURE;
    }
    queue_id = connect_to_queue();
    if (queue_id == -1)
        return EXIT_FAILURE;

    printf("Publisher pid %ld, topic %s. Enter messages:\n",
           (long)getpid(), topic);

    while (!stop_requested && fgets(payload, sizeof(payload), stdin)) {
        int length;

        payload[strcspn(payload, "\n")] = '\0';
        length = snprintf(text, sizeof(text), "send,%ld,%s,%s",
                          (long)getpid(), topic, payload);
        if (length < 0 || (size_t)length >= sizeof(text)) {
            fprintf(stderr, "Message is too long\n");
            continue;
        }
        if (send_text(queue_id, BROKER_TYPE, text, 0) == -1) {
            if (!queue_is_gone())
                perror("msgsnd");
            break;
        }
    }

    printf("Publisher finished\n");
    return EXIT_SUCCESS;
}
