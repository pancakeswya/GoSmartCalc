#include "core/deposit_calc.h"
#include "core/defs.h"
#include "datetime.h"

#include <cassert>
#include <algorithm>
#include <vector>

namespace {

using Date = jed_utils::datetime;

struct Transaction {
  struct Payout {
    Date date;
    double sum;
  } payout;
  int freq;
};

struct Data {
  std::vector<Transaction::Payout> replen;
  std::vector<Date> pay_dates;
  std::vector<double> payment;
  std::vector<double> tax;
  Date start_date;
  Date finish_date;
  double eff_rate;
  double perc_sum;
  double tax_sum;
  double total;
};

struct Conditions {
  bool cap;
  short int term_type;
  short int term;
  int pay_freq;
  double tax_rate;
  double key_rate;
  double sum;
  double intr_rate;
  double non_taking_rem;
  Date start_date;
  std::vector<Transaction> fund;
  std::vector<Transaction> wth;
};

template<typename Tp>
inline Tp* VectorToCArray(const std::vector<Tp>& vec) {
  auto array = reinterpret_cast<Tp*>(malloc(vec.size() * sizeof(Tp)));
  assert(array);
  std::copy(vec.begin(), vec.end(), array);
  return array;
}

inline void SetValidPayDate(Date& date, int day) noexcept {
  if (date.get_day() != day && date.days_in_month() > date.get_day()) {
    date = Date(date.get_year(), date.get_month(), day);
  }
}

inline Date NextPayDate(const Date& cur_date, int freq, int payday,
                        int incr) noexcept {
  Date pay_date = cur_date;
  if (freq < kDepositPayFreqEvMon) {
    pay_date.add_days(incr);
  } else {
    pay_date.add_months(incr);
    SetValidPayDate(pay_date, payday);
  }
  return pay_date;
}

inline int ReplenFreq(int pay_freq) noexcept {
  if (pay_freq == kDepositPayFreqEvHalfYear) {
    return 6;
  } else if (pay_freq == kDepositPayFreqEvYear) {
    return 12;
  }
  return pay_freq;
}

int PayFreq(int pay_freq) noexcept {
  static int real_freq[] = {1, 7, 1, 3, 6, 12};
  return real_freq[pay_freq];
}

void AddReplenishment(Data& data, const Date& start_date,
                      const Date& finish_date, const Transaction& transaction) {
  Date transact_date = transaction.payout.date;
  while (transact_date <= finish_date) {
    if (transact_date > start_date) {
      data.replen.push_back({transact_date, transaction.payout.sum});
      if (transaction.freq == kDepositPayFreqEvDay) {
        break;
      }
    }
    transact_date.add_months(ReplenFreq(transaction.freq));
    SetValidPayDate(transact_date, transact_date.get_day());
  }
}

void CalculateDeposit(Data& data, const Conditions& conds) {
  int pay_freq = PayFreq(conds.pay_freq);
  int am_days = conds.start_date.days_to(data.finish_date);

  double tax_perc = 0.0, add_sum = 0.0, cap_sum = 0.0, pay = 0.0;

  Date date = conds.start_date;
  date.add_days(1);

  data.pay_dates.push_back(NextPayDate(conds.start_date, conds.pay_freq,
                                       conds.start_date.get_day(), pay_freq));

  size_t i = 0, j = 0;
  while (date <= data.finish_date) {
    pay +=
        (conds.sum + add_sum + cap_sum) * conds.intr_rate / date.days_in_year();
    if (date == data.pay_dates[i] || date == data.finish_date) {
      data.pay_dates.push_back(NextPayDate(
          date, conds.pay_freq, conds.start_date.get_day(), pay_freq));
      data.payment.push_back(std::round(pay) / 100.0);
      data.perc_sum += data.payment.back();
      pay = 0.0;
      if (conds.cap) {
        cap_sum = data.perc_sum;
      }
      ++i;
    }
    for (; j < data.replen.size() && date == data.replen[j].date; ++j) {
      if (conds.sum + data.replen[j].sum + add_sum + cap_sum >=
          conds.non_taking_rem) {
        add_sum += data.replen[j].sum;
      }
    }
    if ((date.get_day() == 31 && date.get_month() == 12) ||
        (date == data.finish_date && !data.tax.empty())) {
      double tax_inc = data.perc_sum - tax_perc - conds.key_rate * 10000.0;
      if (tax_inc > 0.0) {
        data.tax.push_back(tax_inc * (conds.tax_rate / 100.0));
        data.tax_sum += data.tax.back();
      }
      tax_perc = data.perc_sum;
    }
    date.add_days(1);
  }
  if (conds.cap) {
    data.eff_rate = (data.perc_sum * double(kDatesConstsAvgDaysInYear) * 100.0) /
                    (conds.sum * am_days);
    data.total += cap_sum;
  }
  data.total += conds.sum + add_sum;
}

Date FinishDate(const Conditions& conds) noexcept {
  Date finish_date = conds.start_date;
  if (conds.term_type == kDepositTermTypeDay) {
    finish_date.add_days(conds.term);
  } else if (conds.term_type == kDepositTermTypeMonth) {
    finish_date.add_months(conds.term);
  } else {
    finish_date.add_years(conds.term);
  }
  return finish_date;
}

void SetReplanishments(Data& data, Conditions& conds) {
  for (const Transaction& single_fund : conds.fund) {
    AddReplenishment(data, conds.start_date, data.finish_date, single_fund);
  }
  for (Transaction& single_wth : conds.wth) {
    single_wth.payout.sum = -single_wth.payout.sum;
    AddReplenishment(data, conds.start_date, data.finish_date, single_wth);
  }
  std::sort(data.replen.begin(), data.replen.end(),
            [](const Transaction::Payout& rhs, const Transaction::Payout& lhs) {
              return rhs.date < lhs.date;
  });
}

Data Calculate(Conditions conds) {
  Data data{};
  data.start_date = conds.start_date;
  data.finish_date = FinishDate(conds);
  SetReplanishments(data, conds);
  CalculateDeposit(data, conds);
  return data;
}

std::vector<Transaction> TransactionConv(const DepositTransaction* transactions, size_t len) {
  std::vector<Transaction> result(len);
  for(size_t i = 0; i < len; ++i) {
    result[i] = {
        .payout = { .date = Date(transactions[i].payout.date[0],
                                 transactions[i].payout.date[1],
                                 transactions[i].payout.date[2]),
                    .sum = transactions[i].payout.sum },
        .freq = transactions[i].freq
    };
  }
  return result;
}

DepositPayout* PayoutsConv(const std::vector<Transaction::Payout>& payouts) noexcept {
  size_t payouts_size = payouts.size();
  auto cpayouts = reinterpret_cast<DepositPayout*>(malloc(payouts_size * sizeof(DepositPayout)));
  assert(cpayouts);
  for(size_t i = 0; i < payouts_size; ++i) {
    cpayouts[i] = {
        .date = { payouts[i].date.get_year(),
                  payouts[i].date.get_month(),
                  payouts[i].date.get_day() },
        .sum = payouts[i].sum
    };
  }
  return cpayouts;
}

int** OneBlockAlloc(size_t rows, size_t cols) {
  auto block = reinterpret_cast<int**>(malloc(rows * cols * sizeof(int) + rows * sizeof(int *)));
  assert(block);
  int *ptr = reinterpret_cast<int*>(block + rows);
  for (size_t i = 0; i < rows; ++i) {
    block[i] = ptr + cols * i;
  }
  return block;
}

int** DatesConv(const std::vector<Date>& dates) noexcept {
  size_t dates_size = dates.size();
  int** cdates = OneBlockAlloc(dates_size, 3);
  for(size_t i = 0; i < dates_size; ++i) {
    cdates[i][0] = dates[i].get_year();
    cdates[i][1] = dates[i].get_month();
    cdates[i][2] = dates[i].get_day();
  }
  return cdates;
}

}  // namespace

