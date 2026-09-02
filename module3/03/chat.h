#ifndef CHAT_H
#define CHAT_H

#include <mqueue.h>
#include <pthread.h>
#include <signal.h>

#define MESSAGE_SIZE 1024
#define MAX_MESSAGES 10
#define QUEUE_NAME_SIZE 256
#define NORMAL_PRIORITY 0U
#define FINISH_PRIORITY 10U

struct chat {
    mqd_t send_queue;
    mqd_t receive_queue;
    char first_name[QUEUE_NAME_SIZE];
    char second_name[QUEUE_NAME_SIZE];
    int owner;
};

struct sender_args {
    mqd_t queue;
    pthread_t main_thread;
};

extern volatile sig_atomic_t stop_requested;

int install_signal_handlers(void);
int open_chat(struct chat *chat, const char *base_name);
void close_chat(struct chat *chat);
int send_text(mqd_t queue, const char *text, unsigned int priority);
void *send_messages(void *argument);
int run_chat(struct chat *chat);

#endif
