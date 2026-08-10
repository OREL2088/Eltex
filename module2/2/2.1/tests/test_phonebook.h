#ifndef TEST_PHONEBOOK_H
#define TEST_PHONEBOOK_H

#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include "../phonebook.h"

void test_idExists(void);
void test_generateId(void);

void test_searchBySurname(void);
void test_searchBySurnameAndPhone(void);
void test_searchNotFound(void);

#endif