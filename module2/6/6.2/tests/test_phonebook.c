#include "test_phonebook.h"

#include "../storage.h"

#include <stdio.h>

static Contact makeContact(int id, const char *name, const char *surname,
                           const char *phone)
{
    Contact contact = {0};

    contact.id = id;
    snprintf(contact.name, sizeof(contact.name), "%s", name);
    snprintf(contact.surname, sizeof(contact.surname), "%s", surname);
    snprintf(contact.phone, sizeof(contact.phone), "%s", phone);
    return contact;
}

void testInitAndClearStatuses(void)
{
    Phonebook phonebook;

    CU_ASSERT_EQUAL(phonebookInit(NULL), PHONEBOOK_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(phonebookInit(&phonebook), PHONEBOOK_OK);
    CU_ASSERT_PTR_NULL(phonebook.head);
    CU_ASSERT_PTR_NULL(phonebook.tail);
    CU_ASSERT_EQUAL(phonebook.size, 0);
    CU_ASSERT_EQUAL(phonebookClear(&phonebook), PHONEBOOK_OK);
    CU_ASSERT_EQUAL(phonebookClear(NULL), PHONEBOOK_INVALID_ARGUMENT);
}

void testSortedInsertionAndDuplicate(void)
{
    Phonebook phonebook;
    Contact petrov = makeContact(1002, "Пётр", "Петров", "222");
    Contact ivanov = makeContact(1001, "Иван", "Иванов", "111");
    Contact alexeev = makeContact(1003, "Анна", "Алексеев", "333");

    phonebookInit(&phonebook);
    CU_ASSERT_EQUAL(phonebookInsert(&phonebook, &petrov), PHONEBOOK_OK);
    CU_ASSERT_EQUAL(phonebookInsert(&phonebook, &ivanov), PHONEBOOK_OK);
    CU_ASSERT_EQUAL(phonebookInsert(&phonebook, &alexeev), PHONEBOOK_OK);
    CU_ASSERT_EQUAL(phonebookInsert(&phonebook, &ivanov),
                    PHONEBOOK_DUPLICATE_ID);
    CU_ASSERT_EQUAL(phonebook.size, 3);
    CU_ASSERT_STRING_EQUAL(phonebook.head->contact.surname, "Алексеев");
    CU_ASSERT_STRING_EQUAL(phonebook.head->next->contact.surname, "Иванов");
    CU_ASSERT_STRING_EQUAL(phonebook.tail->contact.surname, "Петров");
    CU_ASSERT_PTR_NULL(phonebook.head->prev);
    CU_ASSERT_PTR_NULL(phonebook.tail->next);
    CU_ASSERT_PTR_EQUAL(phonebook.head->next->prev, phonebook.head);
    CU_ASSERT_PTR_EQUAL(phonebook.tail->prev->next, phonebook.tail);
    phonebookClear(&phonebook);
}

void testFindAndUpdate(void)
{
    Phonebook phonebook;
    Contact first = makeContact(1001, "Иван", "Иванов", "111");
    Contact second = makeContact(1002, "Пётр", "Петров", "222");
    const Contact *found = NULL;

    phonebookInit(&phonebook);
    phonebookInsert(&phonebook, &first);
    phonebookInsert(&phonebook, &second);
    CU_ASSERT_EQUAL(phonebookFindById(&phonebook, 1002, &found),
                    PHONEBOOK_OK);
    CU_ASSERT_PTR_NOT_NULL(found);
    CU_ASSERT_EQUAL(phonebookUpdate(&phonebook, 1002, "Пётр", "Алексеев",
                                    "999"), PHONEBOOK_OK);
    CU_ASSERT_EQUAL(phonebook.head->contact.id, 1002);
    CU_ASSERT_STRING_EQUAL(phonebook.head->contact.phone, "999");
    CU_ASSERT_EQUAL(phonebookUpdate(&phonebook, 9999, "Нет", "Нет", "000"),
                    PHONEBOOK_NOT_FOUND);
    CU_ASSERT_EQUAL(phonebookFindById(&phonebook, 9999, &found),
                    PHONEBOOK_NOT_FOUND);
    CU_ASSERT_PTR_NULL(found);
    phonebookClear(&phonebook);
}

void testRemoveStatusesAndLinks(void)
{
    Phonebook phonebook;
    Contact first = makeContact(1001, "Иван", "Иванов", "111");
    Contact second = makeContact(1002, "Пётр", "Петров", "222");

    phonebookInit(&phonebook);
    phonebookInsert(&phonebook, &first);
    phonebookInsert(&phonebook, &second);
    CU_ASSERT_EQUAL(phonebookRemove(&phonebook, 1001), PHONEBOOK_OK);
    CU_ASSERT_EQUAL(phonebookRemove(&phonebook, 1001), PHONEBOOK_NOT_FOUND);
    CU_ASSERT_EQUAL(phonebook.size, 1);
    CU_ASSERT_PTR_EQUAL(phonebook.head, phonebook.tail);
    CU_ASSERT_PTR_NULL(phonebook.head->prev);
    CU_ASSERT_PTR_NULL(phonebook.tail->next);
    phonebookClear(&phonebook);
}

void testSearchByTwoFields(void)
{
    Phonebook phonebook;
    Contact first = makeContact(1001, "Иван", "Иванов", "111");
    Contact second = makeContact(1002, "Пётр", "Иванов", "222");
    Contact result[2];
    SearchCriterion criteria[] = {
        {SEARCH_SURNAME, "Иванов"},
        {SEARCH_PHONE, "222"}
    };
    size_t found = 0;

    phonebookInit(&phonebook);
    phonebookInsert(&phonebook, &first);
    phonebookInsert(&phonebook, &second);
    CU_ASSERT_EQUAL(phonebookSearch(&phonebook, criteria, 2, result, 2,
                                    &found), PHONEBOOK_OK);
    CU_ASSERT_EQUAL(found, 1);
    CU_ASSERT_EQUAL(result[0].id, 1002);
    phonebookClear(&phonebook);
}

void testSearchCapacityStatus(void)
{
    Phonebook phonebook;
    Contact first = makeContact(1001, "Иван", "Иванов", "111");
    Contact second = makeContact(1002, "Пётр", "Иванов", "222");
    Contact result[1];
    SearchCriterion criterion = {SEARCH_SURNAME, "Иванов"};
    size_t found = 0;

    phonebookInit(&phonebook);
    phonebookInsert(&phonebook, &first);
    phonebookInsert(&phonebook, &second);
    CU_ASSERT_EQUAL(phonebookSearch(&phonebook, &criterion, 1, result, 1,
                                    &found), PHONEBOOK_RESULT_TOO_SMALL);
    CU_ASSERT_EQUAL(found, 1);
    phonebookClear(&phonebook);
}

void testInvalidArguments(void)
{
    Phonebook phonebook;
    Contact invalid = makeContact(1001, "", "Иванов", "111");
    SearchCriterion invalid_criterion = {(SearchField)99, "Иванов"};
    size_t found = 0;

    phonebookInit(&phonebook);
    CU_ASSERT_EQUAL(phonebookInsert(&phonebook, &invalid),
                    PHONEBOOK_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(phonebookRemove(NULL, 1001), PHONEBOOK_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(phonebookFindById(&phonebook, 1, NULL),
                    PHONEBOOK_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(phonebookSearch(&phonebook, NULL, 0, NULL, 0, &found),
                    PHONEBOOK_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(phonebookSearch(&phonebook, &invalid_criterion, 1, NULL,
                                    0, &found), PHONEBOOK_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(phonebookForEach(&phonebook, NULL, NULL),
                    PHONEBOOK_INVALID_ARGUMENT);
    phonebookClear(&phonebook);
}

void testStorageRoundTrip(void)
{
    const char *filename = "/tmp/eltex_phonebook_6_1_test.txt";
    Phonebook source;
    Phonebook loaded;
    Contact first = makeContact(1001, "Ivan", "Ivanov", "111");
    Contact second = makeContact(1002, "Petr", "Petrov", "222");
    const Contact *found = NULL;
    size_t loaded_count = 0;

    phonebookInit(&source);
    phonebookInit(&loaded);
    phonebookInsert(&source, &first);
    phonebookInsert(&source, &second);
    CU_ASSERT_EQUAL(storageSave(&source, filename), STORAGE_OK);
    CU_ASSERT_EQUAL(storageLoad(&loaded, filename, &loaded_count), STORAGE_OK);
    CU_ASSERT_EQUAL(loaded_count, 2);
    CU_ASSERT_EQUAL(phonebookFindById(&loaded, 1002, &found), PHONEBOOK_OK);
    CU_ASSERT_PTR_NOT_NULL(found);
    if (found != NULL)
        CU_ASSERT_STRING_EQUAL(found->surname, "Petrov");
    remove(filename);
    phonebookClear(&source);
    phonebookClear(&loaded);
}
