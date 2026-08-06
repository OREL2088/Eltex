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
            "2. Поиск контакта;\n"
            "3. Добавить контакт;\n"
            "4. Редактировать контакт;\n"
            "5. Удалить контакт;\n"
            "6. Выход.\n\n"
            "Выберите действие: ");
        scanf ("%d", &choice);
        printf("\n");

        switch (choice)
        {
            case 1:
                printContacts(contacts);
                break;
            case 2:
                searchMenu(contacts);
                break;
            case 3:
                addContact(contacts);
                saveContacts(contacts);
                break;
            case 4:
                editContact(contacts);
                saveContacts(contacts);
                break;
            case 5:
                deleteContact(contacts);
                saveContacts(contacts);
                break;
            case 6:
                printf("До свидания!\n");
                break;
            default:
                printf("Неверный пункт меню!\n");
        }

    } while (choice != 6);

    return 0;
}