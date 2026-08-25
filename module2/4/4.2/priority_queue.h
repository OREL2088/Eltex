#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stddef.h>
#include <stdint.h>

#define MESSAGE_TEXT_SIZE 128

typedef struct {
    unsigned int id;
    uint8_t priority;
    char text[MESSAGE_TEXT_SIZE];
} Message;

typedef struct QueueNode QueueNode;

typedef struct {
    QueueNode *head;
    QueueNode *tail;
    size_t size;
} PriorityQueue;

typedef enum {
    QUEUE_OK,
    QUEUE_EMPTY,
    QUEUE_NOT_FOUND,
    QUEUE_INVALID_ARGUMENT,
    QUEUE_INVALID_PRIORITY,
    QUEUE_NO_MEMORY
} QueueStatus;

typedef void (*MessageHandler)(const Message *message, void *context);

QueueStatus queueInit(PriorityQueue *queue);
QueueStatus queueClear(PriorityQueue *queue);
size_t queueSize(const PriorityQueue *queue);
QueueStatus queueForEach(const PriorityQueue *queue, MessageHandler handler,
                         void *context);

QueueStatus queuePush(PriorityQueue *queue, const Message *message);
QueueStatus queuePopFront(PriorityQueue *queue, Message *message);
QueueStatus queuePopPriority(PriorityQueue *queue, int priority,
                             Message *message);
QueueStatus queuePopNotLower(PriorityQueue *queue, int minPriority,
                             Message *message);

#endif
