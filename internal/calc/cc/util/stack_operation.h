#ifndef SMARTCALC_INTERNAL_UTIL_CC_CORE_STACK_OPERATION_H_
#define SMARTCALC_INTERNAL_UTIL_CC_CORE_STACK_OPERATION_H_

#include "math_operation.h"

#include <stddef.h>

typedef struct StackOperation {
  struct StackOperation* prev;
  MathOperation top;
  size_t size;
} StackOperation;

extern StackOperation* StackOperationNew(void);
extern StackOperation* StackOperationPush(StackOperation* stack, MathOperation val);
extern MathOperation StackOperationPop(StackOperation** stack);
extern void StackOperationDelete(StackOperation* stack);

#endif // SMARTCALC_INTERNAL_UTIL_CC_CORE_STACK_OPERATION_H_