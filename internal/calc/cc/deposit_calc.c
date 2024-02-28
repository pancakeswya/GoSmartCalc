#include "deposit_calc.h"
#include "defs.h"
#include "util/vector.h"

#include <stdlib.h>
#include <math.h>

static inline void SetValidPayDate(Date* date, int day) {
  int curr_day = DateGetDay(date);
  if (curr_day != day && DateDaysInMonth(date) > curr_day) {
    *date = DateNew(DateGetYear(date), DateGetMonth(date), day);
  }
}

static inline Date NextPayDate(Date pay_date, int freq, int payday, int incr) {
  if (freq < kDepositPayFreqEvMon) {
    DateAddDays(&pay_date, incr);
  } else {
    DateAddMonths(&pay_date, incr);
    SetValidPayDate(&pay_date, payday);
  }
  return pay_date;
}

static int PayFreq(DepositPayFreq pay_freq) {
  static const int real_freq[] = {1, 7, 1, 3, 6, 12};
  return real_freq[pay_freq];
}

static DepositCalcError AddReplenishment(DepositPayout** replen,
                                         Date* start_date,
                                         Date* finish_date,
                                         DepositTransaction* transaction) {
  Date* transact_date = &transaction->payout.date;
  while (DateLessEqual(transact_date, finish_date)) {
    if (DateGreater(transact_date, start_date)) {
      DepositPayout payout = {.date = *transact_date,
                              .sum = transaction->payout.sum
      };
      if (!VectorPush(*replen, payout)) {
        return kDepositCalcErrorAllocationFail;
      }
      if (transaction->freq == kDepositPayFreqEvDay) {
        break;
      }
    }
    DateAddMonths(transact_date, transaction->freq);
    SetValidPayDate(transact_date, DateGetDay(transact_date));
  }
  return kDepositCalcErrorSuccess;
}

static DepositCalcError CalculateDeposit(DepositData* data, DepositConditions* conds) {
  int pay_freq = PayFreq(conds->pay_freq);

  Date* start_date = &conds->start_date;
  Date* finish_date = &data->finish_date;
  int am_days = DateDaysTo(start_date, finish_date);

  double tax_perc = 0.0, add_sum = 0.0, cap_sum = 0.0, pay = 0.0;

  Date date = conds->start_date;
  DateAddDays(&date, 1);

  Date pay_date = NextPayDate(*start_date, conds->pay_freq, DateGetDay(start_date), pay_freq);
  if (!VectorPush(data->pay_dates, pay_date)) {
    return kDepositCalcErrorAllocationFail;
  }

  size_t i = 0, j = 0;
  while (DateLessEqual(&date, finish_date)) {
    pay += (conds->sum + add_sum + cap_sum) * conds->intr_rate / DateDaysInYear(&date);

    if (DateEqual(&date, &data->pay_dates[i]) || DateEqual(&date, finish_date)) {
      pay_date = NextPayDate(date, conds->pay_freq, DateGetDay(start_date), pay_freq);
      double payment = round(pay) / 100.0;

      if (!VectorPush(data->pay_dates, pay_date) ||
          !VectorPush(data->payments, payment)) {
        return kDepositCalcErrorAllocationFail;
      }
      data->perc_sum += data->payments[VectorSize(data->payments) - 1];
      pay = 0.0;
      if (conds->capt) {
        cap_sum = data->perc_sum;
      }
      ++i;
    }
    for (; j < VectorSize(data->replen) && DateEqual(&date, &data->replen[j].date); ++j) {
      if (conds->sum + data->replen[j].sum + add_sum + cap_sum >= conds->non_taking_rem) {
        add_sum += data->replen[j].sum;
      }
    }
    if ((DateGetDay(&date) == 31 && DateGetMonth(&date) == 12) ||
        (DateEqual(&date, finish_date) && VectorSize(data->taxes))) {
      double tax_inc = data->perc_sum - tax_perc - conds->key_rate * 10000.0;
      if (tax_inc > 0.0) {
        double tax = tax_inc * (conds->tax_rate / 100.0);
        if (!VectorPush(data->taxes, tax)) {
          return kDepositCalcErrorAllocationFail;
        }
        data->tax_sum += data->taxes[VectorSize(data->taxes) - 1];
      }
      tax_perc = data->perc_sum;
    }
    DateAddDays(&date, 1);
  }
  if (conds->capt) {
    data->eff_rate = (data->perc_sum * (double)kDatesConstsAvgDaysInYear * 100.0) / (conds->sum * am_days);
    data->total += cap_sum;
  }
  data->total += conds->sum + add_sum;

  return kDepositCalcErrorSuccess;
}

static Date FinishDate(const DepositConditions* conds) {
  Date finish_date = conds->start_date;
  if (conds->term_type == kDepositTermTypeDay) {
    DateAddDays(&finish_date, conds->term);
  } else if (conds->term_type == kDepositTermTypeMonth) {
    DateAddMonths(&finish_date, conds->term);
  } else {
    DateAddYears(&finish_date, conds->term);
  }
  return finish_date;
}

static int ComparePayouts(const void* lhs, const void* rhs) {
  DepositPayout* lhs_payout = (DepositPayout*)lhs;
  DepositPayout* rhs_payout = (DepositPayout*)rhs;

  return DateLess(&lhs_payout->date, &rhs_payout->date);
}

static DepositCalcError SetReplanishments(DepositData* data, DepositConditions* conds) {
  Date* start_date = &conds->start_date;
  Date* finish_date = &data->finish_date;

  for (size_t i = 0; i < VectorSize(conds->fund); ++i) {
    DepositCalcError error = AddReplenishment(&data->replen, start_date, finish_date, conds->fund + i);
    if (error != kDepositCalcErrorSuccess) {
      return error;
    }
  }
  for(size_t i = 0; i < VectorSize(conds->wth); ++i) {
    conds->wth[i].payout.sum = -conds->wth[i].payout.sum;
    DepositCalcError error = AddReplenishment(&data->replen, start_date, finish_date, conds->wth + i);
    if (error != kDepositCalcErrorSuccess) {
      return error;
    }
  }
  qsort(data->replen, VectorSize(data->replen), sizeof(DepositPayout), ComparePayouts);

  return kDepositCalcErrorSuccess;
}

DepositCalcError CALL_CONV DepositCalculate(DepositConditions* conds, DepositData* data) {
  *data = (DepositData){0};
  data->pay_dates = VectorNew(Date);
  if (!data->pay_dates) {
    return kDepositCalcErrorAllocationFail;
  }
  data->replen = VectorNew(DepositPayout);
  if (!data->replen) {
    return kDepositCalcErrorAllocationFail;
  }
  data->payments = VectorNew(double);
  if (!data->payments) {
    return kDepositCalcErrorAllocationFail;
  }
  data->taxes = VectorNew(double);
  if (!data->taxes) {
    return kDepositCalcErrorAllocationFail;
  }

  data->start_date = conds->start_date;
  data->finish_date = FinishDate(conds);

  DepositCalcError error = SetReplanishments(data, conds);
  if (error != kDepositCalcErrorSuccess) {
    return error;
  }
  return CalculateDeposit(data, conds);
}

void CALL_CONV DepositDestroyData(DepositData* data) {
  VectorDelete(data->replen);
  VectorDelete(data->taxes);
  VectorDelete(data->payments);
  VectorDelete(data->pay_dates);
  *data = (DepositData){0};
}