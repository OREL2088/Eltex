#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>

#include "pubsub.h"

struct subscription {
    pid_t pid;
    char topic[MAX_TOPIC];
};

struct broker_state {
    struct subscription *subscriptions;
    size_t subscription_count;
    pid_t *publishers;
    size_t publisher_count;
};

static int parse_pid(const char *text, pid_t *pid)
{
    char *end;
    long value = strtol(text, &end, 10);

    if (*text == '\0' || *end != '\0' || value <= 1)
        return -1;
    *pid = (pid_t)value;
    return 0;
}

static int add_publisher(struct broker_state *state, pid_t pid)
{
    pid_t *items;

    for (size_t i = 0; i < state->publisher_count; i++)
        if (state->publishers[i] == pid)
            return 0;

    items = realloc(state->publishers,
                    (state->publisher_count + 1) * sizeof(*items));
    if (!items)
        return -1;
    state->publishers = items;
    state->publishers[state->publisher_count++] = pid;
    return 0;
}

static int subscribe(struct broker_state *state, pid_t pid, const char *topic)
{
    struct subscription *items;

    for (size_t i = 0; i < state->subscription_count; i++)
        if (state->subscriptions[i].pid == pid &&
            strcmp(state->subscriptions[i].topic, topic) == 0)
            return 0;

    items = realloc(state->subscriptions,
                    (state->subscription_count + 1) * sizeof(*items));
    if (!items)
        return -1;
    state->subscriptions = items;
    items[state->subscription_count].pid = pid;
    strcpy(items[state->subscription_count].topic, topic);
    state->subscription_count++;
    return 0;
}

static void unsubscribe(struct broker_state *state, pid_t pid,
                        const char *topic)
{
    for (size_t i = 0; i < state->subscription_count; i++) {
        struct subscription *item = &state->subscriptions[i];

        if (item->pid == pid && strcmp(item->topic, topic) == 0) {
            *item = state->subscriptions[--state->subscription_count];
            return;
        }
    }
}

static void forward_message(int queue_id, const struct broker_state *state,
                            const char *topic, const char *payload)
{
    char text[MAX_TEXT];
    int length = snprintf(text, sizeof(text), "%s: %s", topic, payload);

    if (length < 0 || (size_t)length >= sizeof(text)) {
        fprintf(stderr, "Broker: message is too long\n");
        return;
    }
    for (size_t i = 0; i < state->subscription_count; i++) {
        const struct subscription *item = &state->subscriptions[i];

        if (strcmp(item->topic, topic) == 0 &&
            send_text(queue_id, (long)item->pid, text, IPC_NOWAIT) == -1 &&
            !queue_is_gone())
            fprintf(stderr, "Broker: cannot send to pid %ld\n",
                    (long)item->pid);
    }
}

static void process_message(int queue_id, struct broker_state *state,
                            char *text)
{
    char *save;
    char *command = strtok_r(text, ",", &save);
    char *pid_text = strtok_r(NULL, ",", &save);
    char *topic = strtok_r(NULL, ",", &save);
    pid_t pid;

    if (!command || !pid_text || !topic || !valid_topic(topic) ||
        parse_pid(pid_text, &pid) == -1) {
        fprintf(stderr, "Broker: invalid message\n");
        return;
    }

    if (strcmp(command, "subscribe") == 0) {
        if (subscribe(state, pid, topic) == -1)
            stop_requested = 1;
        else
            printf("Broker: pid %ld subscribed to %s\n", (long)pid, topic);
    } else if (strcmp(command, "unsubscribe") == 0) {
        unsubscribe(state, pid, topic);
        printf("Broker: pid %ld unsubscribed from %s\n", (long)pid, topic);
    } else if (strcmp(command, "send") == 0) {
        if (add_publisher(state, pid) == -1)
            stop_requested = 1;
        else
            forward_message(queue_id, state, topic, save);
    } else {
        fprintf(stderr, "Broker: unknown command\n");
    }
}

static void notify_clients(const struct broker_state *state)
{
    for (size_t i = 0; i < state->publisher_count; i++)
        kill(state->publishers[i], SIGINT);
    for (size_t i = 0; i < state->subscription_count; i++)
        kill(state->subscriptions[i].pid, SIGINT);
}

static void wait_for_empty_queue(int queue_id)
{
    time_t deadline = time(NULL) + SHUTDOWN_TIMEOUT_SECONDS;
    struct msqid_ds info;
    struct message message;

    while (time(NULL) < deadline) {
        while (msgrcv(queue_id, &message, sizeof(message.mtext), BROKER_TYPE,
                      IPC_NOWAIT | MSG_NOERROR) >= 0) {
        }
        if (msgctl(queue_id, IPC_STAT, &info) == -1 || info.msg_qnum == 0)
            return;
        sleep_briefly();
    }
}

int run_broker(void)
{
    int queue_id = msgget(QUEUE_KEY, IPC_CREAT | IPC_EXCL | 0600);
    struct broker_state state = {0};

    if (queue_id == -1) {
        fprintf(stderr, errno == EEXIST ? "Broker is already running\n" :
                                         "Cannot create message queue\n");
        return EXIT_FAILURE;
    }
    printf("Broker started (queue id %d)\n", queue_id);

    while (!stop_requested) {
        struct message message;
        ssize_t size = msgrcv(queue_id, &message, sizeof(message.mtext),
                              BROKER_TYPE, MSG_NOERROR);

        if (size == -1) {
            if (errno == EINTR)
                continue;
            perror("msgrcv");
            break;
        }
        message.mtext[size < MAX_TEXT ? size : MAX_TEXT - 1] = '\0';
        process_message(queue_id, &state, message.mtext);
    }

    printf("Broker is shutting down\n");
    notify_clients(&state);
    wait_for_empty_queue(queue_id);
    msgctl(queue_id, IPC_RMID, NULL);
    free(state.subscriptions);
    free(state.publishers);
    return EXIT_SUCCESS;
}
