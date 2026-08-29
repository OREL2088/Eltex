#include "test_phonebook.h"

int main(void)
{
    CU_pSuite suite;
    unsigned int failures;

    if (CU_initialize_registry() != CUE_SUCCESS)
        return (int)CU_get_error();
    suite = CU_add_suite("Phonebook binary tree tests", NULL, NULL);
    if (suite == NULL) {
        CU_cleanup_registry();
        return (int)CU_get_error();
    }
    if (CU_add_test(suite, "Initialization and status", testInitAndStatuses) == NULL ||
        CU_add_test(suite, "Insert, find and duplicate", testInsertFindAndDuplicate) == NULL ||
        CU_add_test(suite, "Periodic automatic balance", testAutomaticBalance) == NULL ||
        CU_add_test(suite, "Update and reorder", testUpdateReordersTree) == NULL ||
        CU_add_test(suite, "Remove contact", testRemoveContact) == NULL ||
        CU_add_test(suite, "Search by two fields", testSearchByTwoFields) == NULL ||
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
