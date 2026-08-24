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
    Contact data;
    struct ContactNode *prev;
    struct ContactNode *next;
} ContactNode;

typedef struct {
    ContactNode *head;
    ContactNode *tail;
    size_t size;
} Phonebook;

typedef enum {
    SEARCH_NAME = 1,
    SEARCH_SURNAME,
    SEARCH_PHONE
} SearchField;

void phonebookInit(Phonebook *phonebook);
void phonebookClear(Phonebook *phonebook);

int idExists(const Phonebook *phonebook, int id);
int generateId(const Phonebook *phonebook);
ContactNode *findContactById(Phonebook *phonebook, int id);

int insertContactSorted(Phonebook *phonebook, const Contact *contact);
int removeContactById(Phonebook *phonebook, int id);
int updateContact(Phonebook *phonebook, int id, const char *name,
                  const char *surname, const char *phone);

size_t searchContacts(const Phonebook *phonebook, Contact result[],
                      size_t capacity, int count, ...);

int loadContacts(Phonebook *phonebook, const char *filename);
int saveContacts(const Phonebook *phonebook, const char *filename);

void addContact(Phonebook *phonebook);
void editContact(Phonebook *phonebook);
void deleteContact(Phonebook *phonebook);
void printContacts(const Phonebook *phonebook);
void searchMenu(const Phonebook *phonebook);

#endif
