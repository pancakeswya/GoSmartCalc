#ifndef SMARTCALC_INTERNAL_CALC_CC_CORE_BASIC_CALC_H_
#define SMARTCALC_INTERNAL_CALC_CC_CORE_BASIC_CALC_H_

#include "api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  kBasicCalcErrorSuccess = 0,
  kBasicCalcAllocationFail,
  kBasicCalcErrorInvalidSyntax,
  kBasicCalcErrorBracesNotMatching,
  kBasicCalcErrorIncorrectNumberUsage,
  kBasicCalcErrorIncorrectOperatorUsage,
  kBasicCalcErrorIncorrectFunctionUsage,
  kBasicCalcErrorInvalidXExpr,
  kBasicCalcErrorInvalidExpr
} BasicCalcError;

extern CALC_API BasicCalcError BasicCalculateExpr(const char* math_expr, double* res);
extern CALC_API BasicCalcError BasicCalculateEquation(const char* math_expr, const char* x, double* res);

#ifdef __cplusplus
} // extern "C"
#endif

#endif  // SMARTCALC_INTERNAL_CALC_CC_CORE_BASIC_CALC_H_