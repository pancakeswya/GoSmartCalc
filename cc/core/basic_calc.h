#ifndef SMARTCALC_CC_CORE_BASIC_CALC_H_
#define SMARTCALC_CC_CORE_BASIC_CALC_H_

#include "calc_api.h"

#ifndef __GO
extern "C" {
#endif

typedef enum {
  kBasicCalcErrorSuccess,
  kBasicCalcErrorInvalidSyntax,
  kBasicCalcErrorBracesNotMatching,
  kBasicCalcErrorIncorrectNumberUsage,
  kBasicCalcErrorIncorrectOperatorUsage,
  kBasicCalcErrorIncorrectFunctionUsage,
  kBasicCalcErrorInvalidXExpr,
  kBasicCalcErrorInvalidExpr,
  kBasicCalcErrorsLen
} BasicCalcError;

extern CALC_API double BasicCalculateExpr(const char* math_expr, int* error);
extern CALC_API double BasicCalculateEquation(const char* math_expr, double x, int* error);

#ifndef __GO
} // extern "C"
#endif

#endif  // SMARTCALC_CC_CORE_BASIC_CALC_H_