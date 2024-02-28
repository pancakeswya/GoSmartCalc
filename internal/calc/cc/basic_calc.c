#include "basic_calc.h"
#include "util/str_util.h"
#include "util/stack_double.h"
#include "util/stack_operation.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

static double UnaryMinusFunction(double num) {
  return -num;
}

static double UnaryPlusFunction(double num) {
  return num;
}

static double BinaryMinusFunction(double num1, double num2) {
  return num1 - num2;
}

static double BinaryPlusFunction(double num1, double num2) {
  return num1 + num2;
}

static double MultiplyFunction(double num1, double num2) {
  return num1 * num2;
}

static double DivisionFunction(double num1, double num2) {
  if (num2 == 0) {
    return DBL_MAX;
  }
  return num1 / num2;
}

static double FmodFunction(double num1, double num2) {
  if (num2 == 0) {
    return DBL_MIN;
  }
  return fmod(num1, num2);
}

enum MathOperationIdx {
  kUnaryMinus = 0,
  kUnaryPlus,
  kSqrt,
  kSin,
  kCos,
  kTan,
  kAsin,
  kAcos,
  kAtan,
  kLn,
  kLog,
  kPower,
  kMultiply,
  kDivision,
  kFmod,
  kPlus,
  kMinus,
  kOpenBrace
};

static const MathOperation op_map[] = {
    {.type = kUnary,
     .priority = kSign,
     .function = { .unary = UnaryMinusFunction }},
    {.type = kUnary,
     .priority = kSign,
     .function = { .unary = UnaryPlusFunction }},
    {.type = kUnary,
     .priority = kFunction,
     .function = { .unary = sqrt }},
    {.type = kUnary,
     .priority = kFunction,
     .function = { .unary = sin }},
    {.type = kUnary,
     .priority = kFunction,
     .function = { .unary = cos }},
    {.type = kUnary,
     .priority = kFunction,
     .function = { .unary = tan }},
    {.type = kUnary,
     .priority = kFunction,
     .function = { .unary = asin }},
    {.type = kUnary,
     .priority = kFunction,
     .function = { .unary = acos }},
    {.type = kUnary,
     .priority = kFunction,
     .function = { .unary = atan }},
    {.type = kUnary,
     .priority = kFunction,
     .function = { .unary = log }},
    {.type = kUnary,
     .priority = kFunction,
     .function = { .unary = log10 }},
    {.type = kBinary,
     .priority = kFunction,
     .function = { .binary = pow }},
    {.type = kBinary,
     .priority = kComplex,
     .function = { .binary = MultiplyFunction }},
    {.type = kBinary,
     .priority = kComplex,
     .function = { .binary = DivisionFunction }},
    {.type = kBinary,
     .priority = kComplex,
     .function = { .binary = FmodFunction }},
    {.type = kBinary,
     .priority = kSimple,
     .function = { .binary = BinaryPlusFunction }},
    {.type = kBinary,
     .priority = kSimple,
     .function = { .binary = BinaryMinusFunction }},
    {.type = kUnary,
     .priority = kBrace}
};

BasicCalcError FindOperation(char op, bool prev_was_num, MathOperation* operation) {
  switch (op) {
    case '^':
      if (!prev_was_num) {
        return kBasicCalcErrorIncorrectOperatorUsage;
      }
      *operation = op_map[kPower];
      break;
    case '*':
      if (!prev_was_num) {
        return kBasicCalcErrorIncorrectOperatorUsage;
      }
      *operation = op_map[kMultiply];
      break;
    case '/':
      if (!prev_was_num) {
        return kBasicCalcErrorIncorrectOperatorUsage;
      }
      *operation = op_map[kDivision];
      break;
    case 'd':
      if (!prev_was_num) {
        return kBasicCalcErrorIncorrectOperatorUsage;
      }
      *operation = op_map[kFmod];
      break;
    case '+':
      if (prev_was_num) {
        *operation = op_map[kPlus];
      } else {
        *operation = op_map[kUnaryPlus];
      }
      break;
    case '-':
      if (prev_was_num) {
        *operation = op_map[kMinus];
      } else {
        *operation = op_map[kUnaryMinus];
      }
      break;
    default:
      return kBasicCalcErrorIncorrectOperatorUsage;
  }
  return kBasicCalcErrorSuccess;
}

