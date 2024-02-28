#include "stack_operation.h"

#include <stdlib.h>

static inline StackOperation* StackOperationAllocate(void) {
  return (StackOperation*)malloc(sizeof(StackOperation));
}

static inline void StackOperationDeallocate(StackOperation* stack) {
  free(stack);
}

StackOperation* StackOperationNew(void) {
  StackOperation* new_stack = StackOperationAllocate();
  if (!new_stack) {
    return NULL;
  }
  *new_stack = (StackOperation){0};
  return new_stack;
}

StackOperation* StackOperationPush(StackOperation* stack, MathOperation val) {
  StackOperation* new_stack = StackOperationAllocate();
  if (!new_stack) {
    return NULL;
  }
  *new_stack = (StackOperation) {
      .prev = stack,
      .top = val,
      .size = stack->size + 1
  };
  return new_stack;
}

MathOperation StackOperationPop(StackOperation** stack) {
  if ((*stack)->size == 0) {
    *stack = NULL;
    return (MathOperation){0};
  }
  StackOperation* tmp = *stack;
  MathOperation top_val = tmp->top;
  *stack = (*stack)->prev;
  StackOperationDeallocate(tmp);
  return top_val;
}

void StackOperationDelete(StackOperation* stack) {
  while(stack->size != 0) {
    StackOperationPop(&stack);
  }
  StackOperationDeallocate(stack);
}