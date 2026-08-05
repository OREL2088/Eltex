#include <stdio.h>
#include "phonebook.h"

int generateId(Phonebook contacts[])
{
    int id;

    do
    {
        id = rand() % 9000 + 1000;
    }
    while (idExists(contacts, id));

    return id;
}

int idExists(Phonebook contacts[], int id)
{
    for (int i = 0; i < MAX_CONTACTS; i++)
    {
        if (contacts[i].used && contacts[i].id == id)
            return 1;
    }

    return 0;
}

void addContact(Phonebook contacts[])
{
    for (int i = 0; i < MAX_CONTACTS; i++)
    {
        if(!contacts[i].used)
        {
            contacts[i].id = generateId(contacts);

            printf("Имя: ");
            scanf("%19s", contacts[i].name);

            printf("Фамилия: ");
            scanf("%19s", contacts[i].surname);

            printf("Телефон: ");
            scanf("%12s", contacts[i].phone);

            contacts[i].used = 1;
            printf("Контакт добавлен!\n");
            return;
        }
    }

    printf("Справочник заполнен!\n");
}

void editContact(Phonebook contacts[])
{
    int id;
    printf("Введите id контакта для редактирования:");
    scanf("%d", &id);

    for (int i = 0; i < MAX_CONTACTS; i++)
    {
        if (contacts[i].used && contacts[i].id == id)
        {
            printf("%d | %s %s | %s\n",
                    contacts[i].id,
                    contacts[i].name,
                    contacts[i].surname,
                    contacts[i].phone);

            printf("Новое имя: ");
            scanf("%19s", contacts[i].name);

            printf("Новая фамилия: ");
            scanf("%19s", contacts[i].surname);

            printf("Новый телефон: ");
            scanf("%13s", contacts[i].phone);

            printf("Контакт изменен!\n");
            return;
        }
    }

    printf("Контакт не найден :(\n");
}

void deleteContact(Phonebook contacts[])
{
    int id;
    printf("Введите id контакта для удаления:");
    scanf("%d", &id);

    for (int i = 0; i < MAX_CONTACTS; i++)
    {
        if (contacts[i].used && contacts[i].id == id)
        {
            contacts[i].used = 0;
            printf("Контакт удалён!\n");
            return;
        }
    }

    printf("Контакт не найден :(\n");
}

void printContacts(Phonebook contacts[])
{
    int found = 0;

    for (int i = 0; i < MAX_CONTACTS; i++)
    {
        if (contacts[i].used)
        {
            found = 1;
            printf("%d | %s %s | %s\n",
                contacts[i].id,
                contacts[i].name,
                contacts[i].surname,
                contacts[i].phone);
        }
        
    }

    if (!found)
    {
        printf("Контактов нет!\n");
    }
}

void loadContacts(Phonebook contacts[])
{
    FILE *file = fopen("contacts.txt", "r");

    if (file == NULL)
        return;

    int i = 0;

    while (fscanf(file, "%29[^;];%29[^;];%19[^\n]\n",
                  contacts[i].name,
                  contacts[i].surname,
                  contacts[i].phone) == 3)
    {
        contacts[i].used = 1;
        i++;
    }

    fclose(file);
}

void saveContacts(Phonebook contacts[])
{
    FILE *file = fopen("contacts.txt", "w");

    if (file == NULL)
    {
        printf("Ошибка открытия файла.\n");
        return;
    }

    for (int i = 0; i < MAX_CONTACTS; i++)
    {
        if (contacts[i].used)
        {
            fprintf(file, "%d;%s;%s;%s\n",
                    contacts[i].id,
                    contacts[i].name,
                    contacts[i].surname,
                    contacts[i].phone);
        }
    }

    fclose(file);
}