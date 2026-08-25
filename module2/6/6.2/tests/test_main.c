#include "test_phonebook.h"

int main(void)
{
    CU_pSuite suite;
    unsigned int failures;

    if (CU_initialize_registry() != CUE_SUCCESS)
        return (int)CU_get_error();
    suite = CU_add_suite("Phonebook doubly linked list tests", NULL, NULL);
    if (suite == NULL) {
        CU_cleanup_registry();
        return (int)CU_get_error();
    }
    if (CU_add_test(suite, "Initialization and clear statuses",
                    testInitAndClearStatuses) == NULL ||
        CU_add_test(suite, "Sorted insertion and duplicate status",
                    testSortedInsertionAndDuplicate) == NULL ||
        CU_add_test(suite, "Find, update and reorder", testFindAndUpdate) == NULL ||
        CU_add_test(suite, "Remove statuses and links",
                    testRemoveStatusesAndLinks) == NULL ||
        CU_add_test(suite, "Search by two fields", testSearchByTwoFields) == NULL ||
        CU_add_test(suite, "Search capacity status",
                    testSearchCapacityStatus) == NULL ||
        CU_add_test(suite, "Invalid arguments", testInvalidArguments) == NULL ||
        CU_add_test(suite, "Storage round trip", testStorageRoundTrip) == NULL) {
        CU_cleanup_registry();
        return (int)CU_get_error();
    }
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    failures = CU_get_number_of_failures();
    CU_cleanup_registry();
    return failures == 0 ? 0 : 1;
}
