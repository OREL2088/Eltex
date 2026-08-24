#include "test_phonebook.h"

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

void testPhonebookInit(void)
{
    Phonebook phonebook;

    phonebookInit(&phonebook);

    CU_ASSERT_PTR_NULL(phonebook.head);
    CU_ASSERT_PTR_NULL(phonebook.tail);
    CU_ASSERT_EQUAL(phonebook.size, 0);
}

void testIdExists(void)
{
    Phonebook phonebook;
    Contact contact = makeContact(1001, "Иван", "Иванов", "111");

    phonebookInit(&phonebook);
    CU_ASSERT_TRUE(insertContactSorted(&phonebook, &contact));
    CU_ASSERT_TRUE(idExists(&phonebook, 1001));
    CU_ASSERT_FALSE(idExists(&phonebook, 9999));
    CU_ASSERT_FALSE(insertContactSorted(&phonebook, &contact));
    CU_ASSERT_EQUAL(phonebook.size, 1);

    phonebookClear(&phonebook);
}

void testSortedInsertion(void)
{
    Phonebook phonebook;
    Contact petrov = makeContact(1002, "Пётр", "Петров", "222");
    Contact ivanov = makeContact(1001, "Иван", "Иванов", "111");
    Contact alexeev = makeContact(1003, "Анна", "Алексеев", "333");

    phonebookInit(&phonebook);
    CU_ASSERT_TRUE(insertContactSorted(&phonebook, &petrov));
    CU_ASSERT_TRUE(insertContactSorted(&phonebook, &ivanov));
    CU_ASSERT_TRUE(insertContactSorted(&phonebook, &alexeev));

    CU_ASSERT_EQUAL(phonebook.size, 3);
    CU_ASSERT_STRING_EQUAL(phonebook.head->data.surname, "Алексеев");
    CU_ASSERT_STRING_EQUAL(phonebook.head->next->data.surname, "Иванов");
    CU_ASSERT_STRING_EQUAL(phonebook.tail->data.surname, "Петров");
    CU_ASSERT_PTR_NULL(phonebook.head->prev);
    CU_ASSERT_PTR_NULL(phonebook.tail->next);
    CU_ASSERT_PTR_EQUAL(phonebook.head->next->prev, phonebook.head);
    CU_ASSERT_PTR_EQUAL(phonebook.tail->prev->next, phonebook.tail);

    phonebookClear(&phonebook);
}

void testUpdateReordersList(void)
{
    Phonebook phonebook;
    Contact first = makeContact(1001, "Иван", "Иванов", "111");
    Contact second = makeContact(1002, "Пётр", "Петров", "222");

    phonebookInit(&phonebook);
    CU_ASSERT_TRUE(insertContactSorted(&phonebook, &first));
    CU_ASSERT_TRUE(insertContactSorted(&phonebook, &second));
    CU_ASSERT_TRUE(updateContact(&phonebook, 1002, "Пётр", "Алексеев", "999"));

    CU_ASSERT_EQUAL(phonebook.head->data.id, 1002);
    CU_ASSERT_STRING_EQUAL(phonebook.head->data.surname, "Алексеев");
    CU_ASSERT_STRING_EQUAL(phonebook.head->data.phone, "999");
    CU_ASSERT_PTR_EQUAL(phonebook.head->next->prev, phonebook.head);
    CU_ASSERT_FALSE(updateContact(&phonebook, 9999, "Нет", "Нет", "000"));

    phonebookClear(&phonebook);
}

void testRemoveContact(void)
{
    Phonebook phonebook;
    Contact first = makeContact(1001, "Иван", "Иванов", "111");
    Contact second = makeContact(1002, "Пётр", "Петров", "222");

    phonebookInit(&phonebook);
    CU_ASSERT_TRUE(insertContactSorted(&phonebook, &first));
    CU_ASSERT_TRUE(insertContactSorted(&phonebook, &second));
    CU_ASSERT_TRUE(removeContactById(&phonebook, 1001));

    CU_ASSERT_EQUAL(phonebook.size, 1);
    CU_ASSERT_PTR_EQUAL(phonebook.head, phonebook.tail);
    CU_ASSERT_PTR_NULL(phonebook.head->prev);
    CU_ASSERT_PTR_NULL(phonebook.tail->next);
    CU_ASSERT_FALSE(removeContactById(&phonebook, 9999));

    phonebookClear(&phonebook);
}

void testSearchByTwoFields(void)
{
    Phonebook phonebook;
    Contact result[3];
    Contact first = makeContact(1001, "Иван", "Иванов", "111");
    Contact second = makeContact(1002, "Пётр", "Иванов", "222");
    size_t found;

    phonebookInit(&phonebook);
    CU_ASSERT_TRUE(insertContactSorted(&phonebook, &first));
    CU_ASSERT_TRUE(insertContactSorted(&phonebook, &second));

    found = searchContacts(&phonebook, result, 3, 2,
                           SEARCH_SURNAME, "Иванов", SEARCH_PHONE, "222");
    CU_ASSERT_EQUAL(found, 1);
    CU_ASSERT_EQUAL(result[0].id, 1002);
    CU_ASSERT_STRING_EQUAL(result[0].name, "Пётр");

    phonebookClear(&phonebook);
}

void testSearchNotFound(void)
{
    Phonebook phonebook;
    Contact result[1];
    Contact contact = makeContact(1001, "Иван", "Иванов", "111");
    size_t found;

    phonebookInit(&phonebook);
    CU_ASSERT_TRUE(insertContactSorted(&phonebook, &contact));

    found = searchContacts(&phonebook, result, 1, 1,
                           SEARCH_SURNAME, "Петров");
    CU_ASSERT_EQUAL(found, 0);

    phonebookClear(&phonebook);
}
