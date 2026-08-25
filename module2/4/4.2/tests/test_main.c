#include "test_priority_queue.h"

int main(void)
{
    CU_pSuite suite;
    unsigned int failures;

    if (CU_initialize_registry() != CUE_SUCCESS)
        return (int)CU_get_error();

    suite = CU_add_suite("Priority queue tests", NULL, NULL);
    if (suite == NULL) {
        CU_cleanup_registry();
        return (int)CU_get_error();
    }

    if (CU_add_test(suite, "Initialize, clear and validate queue",
                    testQueueInitClearAndValidation) == NULL ||
        CU_add_test(suite, "Pop first message", testQueuePopFront) == NULL ||
        CU_add_test(suite, "Pop message with exact priority",
                    testQueuePopExactPriority) == NULL ||
        CU_add_test(suite, "Pop message with priority not lower than specified",
                    testQueuePopNotLower) == NULL ||
        CU_add_test(suite, "Handle empty queue and invalid arguments",
                    testQueueEmptyAndInvalidArguments) == NULL ||
        CU_add_test(suite, "Traverse messages in FIFO order",
                    testQueueForEach) == NULL) {
        CU_cleanup_registry();
        return (int)CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    failures = CU_get_number_of_failures();
    CU_cleanup_registry();

    return failures == 0 ? 0 : 1;
}
