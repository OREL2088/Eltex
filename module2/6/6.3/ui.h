#ifndef UI_H
#define UI_H

#include "calculator.h"

#include <stddef.h>

const char *uiCalculatorStatusMessage(CalculatorStatus status);
void uiReportLoading(CalculatorStatus status, const char *directory,
                     size_t loaded_count);
CalculatorStatus uiRun(const Calculator *calculator);

#endif
