#include "permissions.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define INPUT_SIZE 256

typedef enum
{
    LINE_OK,
    LINE_END,
    LINE_TOO_LONG
} LineResult;

static void print_permissions(mode_t mode)
{
    char letters[10];
    char binary[10];

    format_letter_permissions(mode, letters);
    format_binary_permissions(mode, binary);

    printf("Буквенный: %s\n", letters);
    printf("Цифровой:  %03o\n", (unsigned int)(mode & 0777));
    printf("Битовый:   %s\n", binary);
}

static char file_type(mode_t mode)
{
    if (S_ISREG(mode))
        return '-';
    if (S_ISDIR(mode))
        return 'd';
    if (S_ISLNK(mode))
        return 'l';
    if (S_ISCHR(mode))
        return 'c';
    if (S_ISBLK(mode))
        return 'b';
    if (S_ISFIFO(mode))
        return 'p';

    return '?';
}

static LineResult read_line(char *buffer, size_t size)
{
    int character;
    char *newline;

    if (fgets(buffer, (int)size, stdin) == NULL)
        return LINE_END;

    newline = strchr(buffer, '\n');
    if (newline != NULL)
    {
        *newline = '\0';
        return LINE_OK;
    }

    while ((character = getchar()) != '\n' && character != EOF)
    {
    }

    return LINE_TOO_LONG;
}

static void print_file_comparison(mode_t mode)
{
    char letters[10];

    format_letter_permissions(mode, letters);
    printf("Как в ls -l: %c%s\n", file_type(mode), letters);
}

static void run_menu(mode_t initial_mode)
{
    mode_t current = initial_mode & 0777;
    char input[INPUT_SIZE];

    for (;;)
    {
        char *end;
        long choice;
        LineResult line_result;

        printf("\nМеню:\n");
        printf("1. Показать текущие права\n");
        printf("2. Изменить права\n");
        printf("3. Выйти\n");
        printf("Выберите действие: ");

        line_result = read_line(input, sizeof(input));
        if (line_result == LINE_END)
            return;
        if (line_result == LINE_TOO_LONG)
        {
            printf("Слишком длинный ввод.\n");
            continue;
        }

        choice = strtol(input, &end, 10);
        if (end == input || *end != '\0')
        {
            printf("Некорректный пункт меню.\n");
            continue;
        }

        if (choice == 1)
        {
            print_permissions(current);
        }
        else if (choice == 2)
        {
            mode_t changed;

            printf("Введите новые права или команду изменения: ");
            line_result = read_line(input, sizeof(input));
            if (line_result == LINE_END)
                return;
            if (line_result == LINE_TOO_LONG)
            {
                printf("Слишком длинная команда.\n");
                continue;
            }

            if (!parse_octal_permissions(input, &changed) &&
                !apply_permission_command(current, input, &changed))
            {
                printf("Некорректные права или команда изменения.\n");
                continue;
            }

            current = changed;
            print_permissions(current);
        }
        else if (choice == 3)
        {
            return;
        }
        else
        {
            printf("Некорректный пункт меню.\n");
        }
    }
}

int main(int argc, char *argv[])
{
    mode_t initial_mode;
    struct stat info;

    if (argc != 2)
    {
        fprintf(stderr, "Использование: %s <права|файл>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (parse_octal_permissions(argv[1], &initial_mode) ||
        parse_letter_permissions(argv[1], &initial_mode))
    {
        int descriptor = open("dummy_file", O_CREAT | O_WRONLY, 0666);

        if (descriptor == -1)
        {
            perror("Не удалось создать dummy_file");
            return EXIT_FAILURE;
        }
        if (close(descriptor) == -1)
        {
            perror("Не удалось закрыть dummy_file");
            return EXIT_FAILURE;
        }

        printf("Создан файл-болванка: dummy_file\n");
    }
    else
    {
        if (stat(argv[1], &info) == -1)
        {
            perror("Не удалось получить информацию о файле");
            return EXIT_FAILURE;
        }

        initial_mode = info.st_mode & 0777;
        print_file_comparison(info.st_mode);
    }

    print_permissions(initial_mode);
    run_menu(initial_mode);

    return EXIT_SUCCESS;
}
