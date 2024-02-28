#include "credit_calc.h"
#include "defs.h"

#include <stdlib.h>
#include <math.h>

static void CalculateAnnuit(const CreditConditions* conds, CreditData* data) {
  double r = conds->int_rate / (kDatesConstsMonthInYear * 100);
  double ann_k =
    (r * pow(1 + r, (double)data->payments_size)) / ((pow(1 + r, (double)data->payments_size)) - 1);
  double ann_pay = round(conds->sum * ann_k * 100.0) / 100.0;

  for(size_t i = 0; i < data->payments_size; ++i) {
    data->payments[i] = ann_pay;
  }

  data->total = ann_pay * (double)data->payments_size;
  data->overpay = data->total - conds->sum;
}

static void CalculateDiff(const CreditConditions* conds, CreditData* data)  {
  double rest = conds->sum, payment;
  double mp_real = conds->sum / (double)data->payments_size;

  for (size_t i = 0; i < data->payments_size; ++i) {
    payment = mp_real + (rest * conds->int_rate / (kDatesConstsMonthInYear * 100));
    data->payments[i] = payment;
    data->total += payment;
    rest -= mp_real;
  }
  data->overpay = data->total - conds->sum;
}

CreditCalcError CALL_CONV CreditCalculate(const CreditConditions* conds, CreditData* data) {
  data->payments_size = (conds->term_type == kCreditTermTypeYear) ? conds->term * kDatesConstsMonthInYear
                                                                  : conds->term;
  data->payments = (double*)malloc(data->payments_size * sizeof(double));
  if (!data->payments) {
    return kCreditCalcErrorAllocationFail;
  }
  if (conds->credit_type == kCreditTypeAnnuit) {
    CalculateAnnuit(conds, data);
  } else {
    CalculateDiff(conds, data);
  }
  return kCreditCalcErrorSuccess;
}

void CALL_CONV CreditDestroyData(CreditData* data) {
  free(data->payments);
  *data = (CreditData){0};
}