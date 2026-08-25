#include "priority_queue.h"

#include <stdlib.h>

struct QueueNode {
    Message message;
    struct QueueNode *next;
};

static int isValidPriority(int priority)
{
    return priority >= 0 && priority <= UINT8_MAX;
}

static void removeNode(PriorityQueue *queue, QueueNode *previous,
                       QueueNode *node, Message *message)
{
    *message = node->message;

    if (previous == NULL)
        queue->head = node->next;
    else
        previous->next = node->next;

    if (queue->tail == node)
        queue->tail = previous;

    free(node);
    queue->size--;
}

QueueStatus queueInit(PriorityQueue *queue)
{
    if (queue == NULL)
        return QUEUE_INVALID_ARGUMENT;

    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    return QUEUE_OK;
}

QueueStatus queueClear(PriorityQueue *queue)
{
    QueueNode *current;

    if (queue == NULL)
        return QUEUE_INVALID_ARGUMENT;

    current = queue->head;
    while (current != NULL) {
        QueueNode *next = current->next;

        free(current);
        current = next;
    }

    return queueInit(queue);
}

size_t queueSize(const PriorityQueue *queue)
{
    return queue == NULL ? 0 : queue->size;
}

QueueStatus queueForEach(const PriorityQueue *queue, MessageHandler handler,
                         void *context)
{
    const QueueNode *current;

    if (queue == NULL || handler == NULL)
        return QUEUE_INVALID_ARGUMENT;
    if (queue->head == NULL)
        return QUEUE_EMPTY;

    for (current = queue->head; current != NULL; current = current->next)
        handler(&current->message, context);

    return QUEUE_OK;
}

QueueStatus queuePush(PriorityQueue *queue, const Message *message)
{
    QueueNode *node;

    if (queue == NULL || message == NULL)
        return QUEUE_INVALID_ARGUMENT;

    node = malloc(sizeof(*node));
    if (node == NULL)
        return QUEUE_NO_MEMORY;

    node->message = *message;
    node->next = NULL;

    if (queue->tail == NULL)
        queue->head = node;
    else
        queue->tail->next = node;

    queue->tail = node;
    queue->size++;
    return QUEUE_OK;
}

QueueStatus queuePopFront(PriorityQueue *queue, Message *message)
{
    if (queue == NULL || message == NULL)
        return QUEUE_INVALID_ARGUMENT;
    if (queue->head == NULL)
        return QUEUE_EMPTY;

    removeNode(queue, NULL, queue->head, message);
    return QUEUE_OK;
}

QueueStatus queuePopPriority(PriorityQueue *queue, int priority,
                             Message *message)
{
    QueueNode *previous = NULL;
    QueueNode *current;

    if (queue == NULL || message == NULL)
        return QUEUE_INVALID_ARGUMENT;
    if (!isValidPriority(priority))
        return QUEUE_INVALID_PRIORITY;
    if (queue->head == NULL)
        return QUEUE_EMPTY;

    current = queue->head;
    while (current != NULL && current->message.priority != priority) {
        previous = current;
        current = current->next;
    }

    if (current == NULL)
        return QUEUE_NOT_FOUND;

    removeNode(queue, previous, current, message);
    return QUEUE_OK;
}

QueueStatus queuePopNotLower(PriorityQueue *queue, int minPriority,
                             Message *message)
{
    QueueNode *previous = NULL;
    QueueNode *current;

    if (queue == NULL || message == NULL)
        return QUEUE_INVALID_ARGUMENT;
    if (!isValidPriority(minPriority))
        return QUEUE_INVALID_PRIORITY;
    if (queue->head == NULL)
        return QUEUE_EMPTY;

    current = queue->head;
    while (current != NULL && current->message.priority < minPriority) {
        previous = current;
        current = current->next;
    }

    if (current == NULL)
        return QUEUE_NOT_FOUND;

    removeNode(queue, previous, current, message);
    return QUEUE_OK;
}
