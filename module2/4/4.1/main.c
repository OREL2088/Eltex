#include "phonebook.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CONTACTS_FILE "contacts.txt"

int main(void)
{
    Phonebook phonebook;
    int choice = 0;

    phonebookInit(&phonebook);
    srand((unsigned int)time(NULL));
    loadContacts(&phonebook, CONTACTS_FILE);

    do {
        printf("\n============ МЕНЮ ============\n"
               "1. Вывести список контактов\n"
               "2. Поиск контакта\n"
               "3. Добавить контакт\n"
               "4. Редактировать контакт\n"
               "5. Удалить контакт\n"
               "6. Выход\n\n"
               "Выберите действие: ");

        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода.\n");
            break;
        }

        switch (choice) {
        case 1:
            printContacts(&phonebook);
            break;
        case 2:
            searchMenu(&phonebook);
            break;
        case 3:
            addContact(&phonebook);
            saveContacts(&phonebook, CONTACTS_FILE);
            break;
        case 4:
            editContact(&phonebook);
            saveContacts(&phonebook, CONTACTS_FILE);
            break;
        case 5:
            deleteContact(&phonebook);
            saveContacts(&phonebook, CONTACTS_FILE);
            break;
        case 6:
            printf("До свидания!\n");
            break;
        default:
            printf("Неверный пункт меню!\n");
        }
    } while (choice != 6);

    phonebookClear(&phonebook);
    return 0;
}
