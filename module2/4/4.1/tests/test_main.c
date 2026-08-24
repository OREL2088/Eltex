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

    if (CU_add_test(suite, "Initialize empty phonebook", testPhonebookInit) == NULL ||
        CU_add_test(suite, "Unique contact ID", testIdExists) == NULL ||
        CU_add_test(suite, "Sorted insertion and links", testSortedInsertion) == NULL ||
        CU_add_test(suite, "Update and reorder node", testUpdateReordersList) == NULL ||
        CU_add_test(suite, "Remove node and restore links", testRemoveContact) == NULL ||
        CU_add_test(suite, "Search by surname and phone", testSearchByTwoFields) == NULL ||
        CU_add_test(suite, "Search result is empty", testSearchNotFound) == NULL) {
        CU_cleanup_registry();
        return (int)CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    failures = CU_get_number_of_failures();
    CU_cleanup_registry();

    return failures == 0 ? 0 : 1;
}
