#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stddef.h>

typedef struct CalculatorOperation CalculatorOperation;

typedef struct
{
    CalculatorOperation *operations;
    size_t count;
} Calculator;

typedef enum
{
    CALCULATOR_OK = 0,
    CALCULATOR_INVALID_ARGUMENT,
    CALCULATOR_DIRECTORY_ERROR,
    CALCULATOR_NO_MEMORY,
    CALCULATOR_NO_OPERATIONS,
    CALCULATOR_INDEX_OUT_OF_RANGE,
    CALCULATOR_INVALID_OPERAND
} CalculatorStatus;

CalculatorStatus calculatorInit(Calculator *calculator,
                                const char *plugin_directory);
CalculatorStatus calculatorClear(Calculator *calculator);
CalculatorStatus calculatorGetOperationName(const Calculator *calculator,
                                            size_t index, const char **name);
CalculatorStatus calculatorExecute(const Calculator *calculator, size_t index,
                                   double first, double second,
                                   double *result);

#endif
