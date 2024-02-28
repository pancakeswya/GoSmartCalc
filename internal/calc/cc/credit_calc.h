#ifndef SMARTCALC_INTERNAL_CALC_CC_CORE_CREDIT_CALC_H_
#define SMARTCALC_INTERNAL_CALC_CC_CORE_CREDIT_CALC_H_

#include "api.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { kCreditCalcErrorSuccess, kCreditCalcErrorAllocationFail } CreditCalcError;
typedef enum { kCreditTypeAnnuit, kCreditTypeDiff } CreditType;
typedef enum { kCreditTermTypeMonth, kCreditTermTypeYear } CreditTermType;

typedef struct {
  double sum;
  double int_rate;
  unsigned short int term;
  CreditTermType term_type;
  CreditType credit_type;
} CreditConditions;

typedef struct {
  double total;
  double overpay;
  double* payments;
  size_t payments_size;
} CreditData;

extern CALC_API CreditCalcError CreditCalculate(const CreditConditions* conds, CreditData* data);
extern CALC_API void CreditDestroyData(CreditData* data);

#ifdef __cplusplus
}; // extern "C"
#endif


#endif  // SMARTCALC_INTERNAL_CALC_CC_CORE_CREDIT_CALC_H_