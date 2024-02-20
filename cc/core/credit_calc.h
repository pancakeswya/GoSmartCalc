#ifndef SMARTCALC_CC_CORE_CREDIT_CALC_H_
#define SMARTCALC_CC_CORE_CREDIT_CALC_H_

#include "calc_api.h"

#include <stddef.h>

#ifndef __GO
extern "C" {
#endif

typedef enum { kCreditTypeAnnuit, kCreditTypeDiff } CreditType;

typedef enum { kCreditTermTypeMonth, kCreditTermTypeYear } CreditTermType;

typedef struct {
  double sum;
  double int_rate;
  short int term;
  int term_type;
  int credit_type;
} CreditConditions;

typedef struct {
  double total;
  double overpay;
  double* payments;
  size_t payments_size;
} CreditData;

extern CALC_API void CreditCalculate(const CreditConditions* conds, CreditData* data);
extern CALC_API void CreditFreeData(CreditData* data);

#ifndef __GO
}; // extern "C"
#endif


#endif  // SMARTCALC_CC_CORE_CREDIT_CALC_H_