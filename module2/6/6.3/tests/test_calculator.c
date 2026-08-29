#include "test_calculator.h"

#include <string.h>

void testInitialization(void)
{
    Calculator calculator;
    const char *name = NULL;

    CU_ASSERT_EQUAL(calculatorInit(&calculator, "./plugins"), CALCULATOR_OK);
    CU_ASSERT_EQUAL(calculator.count, 4U);
    CU_ASSERT_EQUAL(calculatorGetOperationName(&calculator, 0U, &name),
                    CALCULATOR_OK);
    CU_ASSERT_PTR_NOT_NULL(name);
    if (name != NULL)
        CU_ASSERT_STRING_EQUAL(name, "Сложение");
    CU_ASSERT_EQUAL(calculatorClear(&calculator), CALCULATOR_OK);
    CU_ASSERT_PTR_NULL(calculator.operations);
    CU_ASSERT_EQUAL(calculator.count, 0U);
}

void testOperations(void)
{
    Calculator calculator;
    double result = 0.0;

    CU_ASSERT_EQUAL(calculatorInit(&calculator, "./plugins"), CALCULATOR_OK);

    CU_ASSERT_EQUAL(calculatorExecute(&calculator, 0U, 2.0, 3.0, &result),
                    CALCULATOR_OK);
    CU_ASSERT_DOUBLE_EQUAL(result, 5.0, 1e-9);

    CU_ASSERT_EQUAL(calculatorExecute(&calculator, 1U, 8.0, 2.0, &result),
                    CALCULATOR_OK);
    CU_ASSERT_DOUBLE_EQUAL(result, 6.0, 1e-9);

    CU_ASSERT_EQUAL(calculatorExecute(&calculator, 2U, 4.0, 3.0, &result),
                    CALCULATOR_OK);
    CU_ASSERT_DOUBLE_EQUAL(result, 12.0, 1e-9);

    CU_ASSERT_EQUAL(calculatorExecute(&calculator, 3U, 8.0, 2.0, &result),
                    CALCULATOR_OK);
    CU_ASSERT_DOUBLE_EQUAL(result, 4.0, 1e-9);

    CU_ASSERT_EQUAL(calculatorClear(&calculator), CALCULATOR_OK);
}

void testErrorStatuses(void)
{
    Calculator calculator;
    double result = 0.0;

    CU_ASSERT_EQUAL(calculatorInit(NULL, "./plugins"),
                    CALCULATOR_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(calculatorInit(&calculator, "./directory-does-not-exist"),
                    CALCULATOR_DIRECTORY_ERROR);

    CU_ASSERT_EQUAL(calculatorInit(&calculator, "./plugins"), CALCULATOR_OK);
    CU_ASSERT_EQUAL(calculatorExecute(&calculator, 3U, 8.0, 0.0, &result),
                    CALCULATOR_INVALID_OPERAND);
    CU_ASSERT_EQUAL(calculatorExecute(&calculator, calculator.count,
                                      1.0, 1.0, &result),
                    CALCULATOR_INDEX_OUT_OF_RANGE);
    CU_ASSERT_EQUAL(calculatorExecute(&calculator, 0U, 1.0, 1.0, NULL),
                    CALCULATOR_INVALID_ARGUMENT);
    CU_ASSERT_EQUAL(calculatorClear(&calculator), CALCULATOR_OK);
}
