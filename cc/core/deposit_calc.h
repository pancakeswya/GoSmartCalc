#ifndef SMARTCALC_CC_CORE_DEPOSIT_CALC_H_
#define SMARTCALC_CC_CORE_DEPOSIT_CALC_H_

#include "calc_api.h"

#include <stddef.h>

#ifndef __GO
extern "C" {
#endif

typedef enum { kDepositTermTypeDay, kDepositTermTypeMonth, kDepositTermTypeYear } DepositTermType;

typedef enum {
  kDepositPayFreqEvDay,
  kDepositPayFreqEvWeek,
  kDepositPayFreqEvMon,
  kDepositPayFreqEvQuart,
  kDepositPayFreqEvHalfYear,
  kDepositPayFreqEvYear
} DepositPayFreq;

typedef struct {
  int date[3];
  double sum;
} DepositPayout;

typedef struct {
  DepositPayout payout;
  int freq;
} DepositTransaction;

typedef struct {
  DepositPayout *replen;
  size_t replen_size;

  int **pay_dates;
  size_t pay_dates_size;

  double *payment;
  size_t payment_size;

  double *tax;
  size_t tax_size;

  int start_date[3];
  int finish_date[3];
  double eff_rate;
  double perc_sum;
  double tax_sum;
  double total;
} DepositData;

typedef struct {
  short int term_type;
  short int term;

  int cap;
  int pay_freq;

  double tax_rate;
  double key_rate;
  double sum;
  double intr_rate;
  double non_taking_rem;

  int start_date[3];

  DepositTransaction *fund;
  size_t fund_size;

  DepositTransaction *wth;
  size_t wth_size;
} DepositConditions;

extern CALC_API void DepositCalculate(const DepositConditions* conds, DepositData* data);
extern CALC_API void DepositFreeData(DepositData* data);

#ifndef __GO
} // extern "C"
#endif

#endif  // SMARTCALC_CC_CORE_DEPOSIT_CALC_H_