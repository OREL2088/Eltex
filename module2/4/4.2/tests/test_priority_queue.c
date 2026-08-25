#include "test_priority_queue.h"

#include <stdio.h>

static Message makeMessage(unsigned int id, uint8_t priority, const char *text)
{
    Message message;

    message.id = id;
    message.priority = priority;
    snprintf(message.text, sizeof(message.text), "%s", text);
    return message;
}

static void fillQueue(PriorityQueue *queue)
{
    Message messages[] = {
        makeMessage(1, 10, "first"),
        makeMessage(2, 200, "second"),
        makeMessage(3, 100, "third"),
        makeMessage(4, 200, "fourth")
    };
    size_t i;

    CU_ASSERT_EQUAL(queueInit(queue), QUEUE_OK);
    for (i = 0; i < sizeof(messages) / sizeof(messages[0]); i++)
        CU_ASSERT_EQUAL(queuePush(queue, &messages[i]), QUEUE_OK);
}

typedef struct {
    Message messages[4];
    size_t count;
} MessageCollector;

static void collectMessage(const Message *message, void *context)
{
    MessageCollector *collector = context;

    collector->messages[collector->count++] = *message;
}

void testQueueInitClearAndValidation(void)
{
    PriorityQueue queue;
    Message message = makeMessage(1, 0, "minimum");

    CU_ASSERT_EQUAL(queueInit(NULL), QUEUE_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(queueClear(NULL), QUEUE_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(queueInit(&queue), QUEUE_OK);
    CU_ASSERT_PTR_NULL(queue.head);
    CU_ASSERT_PTR_NULL(queue.tail);
    CU_ASSERT_EQUAL(queueSize(&queue), 0);

    CU_ASSERT_EQUAL(queuePush(NULL, &message), QUEUE_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(queuePush(&queue, NULL), QUEUE_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(queuePush(&queue, &message), QUEUE_OK);

    message = makeMessage(2, 255, "maximum");
    CU_ASSERT_EQUAL(queuePush(&queue, &message), QUEUE_OK);
    CU_ASSERT_EQUAL(queueSize(&queue), 2);

    CU_ASSERT_EQUAL(queueClear(&queue), QUEUE_OK);
    CU_ASSERT_PTR_NULL(queue.head);
    CU_ASSERT_PTR_NULL(queue.tail);
    CU_ASSERT_EQUAL(queueSize(&queue), 0);
}

void testQueuePopFront(void)
{
    PriorityQueue queue;
    Message message;

    fillQueue(&queue);

    CU_ASSERT_EQUAL(queuePopFront(&queue, &message), QUEUE_OK);
    CU_ASSERT_EQUAL(message.id, 1);
    CU_ASSERT_EQUAL(message.priority, 10);
    CU_ASSERT_STRING_EQUAL(message.text, "first");
    CU_ASSERT_EQUAL(queueSize(&queue), 3);

    CU_ASSERT_EQUAL(queueClear(&queue), QUEUE_OK);
}

void testQueuePopExactPriority(void)
{
    PriorityQueue queue;
    Message message;

    fillQueue(&queue);

    CU_ASSERT_EQUAL(queuePopPriority(&queue, 200, &message), QUEUE_OK);
    CU_ASSERT_EQUAL(message.id, 2);
    CU_ASSERT_STRING_EQUAL(message.text, "second");

    CU_ASSERT_EQUAL(queuePopPriority(&queue, 200, &message), QUEUE_OK);
    CU_ASSERT_EQUAL(message.id, 4);
    CU_ASSERT_STRING_EQUAL(message.text, "fourth");

    CU_ASSERT_EQUAL(queuePopPriority(&queue, 200, &message), QUEUE_NOT_FOUND);
    CU_ASSERT_EQUAL(queuePopPriority(&queue, -1, &message),
                    QUEUE_INVALID_PRIORITY);
    CU_ASSERT_EQUAL(queuePopPriority(&queue, 256, &message),
                    QUEUE_INVALID_PRIORITY);
    CU_ASSERT_EQUAL(queueSize(&queue), 2);

    CU_ASSERT_EQUAL(queueClear(&queue), QUEUE_OK);
}

void testQueuePopNotLower(void)
{
    PriorityQueue queue;
    Message message;

    fillQueue(&queue);

    CU_ASSERT_EQUAL(queuePopNotLower(&queue, 100, &message), QUEUE_OK);
    CU_ASSERT_EQUAL(message.id, 2);
    CU_ASSERT_EQUAL(message.priority, 200);

    CU_ASSERT_EQUAL(queuePopNotLower(&queue, 100, &message), QUEUE_OK);
    CU_ASSERT_EQUAL(message.id, 3);
    CU_ASSERT_EQUAL(message.priority, 100);

    CU_ASSERT_EQUAL(queuePopNotLower(&queue, 201, &message), QUEUE_NOT_FOUND);
    CU_ASSERT_EQUAL(queuePopNotLower(&queue, -1, &message),
                    QUEUE_INVALID_PRIORITY);

    CU_ASSERT_EQUAL(queueClear(&queue), QUEUE_OK);
}

void testQueueEmptyAndInvalidArguments(void)
{
    PriorityQueue queue;
    Message message = makeMessage(7, 42, "only");

    CU_ASSERT_EQUAL(queueInit(&queue), QUEUE_OK);
    CU_ASSERT_EQUAL(queuePopFront(&queue, &message), QUEUE_EMPTY);
    CU_ASSERT_EQUAL(queuePopPriority(&queue, 0, &message), QUEUE_EMPTY);
    CU_ASSERT_EQUAL(queuePopNotLower(&queue, 0, &message), QUEUE_EMPTY);

    CU_ASSERT_EQUAL(queuePopFront(NULL, &message), QUEUE_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(queuePopFront(&queue, NULL), QUEUE_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(queuePopPriority(&queue, 42, NULL),
                    QUEUE_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(queuePopNotLower(NULL, 42, &message),
                    QUEUE_INVALID_ARGUMENT);

    CU_ASSERT_EQUAL(queuePush(&queue, &message), QUEUE_OK);
    CU_ASSERT_EQUAL(queuePopPriority(&queue, 42, &message), QUEUE_OK);
    CU_ASSERT_EQUAL(queueSize(&queue), 0);
    CU_ASSERT_PTR_NULL(queue.head);
    CU_ASSERT_PTR_NULL(queue.tail);

    message = makeMessage(8, 43, "new");
    CU_ASSERT_EQUAL(queuePush(&queue, &message), QUEUE_OK);
    CU_ASSERT_EQUAL(queuePopFront(&queue, &message), QUEUE_OK);
    CU_ASSERT_EQUAL(message.id, 8);

    CU_ASSERT_EQUAL(queueClear(&queue), QUEUE_OK);
}

void testQueueForEach(void)
{
    PriorityQueue queue;
    MessageCollector collector = {0};

    CU_ASSERT_EQUAL(queueInit(&queue), QUEUE_OK);
    CU_ASSERT_EQUAL(queueForEach(&queue, collectMessage, &collector),
                    QUEUE_EMPTY);
    CU_ASSERT_EQUAL(queueForEach(NULL, collectMessage, &collector),
                    QUEUE_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(queueForEach(&queue, NULL, &collector),
                    QUEUE_INVALID_ARGUMENT);

    fillQueue(&queue);
    CU_ASSERT_EQUAL(queueForEach(&queue, collectMessage, &collector), QUEUE_OK);
    CU_ASSERT_EQUAL(collector.count, 4);
    CU_ASSERT_EQUAL(collector.messages[0].id, 1);
    CU_ASSERT_EQUAL(collector.messages[1].id, 2);
    CU_ASSERT_EQUAL(collector.messages[2].id, 3);
    CU_ASSERT_EQUAL(collector.messages[3].id, 4);
    CU_ASSERT_STRING_EQUAL(collector.messages[0].text, "first");
    CU_ASSERT_STRING_EQUAL(collector.messages[3].text, "fourth");
    CU_ASSERT_EQUAL(queueSize(&queue), 4);

    CU_ASSERT_EQUAL(queueClear(&queue), QUEUE_OK);
}
