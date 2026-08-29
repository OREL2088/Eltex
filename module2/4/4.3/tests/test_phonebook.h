#ifndef TEST_PHONEBOOK_H
#define TEST_PHONEBOOK_H

#include <CUnit/Basic.h>

#include "../phonebook.h"

void testInitAndStatuses(void);
void testInsertFindAndDuplicate(void);
void testAutomaticBalance(void);
void testUpdateReordersTree(void);
void testRemoveContact(void);
void testSearchByTwoFields(void);
void testInvalidArguments(void);
void testStorageRoundTrip(void);

#endif
