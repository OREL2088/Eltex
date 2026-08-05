#include "phonebook.h"

int main ()
{
    Phonebook contacts[MAX_CONTACTS] = {0};
    loadContacts(contacts);
    int choice;
    srand(time(NULL));

    do {
        printf("\n");
        printf("============МЕНЮ============\n"
            "\n"
            "1. Вывести список контактов;\n"
            "2. Добавить контакт;\n"
            "3. Редактировать контакт;\n"
            "4. Удалить контакт;\n"
            "5. Выход.\n\n"
            "Выберите действие: ");
        scanf ("%d", &choice);
        printf("\n");

        switch (choice)
        {
            case 1:
                printContacts(contacts);
                break;
            case 2:
                addContact(contacts);
                saveContacts(contacts);
                break;
            case 3:
                editContact(contacts);
                saveContacts(contacts);
                break;
            case 4:
                deleteContact(contacts);
                saveContacts(contacts);
                break;
            case 5:
                printf("До свидания!\n");
                break;
            default:
                printf("Неверный пункт меню!\n");
        }

    } while (choice != 5);

    return 0;
}