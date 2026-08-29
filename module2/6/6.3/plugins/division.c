#include "../plugin.h"

static double calculate(double first, double second)
{
    return first / second;
}

const CalculatorPlugin calculator_plugin = {
    "Деление", calculate, 4U, OPERATION_SECOND_OPERAND_NONZERO
};
