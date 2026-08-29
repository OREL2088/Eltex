#include "ui.h"

#include <stdio.h>
#include <string.h>

static int readLine(const char *prompt, char *line, size_t size)
{
    size_t length;
    int character;

    printf("%s", prompt);
    if (fgets(line, (int)size, stdin) == NULL)
        return -1;

    length = strlen(line);
    if (length > 0U && line[length - 1U] == '\n')
    {
        line[length - 1U] = '\0';
        return 1;
    }
    while ((character = getchar()) != '\n' && character != EOF)
    {
    }
    return 0;
}

static int readInt(const char *prompt, int *value)
{
    char line[64];
    char extra;
    int status = readLine(prompt, line, sizeof(line));

    if (status != 1)
        return status;
    return sscanf(line, "%d %c", value, &extra) == 1;
}

static int readDouble(const char *prompt, double *value)
{
    char line[64];
    char extra;
    int status = readLine(prompt, line, sizeof(line));

    if (status != 1)
        return status;
    return sscanf(line, "%lf %c", value, &extra) == 1;
}

const char *uiCalculatorStatusMessage(CalculatorStatus status)
{
    switch (status)
    {
        case CALCULATOR_OK:
            return "операция выполнена";
        case CALCULATOR_INVALID_ARGUMENT:
            return "некорректный аргумент";
        case CALCULATOR_DIRECTORY_ERROR:
            return "не удалось открыть каталог библиотек";
        case CALCULATOR_NO_MEMORY:
            return "недостаточно памяти";
        case CALCULATOR_NO_OPERATIONS:
            return "операции не найдены";
        case CALCULATOR_INDEX_OUT_OF_RANGE:
            return "операция не существует";
        case CALCULATOR_INVALID_OPERAND:
            return "недопустимый операнд";
        default:
            return "неизвестная ошибка";
    }
}

void uiReportLoading(CalculatorStatus status, const char *directory,
                     size_t loaded_count)
{
    if (status != CALCULATOR_OK)
    {
        fprintf(stderr, "Ошибка загрузки из %s: %s.\n", directory,
                uiCalculatorStatusMessage(status));
        return;
    }

    printf("Загружено операций из %s: %zu\n", directory, loaded_count);
}

CalculatorStatus uiRun(const Calculator *calculator)
{
    size_t count;
    CalculatorStatus status;

    if (calculator == NULL)
        return CALCULATOR_INVALID_ARGUMENT;
    count = calculator->count;

    for (;;)
    {
        int choice;
        int input_status;
        size_t index;
        double first;
        double second;
        double result;

        printf("\n=== Калькулятор ===\n");
        for (index = 0U; index < count; ++index)
        {
            const char *name;

            status = calculatorGetOperationName(calculator, index, &name);
            if (status != CALCULATOR_OK)
            {
                return status;
            }
            printf("%zu. %s\n", index + 1U, name);
        }
        printf("0. Выход\n");
        input_status = readInt("Выберите действие: ", &choice);
        if (input_status < 0)
            return CALCULATOR_OK;
        if (input_status == 0)
        {
            printf("Некорректный ввод.\n");
            continue;
        }
        if (choice == 0)
        {
            printf("До свидания!\n");
            return CALCULATOR_OK;
        }
        if (choice < 1 || (size_t)choice > count)
        {
            printf("Такого пункта меню нет.\n");
            continue;
        }

        input_status = readDouble("Введите первое число: ", &first);
        if (input_status < 0)
            return CALCULATOR_OK;
        if (input_status == 0)
        {
            printf("Некорректный ввод.\n");
            continue;
        }
        input_status = readDouble("Введите второе число: ", &second);
        if (input_status < 0)
            return CALCULATOR_OK;
        if (input_status == 0)
        {
            printf("Некорректный ввод.\n");
            continue;
        }

        status = calculatorExecute(calculator, (size_t)choice - 1U,
                                   first, second, &result);
        if (status == CALCULATOR_INVALID_OPERAND)
        {
            printf("Ошибка: деление на ноль.\n");
            continue;
        }
        if (status != CALCULATOR_OK)
        {
            printf("Ошибка: %s.\n", uiCalculatorStatusMessage(status));
            continue;
        }
        printf("Результат: %.2f\n", result);
    }
}
