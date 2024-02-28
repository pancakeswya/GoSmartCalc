#ifndef SMARTCALC_INTERNAL_UTIL_CC_CORE_STACK_DOUBLE_H_
#define SMARTCALC_INTERNAL_UTIL_CC_CORE_STACK_DOUBLE_H_

#include <stddef.h>

typedef struct StackDouble {
  struct StackDouble* prev;

  size_t size;
  double top;
} StackDouble;

extern StackDouble* StackDoubleNew(void);
extern StackDouble* StackDoublePush(StackDouble* stack, double val);
extern double StackDoublePop(StackDouble** stack);
extern void StackDoubleDelete(StackDouble* stack);

#endif // SMARTCALC_INTERNAL_UTIL_CC_CORE_STACK_DOUBLE_H_

