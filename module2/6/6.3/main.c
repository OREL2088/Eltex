#include "calculator.h"
#include "ui.h"

#include <stdlib.h>

int main(int argc, char *argv[])
{
    const char *directory = argc > 1 ? argv[1] : "./plugins";
    Calculator calculator;
    CalculatorStatus status;

    status = calculatorInit(&calculator, directory);
    uiReportLoading(status, directory, calculator.count);
    if (status != CALCULATOR_OK)
    {
        return EXIT_FAILURE;
    }

    status = uiRun(&calculator);
    if (calculatorClear(&calculator) != CALCULATOR_OK)
    {
        return EXIT_FAILURE;
    }

    return status == CALCULATOR_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
