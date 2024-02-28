#ifndef SMARTCALC_INTERNAL_UTIL_CC_CORE_MATH_OPERATION_H_
#define SMARTCALC_INTERNAL_UTIL_CC_CORE_MATH_OPERATION_H_

#include <stdbool.h>

typedef struct {
  int priority;
  int type;
  union {
    double (*unary)(double);
    double (*binary)(double, double);
  } function;
} MathOperation;

enum MathOperationType { kUnary, kBinary };

enum MathOperationPriority {
  kBrace,
  kSimple,
  kComplex,
  kFunction,
  kSign
};

#endif // SMARTCALC_INTERNAL_UTIL_CC_CORE_MATH_OPERATION_H_