#include "phonebook.h"

#include <stdlib.h>
#include <string.h>

static int validText(const char *text, size_t capacity)
{
    return text != NULL && text[0] != '\0' &&
           memchr(text, '\0', capacity) != NULL;
}

static int validContact(const Contact *contact)
{
    return contact != NULL && validText(contact->name, NAME_LEN) &&
           validText(contact->surname, SURNAME_LEN) &&
           validText(contact->phone, PHONE_LEN);
}

static int compareContacts(const Contact *left, const Contact *right)
{
    int result = strcmp(left->surname, right->surname);

    if (result == 0)
        result = strcmp(left->name, right->name);
    if (result == 0) {
        if (left->id < right->id)
            return -1;
        if (left->id > right->id)
            return 1;
    }
    return result;
}

static ContactNode *findNodeById(Phonebook *phonebook, int id)
{
    ContactNode *node;

    for (node = phonebook->head; node != NULL; node = node->next) {
        if (node->contact.id == id)
            return node;
    }
    return NULL;
}

static const ContactNode *findConstNodeById(const Phonebook *phonebook, int id)
{
    const ContactNode *node;

    for (node = phonebook->head; node != NULL; node = node->next) {
        if (node->contact.id == id)
            return node;
    }
    return NULL;
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

    while (current != NULL &&
           compareContacts(&current->contact, &node->contact) <= 0)
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
        node->prev = current->prev;
        node->next = current;
        if (current->prev != NULL)
            current->prev->next = node;
        else
            phonebook->head = node;
        current->prev = node;
    }
    phonebook->size++;
}

static int validCriterion(const SearchCriterion *criterion)
{
    size_t capacity;

    if (criterion == NULL)
        return 0;
    switch (criterion->field) {
    case SEARCH_NAME:
        capacity = NAME_LEN;
        break;
    case SEARCH_SURNAME:
        capacity = SURNAME_LEN;
        break;
    case SEARCH_PHONE:
        capacity = PHONE_LEN;
        break;
    default:
        return 0;
    }
    return validText(criterion->value, capacity);
}

static int matches(const Contact *contact, const SearchCriterion *criterion)
{
    switch (criterion->field) {
    case SEARCH_NAME:
        return strcmp(contact->name, criterion->value) == 0;
    case SEARCH_SURNAME:
        return strcmp(contact->surname, criterion->value) == 0;
    case SEARCH_PHONE:
        return strcmp(contact->phone, criterion->value) == 0;
    default:
        return 0;
    }
}

PhonebookStatus phonebookInit(Phonebook *phonebook)
{
    if (phonebook == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    phonebook->head = NULL;
    phonebook->tail = NULL;
    phonebook->size = 0;
    return PHONEBOOK_OK;
}

PhonebookStatus phonebookClear(Phonebook *phonebook)
{
    ContactNode *node;

    if (phonebook == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    node = phonebook->head;
    while (node != NULL) {
        ContactNode *next = node->next;

        free(node);
        node = next;
    }
    return phonebookInit(phonebook);
}

PhonebookStatus phonebookGenerateId(const Phonebook *phonebook, int *id)
{
    int candidate;

    if (phonebook == NULL || id == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    if (phonebook->size >= 9000U)
        return PHONEBOOK_ID_SPACE_EXHAUSTED;
    do {
        candidate = rand() % 9000 + 1000;
    } while (findConstNodeById(phonebook, candidate) != NULL);
    *id = candidate;
    return PHONEBOOK_OK;
}

PhonebookStatus phonebookFindById(const Phonebook *phonebook, int id,
                                  const Contact **contact)
{
    const ContactNode *node;

    if (phonebook == NULL || contact == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    node = findConstNodeById(phonebook, id);
    *contact = node == NULL ? NULL : &node->contact;
    return node == NULL ? PHONEBOOK_NOT_FOUND : PHONEBOOK_OK;
}

PhonebookStatus phonebookInsert(Phonebook *phonebook, const Contact *contact)
{
    ContactNode *node;

    if (phonebook == NULL || !validContact(contact))
        return PHONEBOOK_INVALID_ARGUMENT;
    if (findNodeById(phonebook, contact->id) != NULL)
        return PHONEBOOK_DUPLICATE_ID;
    node = malloc(sizeof(*node));
    if (node == NULL)
        return PHONEBOOK_NO_MEMORY;
    node->contact = *contact;
    node->prev = NULL;
    node->next = NULL;
    insertNodeSorted(phonebook, node);
    return PHONEBOOK_OK;
}

PhonebookStatus phonebookRemove(Phonebook *phonebook, int id)
{
    ContactNode *node;

    if (phonebook == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    node = findNodeById(phonebook, id);
    if (node == NULL)
        return PHONEBOOK_NOT_FOUND;
    unlinkNode(phonebook, node);
    free(node);
    return PHONEBOOK_OK;
}

PhonebookStatus phonebookUpdate(Phonebook *phonebook, int id,
                                const char *name, const char *surname,
                                const char *phone)
{
    ContactNode *node;

    if (phonebook == NULL || !validText(name, NAME_LEN) ||
        !validText(surname, SURNAME_LEN) || !validText(phone, PHONE_LEN))
        return PHONEBOOK_INVALID_ARGUMENT;
    node = findNodeById(phonebook, id);
    if (node == NULL)
        return PHONEBOOK_NOT_FOUND;

    strcpy(node->contact.name, name);
    strcpy(node->contact.surname, surname);
    strcpy(node->contact.phone, phone);
    unlinkNode(phonebook, node);
    insertNodeSorted(phonebook, node);
    return PHONEBOOK_OK;
}

PhonebookStatus phonebookSearch(const Phonebook *phonebook,
                                const SearchCriterion criteria[],
                                size_t criteria_count, Contact result[],
                                size_t capacity, size_t *result_count)
{
    const ContactNode *node;
    size_t index;

    if (phonebook == NULL || criteria == NULL || criteria_count == 0 ||
        criteria_count > 3 || result_count == NULL ||
        (capacity > 0 && result == NULL))
        return PHONEBOOK_INVALID_ARGUMENT;
    for (index = 0; index < criteria_count; index++) {
        if (!validCriterion(&criteria[index]))
            return PHONEBOOK_INVALID_ARGUMENT;
    }

    *result_count = 0;
    for (node = phonebook->head; node != NULL; node = node->next) {
        int match = 1;

        for (index = 0; index < criteria_count; index++) {
            if (!matches(&node->contact, &criteria[index])) {
                match = 0;
                break;
            }
        }
        if (match) {
            if (*result_count >= capacity)
                return PHONEBOOK_RESULT_TOO_SMALL;
            result[(*result_count)++] = node->contact;
        }
    }
    return PHONEBOOK_OK;
}

PhonebookStatus phonebookForEach(const Phonebook *phonebook,
                                 PhonebookVisitor visitor, void *context)
{
    const ContactNode *node;

    if (phonebook == NULL || visitor == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    for (node = phonebook->head; node != NULL; node = node->next) {
        PhonebookStatus status = visitor(&node->contact, context);

        if (status != PHONEBOOK_OK)
            return status;
    }
    return PHONEBOOK_OK;
}
