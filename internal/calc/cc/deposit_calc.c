#include "deposit_calc.h"
#include "defs.h"
#include "util/vector.h"

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

static inline bool LastDayOfTheYear(Date* date) {
  return DateGetDay(date) == 31 && DateGetMonth(date) == 12;
}

static inline Date NextPayDate(Date pay_date, int freq) {
  static const int incr[] = {1, 7, 1, 3, 6, 12};
  if (freq < kDepositPayFreqEvMon) {
    DateAddDays(&pay_date, incr[freq]);
  } else {
    DateAddMonths(&pay_date, incr[freq]);
  }
  return pay_date;
}

static DepositCalcError AddReplenishment(DepositPayout** replen,
                                         Date* start_date,
                                         Date* finish_date,
                                         const DepositTransaction* transaction) {
  Date transact_date = transaction->payout.date;
  while (DateLessEqual(&transact_date, finish_date)) {
    if (DateGreaterEqual(&transact_date, start_date)) {
      DepositPayout payout = {.date = transact_date,
                              .sum = transaction->payout.sum
      };
      if (!VectorPush(*replen, payout)) {
        return kDepositCalcErrorAllocationFail;
      }
      if (transaction->freq == kDepositTransactionFreqOnce) {
        break;
      }
    }
    DateAddMonths(&transact_date, transaction->freq);
  }
  return kDepositCalcErrorSuccess;
}

static DepositCalcError CalculateDeposit(DepositData* data, const DepositConditions* conds) {
  Date start_date = conds->start_date;
  Date finish_date = data->finish_date;
  int am_days = DateDaysTo(&start_date, &finish_date);


  Date curr_date = conds->start_date;
  DateAddDays(&curr_date, 1);

  Date pay_date = NextPayDate(start_date, conds->pay_freq);
  if (!VectorPush(data->pay_dates, pay_date)) {
    return kDepositCalcErrorAllocationFail;
  }

  size_t i = 0, j = 0;
  for (; j < VectorSize(data->replen) && DateGreater(&curr_date, &data->replen[j].date); ++j) {
    if (conds->sum + data->replen[j].sum >= conds->non_taking_rem) {
      data->total += data->replen[j].sum;
    }
  }
  double add_sum = 0.0, cap_sum = 0.0, pay = 0.0;
  double year_perc = 0.0, non_add_pay = 0.0, non_add_perc = 0.0;
  for(;DateLessEqual(&curr_date, &finish_date); DateAddDays(&curr_date, 1)) {
    pay += (conds->sum + add_sum + cap_sum) * conds->intr_rate / DateDaysInYear(&curr_date);
    non_add_pay += (conds->sum + non_add_perc) * conds->intr_rate / DateDaysInYear(&curr_date);

    if (DateEqual(&curr_date, &data->pay_dates[i]) || DateEqual(&curr_date, &finish_date)) {
      pay_date = NextPayDate(curr_date, conds->pay_freq);
      double payment = round(pay) * 0.01;
      double non_add_payment = round(non_add_pay) * 0.01;

      if (!VectorPush(data->pay_dates, pay_date) ||
          !VectorPush(data->payments, payment)) {
        return kDepositCalcErrorAllocationFail;
      }
      data->perc_sum += payment;
      year_perc += payment;
      non_add_pay = pay = 0.0;
      if (conds->capt) {
        non_add_perc += non_add_payment;
        cap_sum = data->perc_sum;
      }
      ++i;
    }
    for (; j < VectorSize(data->replen) && DateEqual(&curr_date, &data->replen[j].date); ++j) {
      if (conds->sum + data->replen[j].sum + add_sum + cap_sum >= conds->non_taking_rem) {
        add_sum += data->replen[j].sum;
      }
    }
    if (LastDayOfTheYear(&curr_date) || (DateEqual(&curr_date, &finish_date) && VectorSize(data->taxes))) {
      double tax_inc = year_perc - conds->key_rate * 10000.0;
      year_perc = 0.0;
      if (tax_inc > 0.0) {
        double tax = round(tax_inc * conds->tax_rate) * 0.01;
        if (!VectorPush(data->taxes, tax)) {
          return kDepositCalcErrorAllocationFail;
        }
        data->tax_sum += tax;
      }
    }
  }
  if (conds->capt) {
    data->eff_rate = (non_add_perc * (double)kDatesConstsAvgDaysInYear * 100.0) / (conds->sum * am_days);
  }
  data->total += conds->sum + data->perc_sum + add_sum;

  return kDepositCalcErrorSuccess;
}

static Date CalcFinishDate(Date start_date, DepositTermType term_type, int term) {
  if (term_type == kDepositTermTypeDay) {
    DateAddDays(&start_date, term);
  } else if (term_type == kDepositTermTypeMonth) {
    DateAddMonths(&start_date, term);
  } else {
    DateAddYears(&start_date, term);
  }
  return start_date;
}

static int ComparePayouts(const void* lhs, const void* rhs) {
  DepositPayout* lhs_payout = (DepositPayout*)lhs;
  DepositPayout* rhs_payout = (DepositPayout*)rhs;
  if (DateLess(&lhs_payout->date, &rhs_payout->date)) {
    return -1;
  }
  if (DateGreater(&lhs_payout->date, &rhs_payout->date)) {
    return 1;
  }
  return 0;
}

static DepositCalcError SetReplenishments(DepositData* data, const DepositConditions* conds) {
  Date start_date = data->start_date;
  Date finish_date = data->finish_date;

  for (size_t i = 0; i < VectorSize(conds->fund); ++i) {
    DepositCalcError error = AddReplenishment(&data->replen, &start_date, &finish_date, conds->fund + i);
    if (error != kDepositCalcErrorSuccess) {
      return error;
    }
  }
  for(size_t i = 0; i < VectorSize(conds->wth); ++i) {
    DepositTransaction wth = conds->wth[i];
    wth.payout.sum = -wth.payout.sum;
    DepositCalcError error = AddReplenishment(&data->replen, &start_date, &finish_date, &wth);
    if (error != kDepositCalcErrorSuccess) {
      return error;
    }
  }
  qsort(data->replen, VectorSize(data->replen), sizeof(DepositPayout), ComparePayouts);

  return kDepositCalcErrorSuccess;
}

DepositCalcError CALL_CONV DepositCalculate(const DepositConditions* conds, DepositData* data) {
  *data = (DepositData){
    .start_date = conds->start_date,
    .finish_date = CalcFinishDate(conds->start_date, conds->term_type, conds->term)
  };
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
  DepositCalcError error = SetReplenishments(data, conds);
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