#include "phonebook.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int compareContacts(const Contact *left, const Contact *right)
{
    int result = strcmp(left->surname, right->surname);

    if (result == 0)
        result = strcmp(left->name, right->name);
    if (result == 0)
        result = left->id - right->id;

    return result;
}

static void unlinkNode(Phonebook *phonebook, ContactNode *node)
{
    if (node->prev != NULL)
        node->prev->next = node->next;
    else
        phonebook->head = node->next;

    if (node->next != NULL)
        node->next->prev = node->prev;
    else
        phonebook->tail = node->prev;

    node->prev = NULL;
    node->next = NULL;
    phonebook->size--;
}

static void insertNodeSorted(Phonebook *phonebook, ContactNode *node)
{
    ContactNode *current = phonebook->head;

    while (current != NULL && compareContacts(&current->data, &node->data) <= 0)
        current = current->next;

    if (current == NULL) {
        node->prev = phonebook->tail;
        node->next = NULL;

        if (phonebook->tail != NULL)
            phonebook->tail->next = node;
        else
            phonebook->head = node;

        phonebook->tail = node;
    } else {
        node->next = current;
        node->prev = current->prev;

        if (current->prev != NULL)
            current->prev->next = node;
        else
            phonebook->head = node;

        current->prev = node;
    }

    phonebook->size++;
}

static int matches(const Contact *contact, SearchField field, const char *value)
{
    switch (field) {
    case SEARCH_NAME:
        return strcmp(contact->name, value) == 0;
    case SEARCH_SURNAME:
        return strcmp(contact->surname, value) == 0;
    case SEARCH_PHONE:
        return strcmp(contact->phone, value) == 0;
    default:
        return 0;
    }
}

void phonebookInit(Phonebook *phonebook)
{
    phonebook->head = NULL;
    phonebook->tail = NULL;
    phonebook->size = 0;
}

void phonebookClear(Phonebook *phonebook)
{
    ContactNode *current = phonebook->head;

    while (current != NULL) {
        ContactNode *next = current->next;
        free(current);
        current = next;
    }

    phonebookInit(phonebook);
}

int idExists(const Phonebook *phonebook, int id)
{
    const ContactNode *current;

    for (current = phonebook->head; current != NULL; current = current->next) {
        if (current->data.id == id)
            return 1;
    }

    return 0;
}

int generateId(const Phonebook *phonebook)
{
    int id;

    do {
        id = rand() % 9000 + 1000;
    } while (idExists(phonebook, id));

    return id;
}

ContactNode *findContactById(Phonebook *phonebook, int id)
{
    ContactNode *current;

    for (current = phonebook->head; current != NULL; current = current->next) {
        if (current->data.id == id)
            return current;
    }

    return NULL;
}

int insertContactSorted(Phonebook *phonebook, const Contact *contact)
{
    ContactNode *node;

    if (idExists(phonebook, contact->id))
        return 0;

    node = malloc(sizeof(*node));
    if (node == NULL)
        return 0;

    node->data = *contact;
    node->prev = NULL;
    node->next = NULL;
    insertNodeSorted(phonebook, node);
    return 1;
}

int removeContactById(Phonebook *phonebook, int id)
{
    ContactNode *node = findContactById(phonebook, id);

    if (node == NULL)
        return 0;

    unlinkNode(phonebook, node);
    free(node);
    return 1;
}

int updateContact(Phonebook *phonebook, int id, const char *name,
                  const char *surname, const char *phone)
{
    ContactNode *node = findContactById(phonebook, id);

    if (node == NULL)
        return 0;

    snprintf(node->data.name, sizeof(node->data.name), "%s", name);
    snprintf(node->data.surname, sizeof(node->data.surname), "%s", surname);
    snprintf(node->data.phone, sizeof(node->data.phone), "%s", phone);

    unlinkNode(phonebook, node);
    insertNodeSorted(phonebook, node);
    return 1;
}

size_t searchContacts(const Phonebook *phonebook, Contact result[],
                      size_t capacity, int count, ...)
{
    SearchField fields[3];
    const char *values[3];
    const ContactNode *current;
    size_t found = 0;
    va_list args;
    int i;

    if (count < 1 || count > 3)
        return 0;

    va_start(args, count);
    for (i = 0; i < count; i++) {
        fields[i] = (SearchField)va_arg(args, int);
        values[i] = va_arg(args, const char *);
    }
    va_end(args);

    for (current = phonebook->head; current != NULL; current = current->next) {
        int match = 1;

        for (i = 0; i < count; i++) {
            if (!matches(&current->data, fields[i], values[i])) {
                match = 0;
                break;
            }
        }

        if (match && found < capacity)
            result[found++] = current->data;
    }

    return found;
}

