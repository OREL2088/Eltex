#include "../plugin.h"

static double calculate(double first, double second)
{
    return first - second;
}

const CalculatorPlugin calculator_plugin = {"Вычитание", calculate, 2U, 0U};
