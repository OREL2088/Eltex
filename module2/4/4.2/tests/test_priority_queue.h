#ifndef TEST_PRIORITY_QUEUE_H
#define TEST_PRIORITY_QUEUE_H

#include <CUnit/Basic.h>

#include "../priority_queue.h"

void testQueueInitClearAndValidation(void);
void testQueuePopFront(void);
void testQueuePopExactPriority(void);
void testQueuePopNotLower(void);
void testQueueEmptyAndInvalidArguments(void);
void testQueueForEach(void);

#endif
