#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>

#include "chat.h"

int run_chat(struct chat *chat)
{
    struct sender_args args = {
        .queue = chat->send_queue,
        .main_thread = pthread_self()
    };
    sigset_t blocked_signals;
    pthread_t sender;
    int result = 0;

    sigemptyset(&blocked_signals);
    sigaddset(&blocked_signals, SIGINT);
    sigaddset(&blocked_signals, SIGUSR1);

    /* The sender inherits this mask, so SIGINT is handled by the main thread. */
    pthread_sigmask(SIG_BLOCK, &blocked_signals, NULL);
    if (pthread_create(&sender, NULL, send_messages, &args) != 0) {
        fprintf(stderr, "Cannot create sender thread\n");
        pthread_sigmask(SIG_UNBLOCK, &blocked_signals, NULL);
        return -1;
    }
    pthread_sigmask(SIG_UNBLOCK, &blocked_signals, NULL);

    while (!stop_requested) {
        char message[MESSAGE_SIZE + 1];
        unsigned int priority;
        ssize_t size = mq_receive(chat->receive_queue, message,
                                  MESSAGE_SIZE, &priority);

        if (size == -1) {
            if (errno == EINTR)
                continue;
            perror("mq_receive");
            result = -1;
            break;
        }
        if (priority == FINISH_PRIORITY) {
            printf("Peer has left the chat.\n");
            break;
        }

        message[size] = '\0';
        printf("Peer: %s\n", message);
        fflush(stdout);
    }

    stop_requested = 1;
    send_text(chat->send_queue, "finish", FINISH_PRIORITY);
    pthread_cancel(sender);
    pthread_join(sender, NULL);
    return result;
}
