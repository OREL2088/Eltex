#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#include <stddef.h>

#define NAME_LEN 50
#define SURNAME_LEN 50
#define PHONE_LEN 20

typedef struct {
    int id;
    char name[NAME_LEN];
    char surname[SURNAME_LEN];
    char phone[PHONE_LEN];
} Contact;

typedef struct ContactNode {
    Contact contact;
    struct ContactNode *prev;
    struct ContactNode *next;
} ContactNode;

typedef struct {
    ContactNode *head;
    ContactNode *tail;
    size_t size;
} Phonebook;

typedef enum {
    PHONEBOOK_OK = 0,
    PHONEBOOK_INVALID_ARGUMENT,
    PHONEBOOK_DUPLICATE_ID,
    PHONEBOOK_NOT_FOUND,
    PHONEBOOK_NO_MEMORY,
    PHONEBOOK_ID_SPACE_EXHAUSTED,
    PHONEBOOK_RESULT_TOO_SMALL
} PhonebookStatus;

typedef enum {
    SEARCH_NAME = 1,
    SEARCH_SURNAME,
    SEARCH_PHONE
} SearchField;

typedef struct {
    SearchField field;
    const char *value;
} SearchCriterion;

typedef PhonebookStatus (*PhonebookVisitor)(const Contact *contact,
                                            void *context);

PhonebookStatus phonebookInit(Phonebook *phonebook);
PhonebookStatus phonebookClear(Phonebook *phonebook);

PhonebookStatus phonebookGenerateId(const Phonebook *phonebook, int *id);
PhonebookStatus phonebookFindById(const Phonebook *phonebook, int id,
                                  const Contact **contact);
PhonebookStatus phonebookInsert(Phonebook *phonebook, const Contact *contact);
PhonebookStatus phonebookRemove(Phonebook *phonebook, int id);
PhonebookStatus phonebookUpdate(Phonebook *phonebook, int id,
                                const char *name, const char *surname,
                                const char *phone);

PhonebookStatus phonebookSearch(const Phonebook *phonebook,
                                const SearchCriterion criteria[],
                                size_t criteria_count, Contact result[],
                                size_t capacity, size_t *result_count);
PhonebookStatus phonebookForEach(const Phonebook *phonebook,
                                 PhonebookVisitor visitor, void *context);

#endif
