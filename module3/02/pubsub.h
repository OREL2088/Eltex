#ifndef PUBSUB_H
#define PUBSUB_H

#include <signal.h>
#include <sys/ipc.h>
#include <sys/types.h>

#define QUEUE_KEY ((key_t)0x454c5402)
#define BROKER_TYPE 1L
#define MAX_TEXT 1024
#define MAX_TOPIC 128
#define SHUTDOWN_TIMEOUT_SECONDS 3

struct message {
    long mtype;
    char mtext[MAX_TEXT];
};

extern volatile sig_atomic_t stop_requested;

int install_signal_handler(void);
int connect_to_queue(void);
int send_text(int queue_id, long type, const char *text, int flags);
int queue_is_gone(void);
int valid_topic(const char *topic);
void sleep_briefly(void);

int run_broker(void);
int run_publisher(const char *topic);
int run_subscriber(int topic_count, char **topics);

#endif
