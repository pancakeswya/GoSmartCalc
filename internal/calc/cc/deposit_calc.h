#ifndef SMARTCALC_INTERNAL_CALC_CC_CORE_DEPOSIT_CALC_H_
#define SMARTCALC_INTERNAL_CALC_CC_CORE_DEPOSIT_CALC_H_

#include "api.h"
#include "util/date.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { kDepositCalcErrorSuccess, kDepositCalcErrorAllocationFail } DepositCalcError;
typedef enum { kDepositTermTypeDay, kDepositTermTypeMonth, kDepositTermTypeYear } DepositTermType;

typedef enum {
  kDepositTransactionFreqOnce = 0,
  kDepositTransactionFreqEvMon,
  kDepositTransactionFreqEv2Mon,
  kDepositTransactionFreqQuart,
  kDepositTransactionFreqEvHalfYear = 6,
  kDepositTransactionFreqEvYear = 12
} DepositTransactionFreq;

typedef enum {
  kDepositPayFreqEvDay = 0,
  kDepositPayFreqEvWeek,
  kDepositPayFreqEvMon,
  kDepositPayFreqEvQuart,
  kDepositPayFreqEvHalfYear,
  kDepositPayFreqEvYear
} DepositPayFreq;

typedef struct {
  Date date;
  double sum;
} DepositPayout;

typedef struct {
  DepositPayout payout;
  DepositTransactionFreq freq;
} DepositTransaction;

typedef struct {
  DepositPayout *replen;
  Date *pay_dates;

  double *payments;
  double *taxes;

  Date start_date;
  Date finish_date;

  double eff_rate;
  double perc_sum;
  double tax_sum;
  double total;
} DepositData;

typedef struct {
  DepositTermType term_type;
  unsigned short int term;

  int capt;
  DepositPayFreq pay_freq;

  double tax_rate;
  double key_rate;
  double sum;
  double intr_rate;
  double non_taking_rem;

  Date start_date;
  DepositTransaction* fund;
  DepositTransaction* wth;
} DepositConditions;

extern CALC_API DepositCalcError DepositCalculate(const DepositConditions* conds, DepositData* data);
extern CALC_API void DepositDestroyData(DepositData* data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif  // SMARTCALC_INTERNAL_CALC_CC_CORE_DEPOSIT_CALC_H_