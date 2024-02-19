#ifndef SMARTCALC_CC_CORE_BASIC_CALC_H_
#define SMARTCALC_CC_CORE_BASIC_CALC_H_

#ifndef __GO
extern "C" {
#endif

enum BasicCalcError {
  kBasicCalcErrorSuccess,
  kBasicCalcErrorInvalidSyntax,
  kBasicCalcErrorBracesNotMatching,
  kBasicCalcErrorIncorrectNumberUsage,
  kBasicCalcErrorIncorrectOperatorUsage,
  kBasicCalcErrorIncorrectFunctionUsage,
  kBasicCalcErrorInvalidXExpr,
  kBasicCalcErrorInvalidExpr,
  kBasicCalcErrorsLen
};

extern double BasicCalculateExpr(const char* math_expr, int* error);
extern double BasicCalculateEquation(const char* math_expr, double x, int* error);

#ifndef __GO
} // extern "C"
#endif

#endif  // SMARTCALC_CC_CORE_BASIC_CALC_H_