int loadContacts(Phonebook *phonebook, const char *filename)
{
    FILE *file = fopen(filename, "r");
    char line[256];
    int loaded = 0;

    if (file == NULL)
        return 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        Contact contact;

        if (sscanf(line, "%d;%49[^;];%49[^;];%19[^\n]", &contact.id,
                   contact.name, contact.surname, contact.phone) == 4 &&
            insertContactSorted(phonebook, &contact)) {
            loaded++;
        }
    }

    fclose(file);
    return loaded;
}

int saveContacts(const Phonebook *phonebook, const char *filename)
{
    const ContactNode *current;
    FILE *file = fopen(filename, "w");

    if (file == NULL)
        return 0;

    for (current = phonebook->head; current != NULL; current = current->next) {
        fprintf(file, "%d;%s;%s;%s\n", current->data.id, current->data.name,
                current->data.surname, current->data.phone);
    }

    if (fclose(file) != 0)
        return 0;
    return 1;
}

void addContact(Phonebook *phonebook)
{
    Contact contact;

    contact.id = generateId(phonebook);
    printf("Имя: ");
    scanf("%49s", contact.name);
    printf("Фамилия: ");
    scanf("%49s", contact.surname);
    printf("Телефон: ");
    scanf("%19s", contact.phone);

    if (insertContactSorted(phonebook, &contact))
        printf("Контакт добавлен! ID: %d\n", contact.id);
    else
        printf("Не удалось добавить контакт.\n");
}

void editContact(Phonebook *phonebook)
{
    ContactNode *node;
    char name[NAME_LEN];
    char surname[SURNAME_LEN];
    char phone[PHONE_LEN];
    int id;

    printf("Введите ID контакта для редактирования: ");
    if (scanf("%d", &id) != 1)
        return;

    node = findContactById(phonebook, id);
    if (node == NULL) {
        printf("Контакт не найден.\n");
        return;
    }

    printf("%d | %s %s | %s\n", node->data.id, node->data.name,
           node->data.surname, node->data.phone);
    printf("Новое имя: ");
    scanf("%49s", name);
    printf("Новая фамилия: ");
    scanf("%49s", surname);
    printf("Новый телефон: ");
    scanf("%19s", phone);

    updateContact(phonebook, id, name, surname, phone);
    printf("Контакт изменён!\n");
}

void deleteContact(Phonebook *phonebook)
{
    int id;

    printf("Введите ID контакта для удаления: ");
    if (scanf("%d", &id) != 1)
        return;

    if (removeContactById(phonebook, id))
        printf("Контакт удалён!\n");
    else
        printf("Контакт не найден.\n");
}

void printContacts(const Phonebook *phonebook)
{
    const ContactNode *current;

    if (phonebook->head == NULL) {
        printf("Контактов нет!\n");
        return;
    }

    for (current = phonebook->head; current != NULL; current = current->next) {
        printf("%d | %s %s | %s\n", current->data.id, current->data.name,
               current->data.surname, current->data.phone);
    }
}

void searchMenu(const Phonebook *phonebook)
{
    SearchField fields[3];
    char values[3][SURNAME_LEN];
    Contact *result;
    size_t found;
    int count;
    int i;

    printf("Количество критериев (1-3): ");
    if (scanf("%d", &count) != 1 || count < 1 || count > 3) {
        printf("Некорректное количество критериев.\n");
        return;
    }

    for (i = 0; i < count; i++) {
        int choice;

        printf("Критерий %d (1 — имя, 2 — фамилия, 3 — телефон): ", i + 1);
        if (scanf("%d", &choice) != 1 || choice < SEARCH_NAME ||
            choice > SEARCH_PHONE) {
            printf("Некорректный критерий.\n");
            return;
        }
        fields[i] = (SearchField)choice;
        printf("Введите значение: ");
        scanf("%49s", values[i]);
    }

    result = malloc(phonebook->size * sizeof(*result));
    if (result == NULL && phonebook->size != 0) {
        printf("Недостаточно памяти.\n");
        return;
    }

    if (count == 1)
        found = searchContacts(phonebook, result, phonebook->size, 1,
                               fields[0], values[0]);
    else if (count == 2)
        found = searchContacts(phonebook, result, phonebook->size, 2,
                               fields[0], values[0], fields[1], values[1]);
    else
        found = searchContacts(phonebook, result, phonebook->size, 3,
                               fields[0], values[0], fields[1], values[1],
                               fields[2], values[2]);

    if (found == 0) {
        printf("Контакты не найдены.\n");
    } else {
        printf("Найденные контакты:\n");
        for (size_t index = 0; index < found; index++) {
            printf("%d | %s %s | %s\n", result[index].id,
                   result[index].name, result[index].surname,
                   result[index].phone);
        }
    }

    free(result);
}
