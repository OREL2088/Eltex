#include "ui.h"

#include "storage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int readLine(const char *prompt, char *buffer, size_t capacity)
{
    size_t length;
    int character;

    printf("%s", prompt);
    if (fgets(buffer, (int)capacity, stdin) == NULL)
        return 0;
    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
        return length > 1;
    }
    while ((character = getchar()) != '\n' && character != EOF) {
    }
    return 0;
}

static int readInt(const char *prompt, int *value)
{
    char line[64];
    char extra;

    return readLine(prompt, line, sizeof(line)) &&
           sscanf(line, "%d %c", value, &extra) == 1;
}

static void printFailure(PhonebookStatus status)
{
    printf("Ошибка: %s.\n", uiStatusMessage(status));
}

const char *uiStatusMessage(PhonebookStatus status)
{
    switch (status) {
    case PHONEBOOK_OK:
        return "операция выполнена";
    case PHONEBOOK_INVALID_ARGUMENT:
        return "некорректные данные";
    case PHONEBOOK_DUPLICATE_ID:
        return "контакт с таким ID уже существует";
    case PHONEBOOK_NOT_FOUND:
        return "контакт не найден";
    case PHONEBOOK_NO_MEMORY:
        return "недостаточно памяти";
    case PHONEBOOK_IO_ERROR:
        return "ошибка ввода-вывода";
    case PHONEBOOK_FORMAT_ERROR:
        return "ошибка формата данных";
    default:
        return "неизвестная ошибка";
    }
}

static PhonebookStatus printContact(const Contact *contact, void *context)
{
    (void)context;
    printf("%d | %s %s | %s\n", contact->id, contact->name,
           contact->surname, contact->phone);
    return PHONEBOOK_OK;
}

static PhonebookStatus showContacts(const Phonebook *phonebook)
{
    if (phonebook->size == 0) {
        printf("Контактов нет.\n");
        return PHONEBOOK_OK;
    }
    return phonebookForEach(phonebook, printContact, NULL);
}

static PhonebookStatus addContact(Phonebook *phonebook)
{
    Contact contact = {0};
    PhonebookStatus status;

    status = phonebookGenerateId(phonebook, &contact.id);
    if (status != PHONEBOOK_OK)
        return status;
    if (!readLine("Имя: ", contact.name, sizeof(contact.name)) ||
        !readLine("Фамилия: ", contact.surname, sizeof(contact.surname)) ||
        !readLine("Телефон: ", contact.phone, sizeof(contact.phone)))
        return PHONEBOOK_INVALID_ARGUMENT;
    status = phonebookInsert(phonebook, &contact);
    if (status == PHONEBOOK_OK)
        printf("Контакт добавлен. ID: %d\n", contact.id);
    return status;
}

static PhonebookStatus editContact(Phonebook *phonebook)
{
    char name[NAME_LEN];
    char surname[SURNAME_LEN];
    char phone[PHONE_LEN];
    const Contact *contact;
    PhonebookStatus status;
    int id;

    if (!readInt("ID контакта: ", &id))
        return PHONEBOOK_INVALID_ARGUMENT;
    status = phonebookFindById(phonebook, id, &contact);
    if (status != PHONEBOOK_OK)
        return status;
    printContact(contact, NULL);
    if (!readLine("Новое имя: ", name, sizeof(name)) ||
        !readLine("Новая фамилия: ", surname, sizeof(surname)) ||
        !readLine("Новый телефон: ", phone, sizeof(phone)))
        return PHONEBOOK_INVALID_ARGUMENT;
    status = phonebookUpdate(phonebook, id, name, surname, phone);
    if (status == PHONEBOOK_OK)
        printf("Контакт изменён.\n");
    return status;
}

static PhonebookStatus deleteContact(Phonebook *phonebook)
{
    int id;

    if (!readInt("ID контакта: ", &id))
        return PHONEBOOK_INVALID_ARGUMENT;
    PhonebookStatus status = phonebookRemove(phonebook, id);
    if (status == PHONEBOOK_OK)
        printf("Контакт удалён.\n");
    return status;
}

static PhonebookStatus searchContacts(const Phonebook *phonebook)
{
    SearchCriterion criteria[3];
    char values[3][NAME_LEN];
    Contact *result;
    PhonebookStatus status;
    size_t found = 0;
    int count;
    int index;

    if (!readInt("Количество критериев (1-3): ", &count) || count < 1 ||
        count > 3)
        return PHONEBOOK_INVALID_ARGUMENT;
    for (index = 0; index < count; index++) {
        int field;

        if (!readInt("Поле (1 — имя, 2 — фамилия, 3 — телефон): ",
                     &field) ||
            field < SEARCH_NAME || field > SEARCH_PHONE ||
            !readLine("Значение: ", values[index], sizeof(values[index])))
            return PHONEBOOK_INVALID_ARGUMENT;
        criteria[index].field = (SearchField)field;
        criteria[index].value = values[index];
    }
    result = phonebook->size == 0 ? NULL :
             malloc(phonebook->size * sizeof(*result));
    if (result == NULL && phonebook->size != 0)
        return PHONEBOOK_NO_MEMORY;
    status = phonebookSearch(phonebook, criteria, (size_t)count, result,
                             phonebook->size, &found);
    if (status == PHONEBOOK_OK && found == 0) {
        printf("Контакты не найдены.\n");
    } else if (status == PHONEBOOK_OK) {
        size_t result_index;

        for (result_index = 0; result_index < found; result_index++)
            printContact(&result[result_index], NULL);
    }
    free(result);
    return status;
}

PhonebookStatus uiRun(Phonebook *phonebook, const char *contacts_file)
{
    int choice = 0;

    if (phonebook == NULL || contacts_file == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    while (choice != 6) {
        PhonebookStatus status = PHONEBOOK_OK;
        int changed = 0;

        printf("\n============ МЕНЮ ============\n"
               "1. Вывести список контактов\n"
               "2. Поиск контакта\n"
               "3. Добавить контакт\n"
               "4. Редактировать контакт\n"
               "5. Удалить контакт\n"
               "6. Выход\n");
        if (!readInt("Выберите действие: ", &choice)) {
            printf("Некорректный ввод.\n");
            continue;
        }
        switch (choice) {
        case 1:
            status = showContacts(phonebook);
            break;
        case 2:
            status = searchContacts(phonebook);
            break;
        case 3:
            status = addContact(phonebook);
            changed = status == PHONEBOOK_OK;
            break;
        case 4:
            status = editContact(phonebook);
            changed = status == PHONEBOOK_OK;
            break;
        case 5:
            status = deleteContact(phonebook);
            changed = status == PHONEBOOK_OK;
            break;
        case 6:
            printf("До свидания!\n");
            break;
        default:
            printf("Такого пункта меню нет.\n");
        }
        if (status != PHONEBOOK_OK)
            printFailure(status);
        if (changed) {
            status = storageSave(phonebook, contacts_file);
            if (status != PHONEBOOK_OK)
                printFailure(status);
        }
    }
    return PHONEBOOK_OK;
}
