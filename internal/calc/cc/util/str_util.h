#ifndef SMARTCALC_INTERNAL_UTIL_CC_CORE_STR_UTIL_H_
#define SMARTCALC_INTERNAL_UTIL_CC_CORE_STR_UTIL_H_

#include <string.h>

extern char* StrDup(const char* src);
extern char* StrInsert(char* restrict src, const char* restrict str, size_t idx);

#endif // SMARTCALC_INTERNAL_UTIL_CC_CORE_STR_UTIL_H_