#include "calculator.h"
#include <stdio.h>

int main(void)
{
    const Command commands[] =
    {
        {"Addition", add},
        {"Subtraction", subtract},
        {"Multiplication", multiply},
        {"Division", divide}
    };

    const int command_count = (int)(sizeof(commands) / sizeof(commands[0]));

    int choice;
    double a;
    double b;
    double result;

    while (1)
    {
        printf("\n=== Calculator ===\n");

        for (int i = 0; i < command_count; i++)
        {
            printf("%d. %s\n", i + 1, commands[i].name);
        }

        printf("0. Exit\n");
        printf("Choose action: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input.\n");
            return 1;
        }

        if (choice == 0)
        {
            printf("Goodbye!\n");
            break;
        }

        if (choice < 1 || choice > command_count)
        {
            printf("Unknown command.\n");
            continue;
        }

        printf("Enter first number: ");
        if (scanf("%lf", &a) != 1)
        {
            printf("Invalid input.\n");
            return 1;
        }

        printf("Enter second number: ");
        if (scanf("%lf", &b) != 1)
        {
            printf("Invalid input.\n");
            return 1;
        }

        if (commands[choice - 1].function == divide && b == 0)
        {
            printf("Error: division by zero.\n");
            continue;
        }

        result = commands[choice - 1].function(a, b);

        printf("Result: %.2f\n", result);
    }

    return 0;
}
