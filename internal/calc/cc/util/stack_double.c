#include "stack_double.h"

#include <stdlib.h>

static inline StackDouble* StackDoubleAllocate(void) {
  return (StackDouble*)malloc(sizeof(StackDouble));
}

static inline void StackDoubleDeallocate(StackDouble* stack) {
  free(stack);
}

StackDouble* StackDoubleNew(void) {
  StackDouble* new_stack = StackDoubleAllocate();
  if (!new_stack) {
    return NULL;
  }
  *new_stack = (StackDouble){0};
  return new_stack;
}

StackDouble* StackDoublePush(StackDouble* stack, double val) {
  StackDouble* new_stack = StackDoubleAllocate();
  if (!new_stack) {
    return NULL;
  }
  *new_stack = (StackDouble) {
      .prev = stack,
      .top = val,
      .size = stack->size + 1
  };
  return new_stack;
}

double StackDoublePop(StackDouble** stack) {
  if ((*stack)->size == 0) {
    *stack = NULL;
    return 0;
  }
  StackDouble* tmp = *stack;
  double top_val = tmp->top;
  *stack = (*stack)->prev;
  StackDoubleDeallocate(tmp);
  return top_val;
}

void StackDoubleDelete(StackDouble* stack) {
  while(stack->size != 0) {
    StackDoublePop(&stack);
  }
  StackDoubleDeallocate(stack);
}