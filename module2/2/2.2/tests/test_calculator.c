#include "test_calculator.h"

static const double EPSILON = 1e-9;

void test_add(void)
{
    CU_ASSERT_DOUBLE_EQUAL(add(2.0, 3.0), 5.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(add(-2.0, 3.0), 1.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(add(0.0, 0.0), 0.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(add(-2.5, -3.5), -6.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(add(0.1, 0.2), 0.3, EPSILON);
}

void test_subtract(void)
{
    CU_ASSERT_DOUBLE_EQUAL(subtract(5.0, 3.0), 2.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(subtract(3.0, 5.0), -2.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(subtract(0.0, 7.0), -7.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(subtract(-4.0, -6.0), 2.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(subtract(5.5, 2.25), 3.25, EPSILON);
}

void test_multiply(void)
{
    CU_ASSERT_DOUBLE_EQUAL(multiply(4.0, 3.0), 12.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(multiply(-4.0, 3.0), -12.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(multiply(-4.0, -3.0), 12.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(multiply(0.0, 1000.0), 0.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(multiply(1.5, 2.5), 3.75, EPSILON);
}

void test_divide(void)
{
    CU_ASSERT_DOUBLE_EQUAL(divide(8.0, 2.0), 4.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(divide(7.0, 2.0), 3.5, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(divide(-9.0, 3.0), -3.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(divide(-9.0, -3.0), 3.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(divide(0.0, 5.0), 0.0, EPSILON);
    CU_ASSERT_DOUBLE_EQUAL(divide(1.0, 4.0), 0.25, EPSILON);
}

void test_operation_pointer(void)
{
    Operation operation = add;
    CU_ASSERT_DOUBLE_EQUAL(operation(10.0, 5.0), 15.0, EPSILON);

    operation = multiply;
    CU_ASSERT_DOUBLE_EQUAL(operation(10.0, 5.0), 50.0, EPSILON);
}
