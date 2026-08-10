#include "test_phonebook.h"

int init_suite(void)
{
    return 0;
}

int clean_suite(void)
{
    return 0;
}

int main(void)
{
    CU_pSuite suite = NULL;

    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    suite = CU_add_suite(
        "Phonebook tests",
        init_suite,
        clean_suite
    );

    if (suite == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_add_test(
        suite,
        "ID exists",
        test_idExists
    );

    CU_add_test(
        suite,
        "Generate unique ID",
        test_generateId
    );

    CU_add_test(
        suite,
        "Search by surname",
        test_searchBySurname
    );

    CU_add_test(
        suite,
        "Search by surname and phone",
        test_searchBySurnameAndPhone
    );

    CU_add_test(
        suite,
        "Search not found",
        test_searchNotFound
    );

    CU_basic_set_mode(CU_BRM_VERBOSE);

    CU_basic_run_tests();

    CU_cleanup_registry();

    return CU_get_error();
}