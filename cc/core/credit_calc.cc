#include "core/credit_calc.h"
#include "core/defs.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>

namespace {

CreditData CalculateAnnuit(const CreditConditions& conds, int mp_cnt) noexcept {
  CreditData data = {};
  double r = conds.int_rate / (kDatesConstsMonthInYear * 100);
  double ann_k =
      (r * std::pow(1 + r, mp_cnt)) / ((std::pow(1 + r, mp_cnt)) - 1);
  double ann_pay = std::round(conds.sum * ann_k * 100.0) / 100.0;

  data.payments = reinterpret_cast<double*>(malloc(mp_cnt * sizeof(double)));
  assert(data.payments);
  data.payments_size = mp_cnt;

  std::fill(data.payments, data.payments + mp_cnt, ann_pay);

  data.total = ann_pay * mp_cnt;
  data.overpay = data.total - conds.sum;
  return data;
}

CreditData CalculateDiff(const CreditConditions& conds, int mp_cnt) noexcept {
  CreditData data = {};
  double rest = conds.sum, payment;
  double mp_real = conds.sum / (mp_cnt);

  data.payments = reinterpret_cast<double*>(malloc(mp_cnt * sizeof(double)));
  assert(data.payments);
  data.payments_size = mp_cnt;

  for (int i = 0; i < mp_cnt; ++i) {
    payment = mp_real + (rest * conds.int_rate / (kDatesConstsMonthInYear * 100));
    data.payments[i] = payment;
    data.total += payment;
    rest -= mp_real;
  }
  data.overpay = data.total - conds.sum;
  return data;
}

}  // namespace

void CALL_CONV CreditCalculate(const CreditConditions* conds, CreditData* data) {
  int mp_cnt = (conds->term_type == kCreditTermTypeYear) ? conds->term * kDatesConstsMonthInYear
                                                         : conds->term;
  if (conds->credit_type == kCreditTypeAnnuit) {
    *data = CalculateAnnuit(*conds, mp_cnt);
  } else {
    *data = CalculateDiff(*conds, mp_cnt);
  }
}

void CALL_CONV CreditFreeData(CreditData* data) {
  free(data->payments);
  *data = {};
}