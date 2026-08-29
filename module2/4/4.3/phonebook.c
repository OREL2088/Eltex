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

static const PhonebookNode *findById(const PhonebookNode *node, int id)
{
    if (node == NULL)
        return NULL;
    if (node->contact.id == id)
        return node;
    const PhonebookNode *found = findById(node->left, id);
    return found ? found : findById(node->right, id);
}

static void insertNode(PhonebookNode **root, PhonebookNode *node)
{
    while (*root != NULL) {
        if (compareContacts(&node->contact, &(*root)->contact) < 0)
            root = &(*root)->left;
        else
            root = &(*root)->right;
    }
    *root = node;
}

static PhonebookNode *detachNode(PhonebookNode **root, const Contact *contact)
{
    PhonebookNode *node;

    while (*root != NULL) {
        int comparison = compareContacts(contact, &(*root)->contact);

        if (comparison < 0)
            root = &(*root)->left;
        else if (comparison > 0)
            root = &(*root)->right;
        else
            break;
    }
    if (*root == NULL)
        return NULL;

    node = *root;
    if (node->left == NULL) {
        *root = node->right;
    } else if (node->right == NULL) {
        *root = node->left;
    } else {
        PhonebookNode **successor_link = &node->right;
        PhonebookNode *successor;

        while ((*successor_link)->left != NULL)
            successor_link = &(*successor_link)->left;
        successor = *successor_link;
        *successor_link = successor->right;
        successor->left = node->left;
        successor->right = node->right;
        *root = successor;
    }
    node->left = NULL;
    node->right = NULL;
    return node;
}

static void clearTree(PhonebookNode *node)
{
    if (node == NULL)
        return;
    clearTree(node->left);
    clearTree(node->right);
    free(node);
}

static size_t treeToVine(PhonebookNode *pseudo_root)
{
    PhonebookNode *tail = pseudo_root;
    PhonebookNode *rest = tail->right;
    size_t count = 0;

    while (rest != NULL) {
        if (rest->left == NULL) {
            tail = rest;
            rest = rest->right;
            count++;
        } else {
            PhonebookNode *child = rest->left;

            rest->left = child->right;
            child->right = rest;
            tail->right = child;
            rest = child;
        }
    }
    return count;
}

static void compressVine(PhonebookNode *pseudo_root, size_t count)
{
    PhonebookNode *scanner = pseudo_root;
    size_t index;

    for (index = 0; index < count; index++) {
        PhonebookNode *child = scanner->right;
        PhonebookNode *grandchild;

        if (child == NULL || child->right == NULL)
            return;
        grandchild = child->right;
        scanner->right = grandchild;
        child->right = grandchild->left;
        grandchild->left = child;
        scanner = grandchild;
    }
}

