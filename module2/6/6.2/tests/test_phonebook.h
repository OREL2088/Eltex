#ifndef TEST_PHONEBOOK_H
#define TEST_PHONEBOOK_H

#include <CUnit/Basic.h>

#include "../phonebook.h"

void testInitAndClearStatuses(void);
void testSortedInsertionAndDuplicate(void);
void testFindAndUpdate(void);
void testRemoveStatusesAndLinks(void);
void testSearchByTwoFields(void);
void testSearchCapacityStatus(void);
void testInvalidArguments(void);
void testStorageRoundTrip(void);

#endif
