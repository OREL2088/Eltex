#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "chat.h"

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
           sigaction(SIGUSR1, &action, NULL) == -1 ? -1 : 0;
}

static int make_queue_names(struct chat *chat, const char *base_name)
{
    const char *prefix = base_name[0] == '/' ? "" : "/";
    int first_size;
    int second_size;

    if (strchr(base_name + (base_name[0] == '/'), '/') != NULL)
        return -1;

    first_size = snprintf(chat->first_name, sizeof(chat->first_name),
                          "%s%s_1", prefix, base_name);
    second_size = snprintf(chat->second_name, sizeof(chat->second_name),
                           "%s%s_2", prefix, base_name);
    return first_size > 0 && first_size < QUEUE_NAME_SIZE &&
           second_size > 0 && second_size < QUEUE_NAME_SIZE ? 0 : -1;
}

int open_chat(struct chat *chat, const char *base_name)
{
    struct mq_attr attributes = {
        .mq_maxmsg = MAX_MESSAGES,
        .mq_msgsize = MESSAGE_SIZE
    };
    mqd_t first;
    mqd_t second;

    memset(chat, 0, sizeof(*chat));
    chat->send_queue = (mqd_t)-1;
    chat->receive_queue = (mqd_t)-1;

    if (make_queue_names(chat, base_name) == -1) {
        fprintf(stderr, "Invalid queue name\n");
        return -1;
    }

    first = mq_open(chat->first_name, O_RDWR | O_CREAT | O_EXCL,
                    0600, &attributes);
    if (first != (mqd_t)-1) {
        second = mq_open(chat->second_name, O_RDWR | O_CREAT | O_EXCL,
                         0600, &attributes);
        if (second == (mqd_t)-1) {
            perror("mq_open");
            mq_close(first);
            mq_unlink(chat->first_name);
            return -1;
        }
        chat->owner = 1;
        chat->receive_queue = first;
        chat->send_queue = second;
        return 0;
    }

    if (errno != EEXIST) {
        perror("mq_open");
        return -1;
    }

    first = mq_open(chat->first_name, O_WRONLY);
    second = mq_open(chat->second_name, O_RDONLY);
    if (first == (mqd_t)-1 || second == (mqd_t)-1) {
        perror("mq_open");
        if (first != (mqd_t)-1)
            mq_close(first);
        if (second != (mqd_t)-1)
            mq_close(second);
        return -1;
    }

    chat->send_queue = first;
    chat->receive_queue = second;
    return 0;
}

int send_text(mqd_t queue, const char *text, unsigned int priority)
{
    if (mq_send(queue, text, strlen(text) + 1, priority) == -1) {
        if (errno != EINTR)
            perror("mq_send");
        return -1;
    }
    return 0;
}

void close_chat(struct chat *chat)
{
    if (chat->send_queue != (mqd_t)-1)
        mq_close(chat->send_queue);
    if (chat->receive_queue != (mqd_t)-1)
        mq_close(chat->receive_queue);
    if (chat->owner) {
        mq_unlink(chat->first_name);
        mq_unlink(chat->second_name);
    }
}
