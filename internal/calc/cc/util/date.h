#ifndef SMARTCALC_INTERNAL_UTIL_CC_CORE_DATE_H_
#define SMARTCALC_INTERNAL_UTIL_CC_CORE_DATE_H_

#include <stdbool.h>
#include <time.h>
#include <math.h>

typedef struct tm Date;

static const int kOneDay = 86400;
static const int kOneHour = 3600;
static const int kOneMinute = 60;

static inline bool IsLeapYear(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static inline Date DateNewCopy(const Date* date) {
  Date copy = *date;
  return copy;
}

static inline Date DateNewFromArray(const int array[3]) {
  Date date = {
      .tm_year = array[0] - 1900,
      .tm_mon = array[1] - 1,
      .tm_mday = array[2],
      .tm_isdst = -1
  };
  mktime(&date);
  return date;
}

static inline Date DateNew(int year, int month, int day) {
  Date date = {
    .tm_year = year - 1900,
    .tm_mon = month - 1,
    .tm_mday = day,
    .tm_isdst = -1
  };
  mktime(&date);
  return date;
}

static inline int DateGetYear(const Date* date) {
  return date->tm_year + 1900;
}

static inline int DateGetMonth(const Date* date) {
  return date->tm_mon + 1;
}

static inline int DateGetDay(const Date* date) {
  return date->tm_mday;
}

static inline int DateDaysInMonth(const Date* date) {
  static const int month_days[] = {31, 28, 31, 30,
                                   31, 30, 31, 31,
                                   30, 31, 30, 31};
  int month = DateGetMonth(date);
  if (month == 2) {
    return month_days[month - 1] + IsLeapYear(DateGetYear(date));
  }
  return month_days[month - 1];
}

static inline int DateDaysInYear(const Date* date) {
  return 365 + IsLeapYear(DateGetYear(date));
}

static inline int DateDaysTo(Date* src, Date* dst) {
  return (int)difftime(mktime(src), mktime(dst)) / (60 * 60 * 24);
}

static inline bool DateLess(Date* mdt, Date* odt) {
  return mktime(mdt) < mktime(odt);
}

static inline bool DateGreater(Date* mdt, Date* odt) {
  return mktime(odt) < mktime(mdt);
}

static inline bool DateLessEqual(Date* mdt, Date* odt) {
  return mktime(mdt) <= mktime(odt);
}

static inline bool DateGreaterEqual(Date* mdt, Date* odt) {
  return mktime(mdt) >= mktime(odt);
}

static inline bool DateEqual(Date* mdt, Date* odt) {
  return mktime(mdt) == mktime(odt);
}

static inline bool DateNotEqual(Date* mdt, Date* odt) {
  return mktime(mdt) != mktime(odt);
}

static inline void DateAddYears(Date* date, int nb_years) {
  date->tm_year += nb_years;
}

static inline void DateAddMonths(Date* date, int nb_months) {
  int nb_year = (int)(ceil(nb_months / 12));
  int nb_months_final = nb_months % 12;

  if (date->tm_mon + nb_months_final > 11) {
    nb_year++;
    nb_months_final = (date->tm_mon + nb_months_final) - 12;
    date->tm_mon = nb_months_final;
  } else {
    date->tm_mon += nb_months_final;
  }
  date->tm_year += nb_year;
}

static inline void DateAddSeconds(Date* date, int nb_seconds) {
  time_t new_seconds = mktime(date) + nb_seconds;
  *date = *localtime(&new_seconds);
}

static inline void DateAddDays(Date* date, int nb_days) {
  DateAddSeconds(date, nb_days * kOneDay);
}

static inline void DateAddHours(Date* date, int nb_hours) {
  DateAddSeconds(date, nb_hours * kOneHour);
}

static inline void DateAddMinutes(Date* date, int nb_minutes) {
  DateAddSeconds(date, nb_minutes * kOneMinute);
}

#endif // SMARTCALC_INTERNAL_UTIL_CC_CORE_DATE_H_