static BasicCalcError FindFunction(char** ptr_ptr, MathOperation* operation) {
  int is_arc = 0;
  char *ptr = *ptr_ptr;
  if (*ptr == 'a') {
    is_arc = 1;
    ++ptr;
  }
  switch (*ptr) {
    case 'c':
      if (ptr[1] != 'o' || ptr[2] != 's' || ptr[3] != '(') {
        return kBasicCalcErrorIncorrectFunctionUsage;
      }
      *ptr_ptr = ptr + 4;
      if (is_arc) {
        *operation = op_map[kAcos];
      } else {
        *operation = op_map[kCos];
      }
      break;
    case 's':
      if (ptr[1] == 'q') {
        if (ptr[2] != 'r' || ptr[3] != 't' || ptr[4] != '(') {
          return kBasicCalcErrorIncorrectFunctionUsage;
        }
        *ptr_ptr = ptr + 5;
        *operation = op_map[kSqrt];
      } else if (ptr[1] == 'i') {
        if (ptr[2] != 'n' || ptr[3] != '(') {
          return kBasicCalcErrorIncorrectFunctionUsage;
        }
        *ptr_ptr = ptr + 4;
        if (is_arc) {
          *operation = op_map[kAsin];
        } else {
          *operation = op_map[kSin];
        }
      } else {
        return kBasicCalcErrorIncorrectFunctionUsage;
      }
      break;
    case 't':
      if (ptr[1] != 'a' || ptr[2] != 'n' || ptr[3] != '(') {
        return kBasicCalcErrorIncorrectFunctionUsage;
      }
      *ptr_ptr = ptr + 4;
      if (is_arc) {
        *operation = op_map[kAtan];
      } else {
        *operation = op_map[kTan];
      }
      break;
    case 'l':
      if (is_arc) {
        return kBasicCalcErrorIncorrectFunctionUsage;
      }
      if (ptr[1] == 'n') {
        if (ptr[2] != '(') {
          return kBasicCalcErrorIncorrectFunctionUsage;
        }
        *ptr_ptr = ptr + 3;
        *operation = op_map[kLn];
      } else if (ptr[1] == 'o') {
        if (ptr[2] != 'g' || ptr[3] != '(') {
          return kBasicCalcErrorIncorrectFunctionUsage;
        }
        *ptr_ptr = ptr + 4;
        *operation = op_map[kLog];
      } else {
        return kBasicCalcErrorIncorrectFunctionUsage;
      }
      break;
    default:
      return kBasicCalcErrorIncorrectFunctionUsage;
  }
  return kBasicCalcErrorSuccess;
}

static BasicCalcError ShuntYardAlgo(StackDouble** num_stack, StackOperation** op_stack) {
  MathOperation op = StackOperationPop(op_stack);
  if (!*op_stack) {
    return kBasicCalcErrorInvalidSyntax;
  }
  double num1 = StackDoublePop(num_stack);
  if (!*num_stack) {
    return kBasicCalcErrorInvalidSyntax;
  }
  if (op.priority == kBrace) {
    return kBasicCalcErrorBracesNotMatching;
  }
  double res;
  if (op.type == kUnary) {
    res = op.function.unary(num1);
  } else {
    double num2 = StackDoublePop(num_stack);
    if (!*num_stack) {
      return kBasicCalcErrorInvalidSyntax;
    }
    res = op.function.binary(num2, num1);
  }
  *num_stack = StackDoublePush(*num_stack, res);
  if (!*num_stack) {
    return kBasicCalcAllocationFail;
  }
  return kBasicCalcErrorSuccess;
}

