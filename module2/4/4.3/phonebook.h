#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#include <stddef.h>

#define NAME_LEN 50
#define SURNAME_LEN 50
#define PHONE_LEN 20
#define PHONEBOOK_BALANCE_INTERVAL 10U

typedef struct {
    int id;
    char name[NAME_LEN];
    char surname[SURNAME_LEN];
    char phone[PHONE_LEN];
} Contact;

typedef struct PhonebookNode {
    Contact contact;
    struct PhonebookNode *left;
    struct PhonebookNode *right;
} PhonebookNode;

typedef struct {
    PhonebookNode *root;
    size_t size;
    size_t changes_since_balance;
} Phonebook;

typedef enum {
    PHONEBOOK_OK = 0,
    PHONEBOOK_INVALID_ARGUMENT,
    PHONEBOOK_DUPLICATE_ID,
    PHONEBOOK_NOT_FOUND,
    PHONEBOOK_NO_MEMORY,
    PHONEBOOK_IO_ERROR,
    PHONEBOOK_FORMAT_ERROR
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

PhonebookStatus phonebookBalance(Phonebook *phonebook);
size_t phonebookHeight(const Phonebook *phonebook);

#endif
