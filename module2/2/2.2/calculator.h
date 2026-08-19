#ifndef CALCULATOR_H
#define CALCULATOR_H

typedef double (*Operation)(double, double);

typedef struct
{
    const char *name;
    Operation function;
} Command;

double add(double a, double b);
double subtract(double a, double b);
double multiply(double a, double b);
double divide(double a, double b);

#endif
