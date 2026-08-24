#ifndef TEST_PHONEBOOK_H
#define TEST_PHONEBOOK_H

#include <CUnit/Basic.h>
#include <stdio.h>

#include "../phonebook.h"

void testPhonebookInit(void);
void testIdExists(void);
void testSortedInsertion(void);
void testUpdateReordersList(void);
void testRemoveContact(void);
void testSearchByTwoFields(void);
void testSearchNotFound(void);

#endif
