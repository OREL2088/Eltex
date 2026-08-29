#include "../plugin.h"

static double calculate(double first, double second)
{
    return first * second;
}

const CalculatorPlugin calculator_plugin = {"Умножение", calculate, 3U, 0U};
