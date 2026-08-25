#include "menu.h"

#include "priority_queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEMO_MESSAGE_COUNT 12
#define DEMO_MIN_PRIORITY 128

static int readNumber(const char *prompt, int min, int max, int *number)
{
    char line[32];
    char extra;

    for (;;) {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL)
            return 0;

        if (sscanf(line, "%d %c", number, &extra) == 1 &&
            *number >= min && *number <= max) {
            return 1;
        }

        printf("Введите число от %d до %d.\n", min, max);
    }
}

static int readText(char text[MESSAGE_TEXT_SIZE])
{
    size_t length;

    printf("Текст сообщения: ");
    if (fgets(text, MESSAGE_TEXT_SIZE, stdin) == NULL)
        return 0;

    length = strcspn(text, "\n");
    if (text[length] == '\n') {
        text[length] = '\0';
    } else {
        int character;

        while ((character = getchar()) != '\n' && character != EOF)
            ;
    }

    return text[0] != '\0';
}

static void printMessage(const Message *message)
{
    printf("id=%u, приоритет=%u, текст=\"%s\"\n", message->id,
           (unsigned int)message->priority, message->text);
}

static void printQueueMessage(const Message *message, void *context)
{
    (void)context;
    printf("  ");
    printMessage(message);
}

static void printQueue(const PriorityQueue *queue)
{
    printf("\nВсе сообщения:\n");
    if (queueForEach(queue, printQueueMessage, NULL) == QUEUE_EMPTY)
        printf("Очередь пуста.\n");
}

static void printPopResult(QueueStatus status, const Message *message)
{
    if (status == QUEUE_OK) {
        printf("Извлечено: ");
        printMessage(message);
    } else if (status == QUEUE_EMPTY) {
        printf("Очередь пуста.\n");
    } else if (status == QUEUE_NOT_FOUND) {
        printf("Подходящее сообщение не найдено.\n");
    } else {
        printf("Ошибка извлечения.\n");
    }
}

static int addMessage(PriorityQueue *queue, unsigned int *nextId)
{
    Message message;
    int priority;

    if (!readNumber("Приоритет (0-255): ", 0, UINT8_MAX, &priority) ||
        !readText(message.text)) {
        return 0;
    }

    message.id = (*nextId)++;
    message.priority = (uint8_t)priority;

    if (queuePush(queue, &message) != QUEUE_OK) {
        printf("Не удалось добавить сообщение.\n");
        return 1;
    }

    printf("Добавлено: ");
    printMessage(&message);
    return 1;
}

static int popByPriority(PriorityQueue *queue, int notLower)
{
    Message message;
    QueueStatus status;
    int priority;

    if (!readNumber("Приоритет (0-255): ", 0, UINT8_MAX, &priority))
        return 0;

    if (notLower)
        status = queuePopNotLower(queue, priority, &message);
    else
        status = queuePopPriority(queue, priority, &message);

    printPopResult(status, &message);
    return 1;
}

static void runDemo(void)
{
    PriorityQueue queue;
    Message message;
    int exactPriority = 0;
    unsigned int i;

    if (queueInit(&queue) != QUEUE_OK)
        return;

    printf("\nГенерация сообщений:\n");
    for (i = 0; i < DEMO_MESSAGE_COUNT; i++) {
        message.id = i + 1;
        message.priority = (uint8_t)(rand() % 256);
        snprintf(message.text, sizeof(message.text), "Сообщение %u", i + 1);

        if (i == 5)
            exactPriority = message.priority;

        printf("  ");
        printMessage(&message);
        if (queuePush(&queue, &message) != QUEUE_OK) {
            printf("Не удалось добавить сообщение.\n");
            queueClear(&queue);
            return;
        }
    }

    printf("\nПервое в очереди:\n");
    printPopResult(queuePopFront(&queue, &message), &message);
    printf("Первое с приоритетом %d:\n", exactPriority);
    printPopResult(queuePopPriority(&queue, exactPriority, &message), &message);
    printf("Первое с приоритетом не ниже %d:\n", DEMO_MIN_PRIORITY);
    printPopResult(queuePopNotLower(&queue, DEMO_MIN_PRIORITY, &message),
                   &message);
    printf("Осталось сообщений: %zu\n", queueSize(&queue));

    queueClear(&queue);
}

static void printMenu(size_t size)
{
    printf("\n============ ОЧЕРЕДЬ ============\n"
           "Сообщений: %zu\n"
           "1. Добавить сообщение\n"
           "2. Показать все сообщения\n"
           "3. Извлечь первое\n"
           "4. Извлечь с указанным приоритетом\n"
           "5. Извлечь с приоритетом не ниже заданного\n"
           "6. Автоматическая демонстрация\n"
           "0. Выход\n",
           size);
}

int runMenu(void)
{
    PriorityQueue queue;
    unsigned int nextId = 1;
    int choice;
    int running = 1;

    if (queueInit(&queue) != QUEUE_OK)
        return EXIT_FAILURE;

    srand((unsigned int)time(NULL));
    while (running) {
        Message message;

        printMenu(queueSize(&queue));
        if (!readNumber("Выберите действие: ", 0, 6, &choice))
            break;

        switch (choice) {
        case 1:
            running = addMessage(&queue, &nextId);
            break;
        case 2:
            printQueue(&queue);
            break;
        case 3:
            printPopResult(queuePopFront(&queue, &message), &message);
            break;
        case 4:
            running = popByPriority(&queue, 0);
            break;
        case 5:
            running = popByPriority(&queue, 1);
            break;
        case 6:
            runDemo();
            break;
        case 0:
            running = 0;
            break;
        }
    }

    queueClear(&queue);
    return EXIT_SUCCESS;
}