void CALL_CONV CalculateDeposit(const DepositConditions* conds, DepositData* data) {
  Conditions cpp_conds = {
      .cap = static_cast<bool>(conds->cap),
      .term_type = conds->term_type,
      .term = conds->term,
      .pay_freq = conds->pay_freq,
      .tax_rate = conds->tax_rate,
      .key_rate = conds->key_rate,
      .sum = conds->sum,
      .intr_rate = conds->intr_rate,
      .non_taking_rem = conds->non_taking_rem,
      .start_date = Date(conds->start_date[0],
                         conds->start_date[1],
                         conds->start_date[2]),
      .fund = TransactionConv(conds->fund, conds->fund_size),
      .wth = TransactionConv(conds->wth, conds->wth_size)
  };
  Data cpp_data = Calculate(cpp_conds);
  *data = {
    .replen = PayoutsConv(cpp_data.replen),
    .replen_size = cpp_data.replen.size(),
    .pay_dates = DatesConv(cpp_data.pay_dates),
    .pay_dates_size = cpp_data.pay_dates.size(),
    .payment = VectorToCArray(cpp_data.payment),
    .payment_size = cpp_data.payment.size(),
    .tax = VectorToCArray(cpp_data.tax),
    .tax_size = cpp_data.tax.size(),
    .start_date = { cpp_data.start_date.get_year(),
                    cpp_data.start_date.get_month(),
                    cpp_data.start_date.get_day() },
    .finish_date = { cpp_data.finish_date.get_year(),
                     cpp_data.finish_date.get_month(),
                     cpp_data.finish_date.get_day() },
    .eff_rate = cpp_data.eff_rate,
    .perc_sum = cpp_data.perc_sum,
    .tax_sum = cpp_data.tax_sum,
    .total = cpp_data.total
  };
}

void CALL_CONV FreeDepositData(DepositData* data) {
  free(data->replen);
  free(data->tax);
  free(data->payment);
  free(data->pay_dates);
  *data = {};
}