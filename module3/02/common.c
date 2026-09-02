#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <time.h>

#include "pubsub.h"

volatile sig_atomic_t stop_requested = 0;

static void handle_sigint(int signal_number)
{
    (void)signal_number;
    stop_requested = 1;
}

int install_signal_handler(void)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_sigint;
    sigemptyset(&action.sa_mask);
    return sigaction(SIGINT, &action, NULL);
}

int connect_to_queue(void)
{
    int queue_id = msgget(QUEUE_KEY, 0600);

    if (queue_id == -1)
        fprintf(stderr, "Cannot connect to broker: %s\n", strerror(errno));
    return queue_id;
}

int send_text(int queue_id, long type, const char *text, int flags)
{
    struct message message;
    size_t size = strlen(text) + 1;

    if (size > sizeof(message.mtext)) {
        errno = EMSGSIZE;
        return -1;
    }
    message.mtype = type;
    memcpy(message.mtext, text, size);
    return msgsnd(queue_id, &message, size, flags);
}

int queue_is_gone(void)
{
    return errno == EIDRM || errno == EINVAL || errno == ENOENT;
}

int valid_topic(const char *topic)
{
    return *topic != '\0' && strlen(topic) < MAX_TOPIC &&
           strchr(topic, ',') == NULL;
}

void sleep_briefly(void)
{
    struct timespec delay = {.tv_sec = 0, .tv_nsec = 100000000L};

    nanosleep(&delay, NULL);
}
