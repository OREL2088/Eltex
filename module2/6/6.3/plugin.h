#ifndef PLUGIN_H
#define PLUGIN_H

typedef double (*Operation)(double, double);

enum { OPERATION_SECOND_OPERAND_NONZERO = 1U };

typedef struct
{
    const char *name;
    Operation function;
    unsigned int order;
    unsigned int flags;
} CalculatorPlugin;

extern const CalculatorPlugin calculator_plugin;

#endif
