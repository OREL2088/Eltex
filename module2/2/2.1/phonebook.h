#ifndef PHONEBOOK_H
#define PHONEBOOK_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <CUnit/Basic.h>

#define MAX_CONTACTS 100
#define NAME_LEN 20
#define SURNAME_LEN 20
#define PHONE_LEN 13

//Фамилия, имя, отчество, номер телефона, id 

typedef struct
{
    int id;
    char name[NAME_LEN];
    char surname[SURNAME_LEN];
    char phone[PHONE_LEN];
    int used;
} Phonebook;

typedef enum{
    SEARCH_NAME = 1,
    SEARCH_SURNAME,
    SEARCH_PHONE
} SearchField;

int generateId(Phonebook contacts[]);
int idExists(Phonebook contacts[], int id);

void addContact(Phonebook contacts[]);
void editContact(Phonebook contacts[]);
void deleteContact(Phonebook contacts[]);
void printContacts(Phonebook contacts[]);

void searchMenu(Phonebook contacts[]);
int searchContacts(Phonebook contacts[], Phonebook result[], int count, ...);

void loadContacts(Phonebook contacts[]);
void saveContacts(Phonebook contacts[]);
#endif