#ifndef SMARTCALC_INTERNAL_UTIL_CC_CORE_STACK_H_
#define SMARTCALC_INTERNAL_UTIL_CC_CORE_STACK_H_

#include "stack_double.h"
#include "stack_operation.h"

#define StackNew(type) _Generic(((type){0}), double: StackDoubleNew, MathOperation: StackOperationNew)()
#define StackPush(st, val) _Generic((st), StackDouble*: StackDoublePush, StackOperation*: StackOperationPush)(st,val)
#define StackPop(st) _Generic((st), StackDouble**: StackDoublePop, StackOperation**: StackOperationPop)(st)
#define StackDelete(st) _Generic((st), StackDouble*: StackDoubleDelete, StackOperation*: StackOperationDelete)(st)

#endif // SMARTCALC_INTERNAL_UTIL_CC_CORE_STACK_H_