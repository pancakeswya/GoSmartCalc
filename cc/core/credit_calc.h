#ifndef SMARTCALC_CC_CORE_CREDIT_CALC_H_
#define SMARTCALC_CC_CORE_CREDIT_CALC_H_

#include <stddef.h>

#ifndef __GO
extern "C" {
#endif

enum CreditType { kCreditTypeAnnuit, kCreditTypeDiff };

enum CreditTermType { kCreditTermTypeMonth, kCreditTermTypeYear };

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

extern CreditData CalculateCredit(CreditConditions conds);

#ifndef __GO
}; // extern "C"
#endif


#endif  // SMARTCALC_CC_CORE_CREDIT_CALC_H_