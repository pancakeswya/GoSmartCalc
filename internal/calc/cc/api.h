#ifndef SMARTCALC_INTERNAL_CALC_CC_CORE_CALC_API_H_
#define SMARTCALC_INTERNAL_CALC_CC_CORE_CALC_API_H_

#ifdef CALC_SHARED
#   if defined(_WIN32) && !defined(__MINGW32__)
#       ifdef CALC_EXPORT
#         define CALC_API __declspec(dllexport)
#       else
#         define CALC_API __declspec(dllimport)
#       endif
#       define CALL_CONV __stdcall
#   else
#     define CALC_API __attribute__ ((visibility ("default")))
#     define CALL_CONV
#   endif
#else
#   define CALC_API
#   define CALL_CONV
#endif

#endif // SMARTCALC_INTERNAL_CALC_CC_CORE_CALC_API_H_