#include "test_calculator.h"

#include <stdlib.h>

int main(void)
{
    CU_pSuite suite;
    unsigned int failures;

    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    suite = CU_add_suite("Dynamic calculator tests", NULL, NULL);
    if (suite == NULL ||
        CU_add_test(suite, "Initialization", testInitialization) == NULL ||
        CU_add_test(suite, "Operations", testOperations) == NULL ||
        CU_add_test(suite, "Error statuses", testErrorStatuses) == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    failures = CU_get_number_of_failures();
    CU_cleanup_registry();
    return failures == 0U ? EXIT_SUCCESS : EXIT_FAILURE;
}
