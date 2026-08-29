#include "../plugin.h"

static double calculate(double first, double second)
{
    return first + second;
}

const CalculatorPlugin calculator_plugin = {"Сложение", calculate, 1U, 0U};
