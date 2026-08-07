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

void searchContacts(Phonebook contacts[], int count, ...)
{
    SearchField fields[3];
    const char *values[3];

    va_list args;
    va_start(args, count);

    for (int i = 0; i < count; i++)
    {
        fields[i] = va_arg(args, SearchField);
        values[i] = va_arg(args, const char *);
    }

    va_end(args);

    int found = 0;

    for (int i = 0; i < MAX_CONTACTS; i++)
    {
        if (!contacts[i].used)
            continue;
        
        int match = 1;

        for (int j = 0; j < count; j ++)
        {
            switch (fields[j])
            {
                case SEARCH_NAME:
                    if (strcmp(contacts[i].name, values[j]) != 0)
                        match = 0;
                    break;

                case SEARCH_SURNAME:
                    if (strcmp(contacts[i].surname, values[j]) != 0)
                        match = 0;
                    break;

                case SEARCH_PHONE:
                    if (strcmp(contacts[i].phone, values[j]) != 0)
                        match = 0;
                    break;
            }

            if (!match)
                break;
        }

        if (match)
        {
            printf("%d | %s %s | %s\n",
                    contacts[i].id,
                    contacts[i].name,
                    contacts[i].surname,
                    contacts[i].phone);
            found = 1;
        }
    }

    if (!found)
        printf("Контакты не найдены.\n");
}

void searchMenu(Phonebook contacts[])
{
    int count;

    printf("\n===== ПОИСК КОНТАКТА =====\n");

    printf("Количество критериев (1-3): ");
    scanf("%d", &count);

    if (count < 1 || count > 3)
    {
        printf("Некорректное количество критериев.\n");
        return;
    }

    SearchField fields[3];
    char values[3][PHONE_LEN];

    for (int i = 0; i < count; i++)
    {
        printf("\nКритерий %d:\n", i + 1);
        printf("1. Имя\n");
        printf("2. Фамилия\n");
        printf("3. Телефон\n");
        printf("Выберите поле: ");

        int choice;
        scanf("%d", &choice);

        if (choice < SEARCH_NAME || choice > SEARCH_PHONE)
        {
            printf("Некорректный критерий.\n");
            return;
        }

        fields[i] = choice;

        printf("Введите значение: ");
        scanf("%19s", values[i]);
    }

    if (count == 1)
    {
        searchContacts(
            contacts,
            1,
            fields[0], values[0]
        );
    }
    else if (count == 2)
    {
        searchContacts(
            contacts,
            2,
            fields[0], values[0],
            fields[1], values[1]
        );
    }
    else if (count == 3)
    {
        searchContacts(
            contacts,
            3,
            fields[0], values[0],
            fields[1], values[1],
            fields[2], values[2]
        );
    }
}

void loadContacts(Phonebook contacts[])
{
    FILE *file = fopen("contacts.txt", "r");

    if (file == NULL)
        return;

    int i = 0;

    while (fscanf(file, "%d;%29[^;];%29[^;];%19[^\n]\n",
                  &contacts[i].id,
                  contacts[i].name,
                  contacts[i].surname,
                  contacts[i].phone) == 4)
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