static PhonebookStatus registerChange(Phonebook *phonebook)
{
    if (++phonebook->changes_since_balance >= PHONEBOOK_BALANCE_INTERVAL)
        return phonebookBalance(phonebook);
    return PHONEBOOK_OK;
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

static void searchTree(const PhonebookNode *node,
                       const SearchCriterion criteria[], size_t criteria_count,
                       Contact result[], size_t capacity, size_t *result_count)
{
    size_t index;
    int match = 1;

    if (node == NULL)
        return;
    searchTree(node->left, criteria, criteria_count, result, capacity,
               result_count);
    for (index = 0; index < criteria_count; index++) {
        if (!matches(&node->contact, &criteria[index])) {
            match = 0;
            break;
        }
    }
    if (match && *result_count < capacity)
        result[(*result_count)++] = node->contact;
    searchTree(node->right, criteria, criteria_count, result, capacity,
               result_count);
}

static PhonebookStatus visitTree(const PhonebookNode *node,
                                 PhonebookVisitor visitor, void *context)
{
    PhonebookStatus status;

    if (node == NULL)
        return PHONEBOOK_OK;
    status = visitTree(node->left, visitor, context);
    if (status != PHONEBOOK_OK)
        return status;
    status = visitor(&node->contact, context);
    if (status != PHONEBOOK_OK)
        return status;
    return visitTree(node->right, visitor, context);
}

static size_t nodeHeight(const PhonebookNode *node)
{
    size_t left_height;
    size_t right_height;

    if (node == NULL)
        return 0;
    left_height = nodeHeight(node->left);
    right_height = nodeHeight(node->right);
    return 1 + (left_height > right_height ? left_height : right_height);
}

PhonebookStatus phonebookInit(Phonebook *phonebook)
{
    if (phonebook == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    phonebook->root = NULL;
    phonebook->size = 0;
    phonebook->changes_since_balance = 0;
    return PHONEBOOK_OK;
}

PhonebookStatus phonebookClear(Phonebook *phonebook)
{
    if (phonebook == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    clearTree(phonebook->root);
    return phonebookInit(phonebook);
}

PhonebookStatus phonebookGenerateId(const Phonebook *phonebook, int *id)
{
    int candidate;

    if (phonebook == NULL || id == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    if (phonebook->size >= 9000U)
        return PHONEBOOK_NO_MEMORY;
    do {
        candidate = rand() % 9000 + 1000;
    } while (findById(phonebook->root, candidate) != NULL);
    *id = candidate;
    return PHONEBOOK_OK;
}

PhonebookStatus phonebookFindById(const Phonebook *phonebook, int id,
                                  const Contact **contact)
{
    const PhonebookNode *node;

    if (phonebook == NULL || contact == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    node = findById(phonebook->root, id);
    *contact = node == NULL ? NULL : &node->contact;
    if (node == NULL)
        return PHONEBOOK_NOT_FOUND;
    return PHONEBOOK_OK;
}

PhonebookStatus phonebookInsert(Phonebook *phonebook, const Contact *contact)
{
    PhonebookNode *node;

    if (phonebook == NULL || !validContact(contact))
        return PHONEBOOK_INVALID_ARGUMENT;
    if (findById(phonebook->root, contact->id) != NULL)
        return PHONEBOOK_DUPLICATE_ID;
    node = malloc(sizeof(*node));
    if (node == NULL)
        return PHONEBOOK_NO_MEMORY;
    node->contact = *contact;
    node->left = NULL;
    node->right = NULL;
    insertNode(&phonebook->root, node);
    phonebook->size++;
    return registerChange(phonebook);
}

PhonebookStatus phonebookRemove(Phonebook *phonebook, int id)
{
    const PhonebookNode *found;
    Contact key;
    PhonebookNode *node;

    if (phonebook == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    found = findById(phonebook->root, id);
    if (found == NULL)
        return PHONEBOOK_NOT_FOUND;
    key = found->contact;
    node = detachNode(&phonebook->root, &key);
    free(node);
    phonebook->size--;
    return registerChange(phonebook);
}

PhonebookStatus phonebookUpdate(Phonebook *phonebook, int id,
                                const char *name, const char *surname,
                                const char *phone)
{
    const PhonebookNode *found;
    Contact updated;
    PhonebookNode *node;

    if (phonebook == NULL || !validText(name, NAME_LEN) ||
        !validText(surname, SURNAME_LEN) || !validText(phone, PHONE_LEN))
        return PHONEBOOK_INVALID_ARGUMENT;
    found = findById(phonebook->root, id);
    if (found == NULL)
        return PHONEBOOK_NOT_FOUND;
    updated = found->contact;
    strcpy(updated.name, name);
    strcpy(updated.surname, surname);
    strcpy(updated.phone, phone);
    node = detachNode(&phonebook->root, &found->contact);
    node->contact = updated;
    insertNode(&phonebook->root, node);
    return registerChange(phonebook);
}

PhonebookStatus phonebookSearch(const Phonebook *phonebook,
                                const SearchCriterion criteria[],
                                size_t criteria_count, Contact result[],
                                size_t capacity, size_t *result_count)
{
    size_t index;

    if (phonebook == NULL || criteria == NULL || criteria_count == 0 ||
        criteria_count > 3 || result_count == NULL ||
        (capacity > 0 && result == NULL))
        return PHONEBOOK_INVALID_ARGUMENT;
    for (index = 0; index < criteria_count; index++) {
        if (criteria[index].field < SEARCH_NAME ||
            criteria[index].field > SEARCH_PHONE ||
            criteria[index].value == NULL)
            return PHONEBOOK_INVALID_ARGUMENT;
    }
    *result_count = 0;
    searchTree(phonebook->root, criteria, criteria_count, result, capacity,
               result_count);
    return PHONEBOOK_OK;
}

PhonebookStatus phonebookForEach(const Phonebook *phonebook,
                                 PhonebookVisitor visitor, void *context)
{
    if (phonebook == NULL || visitor == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    return visitTree(phonebook->root, visitor, context);
}

PhonebookStatus phonebookBalance(Phonebook *phonebook)
{
    PhonebookNode pseudo_root = {0};
    size_t count;
    size_t full_size = 1;
    size_t leaves;

    if (phonebook == NULL)
        return PHONEBOOK_INVALID_ARGUMENT;
    pseudo_root.right = phonebook->root;
    count = treeToVine(&pseudo_root);
    while (full_size <= count + 1U)
        full_size *= 2U;
    full_size = full_size / 2U - 1U;
    leaves = count - full_size;
    compressVine(&pseudo_root, leaves);
    while (full_size > 1U) {
        full_size /= 2U;
        compressVine(&pseudo_root, full_size);
    }
    phonebook->root = pseudo_root.right;
    phonebook->changes_since_balance = 0;
    return PHONEBOOK_OK;
}

size_t phonebookHeight(const Phonebook *phonebook)
{
    return phonebook == NULL ? 0 : nodeHeight(phonebook->root);
}
