#include "test_permissions.h"

#include <CUnit/Basic.h>
#include <stdlib.h>

int main(void)
{
    CU_pSuite suite;
    unsigned int failures;

    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    suite = CU_add_suite("Permission tests", NULL, NULL);
    if (suite == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (CU_add_test(suite, "Parse octal permissions",
                    test_parse_octal_permissions) == NULL ||
        CU_add_test(suite, "Parse letter permissions",
                    test_parse_letter_permissions) == NULL ||
        CU_add_test(suite, "Format permissions",
                    test_format_permissions) == NULL ||
        CU_add_test(suite, "Apply add and remove",
                    test_apply_add_and_remove) == NULL ||
        CU_add_test(suite, "Apply assignment and sequence",
                    test_apply_assignment_and_sequence) == NULL ||
        CU_add_test(suite, "Reject command atomically",
                    test_reject_command_atomically) == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    failures = CU_get_number_of_failures();
    CU_cleanup_registry();

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
