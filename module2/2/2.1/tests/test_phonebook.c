#include "test_phonebook.h"

void test_idExists(void)
{
    Phonebook contacts[MAX_CONTACTS] = {0};

    contacts[0].id = 1234;
    contacts[0].used = 1;

    CU_ASSERT_TRUE(idExists(contacts, 1234));
    CU_ASSERT_FALSE(idExists(contacts, 5678));
}

void test_generateId(void)
{
    Phonebook contacts[MAX_CONTACTS] = {0};

    contacts[0].id = 1234;
    contacts[0].used = 1;

    contacts[1].id = 5678;
    contacts[1].used = 1;

    int id = generateId(contacts);

    CU_ASSERT_TRUE(id >= 1000);
    CU_ASSERT_TRUE(id <= 9999);

    CU_ASSERT_FALSE(idExists(contacts, id));
}

void test_searchBySurname(void)
{
    Phonebook contacts[MAX_CONTACTS] = {0};
    Phonebook result[MAX_CONTACTS] = {0};

    contacts[0].id = 1001;
    strcpy(contacts[0].name, "Иван");
    strcpy(contacts[0].surname, "Иванов");
    strcpy(contacts[0].phone, "89000000001");
    contacts[0].used = 1;

    contacts[1].id = 1002;
    strcpy(contacts[1].name, "Петр");
    strcpy(contacts[1].surname, "Петров");
    strcpy(contacts[1].phone, "89000000002");
    contacts[1].used = 1;

    int found = searchContacts(
        contacts,
        result,
        1,
        SEARCH_SURNAME, "Иванов"
    );

    CU_ASSERT_EQUAL(found, 1);
    CU_ASSERT_EQUAL(result[0].id, 1001);
    CU_ASSERT_STRING_EQUAL(result[0].name, "Иван");
    CU_ASSERT_STRING_EQUAL(result[0].surname, "Иванов");
}

void test_searchBySurnameAndPhone(void)
{
    Phonebook contacts[MAX_CONTACTS] = {0};
    Phonebook result[MAX_CONTACTS] = {0};

    contacts[0].id = 1001;
    strcpy(contacts[0].name, "Иван");
    strcpy(contacts[0].surname, "Иванов");
    strcpy(contacts[0].phone, "89000000001");
    contacts[0].used = 1;

    contacts[1].id = 1002;
    strcpy(contacts[1].name, "Петр");
    strcpy(contacts[1].surname, "Иванов");
    strcpy(contacts[1].phone, "89000000002");
    contacts[1].used = 1;

    int found = searchContacts(
        contacts,
        result,
        2,
        SEARCH_SURNAME, "Иванов",
        SEARCH_PHONE, "89000000002"
    );

    CU_ASSERT_EQUAL(found, 1);
    CU_ASSERT_EQUAL(result[0].id, 1002);
    CU_ASSERT_STRING_EQUAL(result[0].name, "Петр");
}

void test_searchNotFound(void)
{
    Phonebook contacts[MAX_CONTACTS] = {0};
    Phonebook result[MAX_CONTACTS] = {0};

    contacts[0].id = 1001;
    strcpy(contacts[0].name, "Иван");
    strcpy(contacts[0].surname, "Иванов");
    strcpy(contacts[0].phone, "89000000001");
    contacts[0].used = 1;

    int found = searchContacts(
        contacts,
        result,
        1,
        SEARCH_NAME, "Данияр"
    );

    CU_ASSERT_EQUAL(found, 0);
}