static BasicCalcError ShuntYardBrace(StackDouble** num_stack, StackOperation** op_stack) {
  while ((*op_stack)->size && (*op_stack)->top.priority != kBrace) {
    BasicCalcError error = ShuntYardAlgo(num_stack, op_stack);
    if (error != kBasicCalcErrorSuccess) {
      return error;
    }
  }
  if ((*op_stack)->size == 0) {
    return kBasicCalcErrorBracesNotMatching;
  }
  StackOperationPop(op_stack);
  return kBasicCalcErrorSuccess;
}

static BasicCalcError ShuntYardOperation(const MathOperation* op, StackDouble** num_stack, StackOperation** op_stack) {
  while ((*op_stack)->size && op->priority <= (*op_stack)->top.priority) {
    BasicCalcError error = ShuntYardAlgo(num_stack, op_stack);
    if (error != kBasicCalcErrorSuccess) {
      return error;
    }
  }
  *op_stack = StackOperationPush(*op_stack, *op);
  if (!*op_stack) {
    return kBasicCalcAllocationFail;
  }
  return kBasicCalcErrorSuccess;
}

static char* ClosedBraceNext(char* ptr) {
  for (; *ptr && *ptr != ')'; ++ptr)
    ;
  for (; *ptr == ')'; ++ptr)
    ;
  return ptr;
}

static inline bool PowExprAcceptable(char ch) {
  return isalnum(ch) || isspace(ch) || ch == '^' || ch == '(' || ch == '.';
}

static BasicCalcError FixPower(char** ptr_ptr, char** expr) {
  char* ptr = *ptr_ptr;
  char* start = ptr + 1;
  bool has_pow = false;
  while (PowExprAcceptable(*ptr)) {
    if (*ptr == '^') {
      has_pow = true;
    } else if (*ptr == '(') {
      ptr = ClosedBraceNext(ptr);
      continue;
    }
    ++ptr;
  }
  if (has_pow) {
    size_t idx1 = start - *expr;
    size_t idx2 = ptr - *expr + 1;
    *expr = StrInsert(*expr, "(", idx1);
    if (!*expr) {
      return kBasicCalcAllocationFail;
    }
    *expr = StrInsert(*expr, ")", idx2);
    if (!*expr) {
      return kBasicCalcAllocationFail;
    }
    *ptr_ptr = *expr + idx1 - 1;
  } else {
    *ptr_ptr = start - 1;
  }
  return kBasicCalcErrorSuccess;
}

static inline BasicCalcError ProcessOperation(char** ptr_ptr,
                                              char** expr,
                                              bool prev_was_num,
                                              StackDouble** num_stack,
                                              StackOperation** op_stack) {
  if (**ptr_ptr == '^') {
    BasicCalcError error = FixPower(ptr_ptr, expr);
    if (error != kBasicCalcErrorSuccess) {
      return error;
    }
  }
  MathOperation op;
  BasicCalcError error = FindOperation(**ptr_ptr, prev_was_num, &op);
  if (error != kBasicCalcErrorSuccess) {
    return error;
  }
  return ShuntYardOperation(&op, num_stack, op_stack);
}

static inline BasicCalcError ProcessFunction(char** ptr_ptr, StackOperation** op_stack) {
  MathOperation function;
  BasicCalcError error = FindFunction(ptr_ptr, &function);
  if (error != kBasicCalcErrorSuccess) {
    return error;
  }
  *op_stack = StackOperationPush(*op_stack, function);
  if (!*op_stack) {
    return kBasicCalcAllocationFail;
  }
  return kBasicCalcErrorSuccess;
}

static inline BasicCalcError ProcessNumber(char** expr, StackDouble** num_stack) {
  char* ptr = *expr;
  char* end = NULL;
  double num = strtod(ptr, &end);
  if (ptr == end) {
    return kBasicCalcErrorIncorrectNumberUsage;
  }
  *num_stack = StackDoublePush(*num_stack, num);
  if (!*num_stack) {
    return kBasicCalcAllocationFail;
  }
  *expr = end;
  return kBasicCalcErrorSuccess;
}

