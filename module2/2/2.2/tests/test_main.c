#include "test_calculator.h"

#include <stdlib.h>

int main(void)
{
    if (CU_initialize_registry() != CUE_SUCCESS)
    {
        return CU_get_error();
    }

    CU_pSuite suite = CU_add_suite("Calculator tests", NULL, NULL);

    if (suite == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (CU_add_test(suite, "Addition", test_add) == NULL ||
        CU_add_test(suite, "Subtraction", test_subtract) == NULL ||
        CU_add_test(suite, "Multiplication", test_multiply) == NULL ||
        CU_add_test(suite, "Division", test_divide) == NULL ||
        CU_add_test(suite, "Function pointers", test_operation_pointer) == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    unsigned int failures = CU_get_number_of_failures();
    CU_cleanup_registry();

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