static BasicCalcError ReplaceXInString(char** expr, const char* number) {
  char* ptr = *expr;
  char prev = '\0';
  for (;*ptr; ++ptr) {
    if (*ptr == 'x') {
      if (ptr != *expr && (isdigit(prev) || prev == 'x')) {
        return kBasicCalcErrorInvalidXExpr;
      }
      *ptr = ' ';
      size_t idx = ptr - *expr;
      *expr = StrInsert(*expr, number, idx);
      if (!*expr) {
        return kBasicCalcAllocationFail;
      }
      ptr = *expr + idx;
    }
    if (!isspace(*ptr)) {
      prev = *ptr;
    }
  }
  return kBasicCalcErrorSuccess;
}

BasicCalcError CALL_CONV BasicCalculateExpr(const char* math_expr, double* res) {
  BasicCalcError error = kBasicCalcErrorSuccess;
  char* expr = StrDup(math_expr);
  if (!expr) {
    return kBasicCalcAllocationFail;
  }
  char* ptr = expr;
  bool prev_was_num = false;

  StackDouble* num_stack = StackDoubleNew();
  if (!num_stack) {
    return kBasicCalcAllocationFail;
  }
  StackOperation* op_stack = StackOperationNew();
  if (!op_stack) {
    error = kBasicCalcAllocationFail;
    goto cleanup;
  }
  while(*ptr) {
    switch (*ptr) {
      case ' ':
      case '\n':
      case '\t':
        break;
      case 'm':
        if (ptr[1] != 'o' || ptr[2] != 'd') {
          error = kBasicCalcErrorIncorrectFunctionUsage;
          goto cleanup;
        }
        ptr += 2;
        /* fallthrough */
      case '^':
      case '+':
      case '-':
      case '*':
      case '/':
        error = ProcessOperation(&ptr, &expr,
                                 prev_was_num,
                                 &num_stack,
                                 &op_stack);
        if (error != kBasicCalcErrorSuccess) {
          goto cleanup;
        }
        prev_was_num = false;
        break;
      case '(':
        op_stack = StackOperationPush(op_stack, op_map[kOpenBrace]);
        if (!op_stack) {
          error = kBasicCalcAllocationFail;
          goto cleanup;
        }
        break;
      case ')':
        error = ShuntYardBrace(&num_stack, &op_stack);
        if (error != kBasicCalcErrorSuccess) {
          goto cleanup;
        }
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        error = ProcessNumber(&ptr, &num_stack);
        if (error != kBasicCalcErrorSuccess) {
          goto cleanup;
        } 
        prev_was_num = true;
        continue;
      case 'a':
      case 's':
      case 'c':
      case 't':
      case 'l':
        error = ProcessFunction(&ptr, &op_stack);
        if (error != kBasicCalcErrorSuccess) {
          goto cleanup;
        }
        continue;
      default:
        error = kBasicCalcErrorInvalidExpr;
        goto cleanup;
    }
    ++ptr;
  }
  while (op_stack->size) {
    error = ShuntYardAlgo(&num_stack, &op_stack);
    if (error != kBasicCalcErrorSuccess) {
      goto cleanup;
    }
  }
  if (num_stack->size != 1) {
    error = kBasicCalcErrorInvalidExpr;
  } else {
    *res = num_stack->top;
  }
cleanup:
  StackDoubleDelete(num_stack);
  StackOperationDelete(op_stack);
  free(expr);
  return error;
}

BasicCalcError CALL_CONV BasicCalculateEquation(const char* math_expr, const char* x, double* res) {
  char* expr = StrDup(math_expr);
  if (!expr) {
    return kBasicCalcAllocationFail;
  }
  BasicCalcError error = ReplaceXInString(&expr, x);
  if (error == kBasicCalcErrorSuccess) {
    error = BasicCalculateExpr(expr, res);
  }
  free(expr);
  return error;
}