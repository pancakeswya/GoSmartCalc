package calc

/*
  #cgo CFLAGS: -I../..
  #include <stdlib.h>

  #define __GO
  #include "cc/core/credit_calc.h"
  #undef __GO

  typedef typeof(&CreditCalculate) CreditCalcFnPtr;
  typedef typeof(&CreditFreeData) CreditFreeDataFnPtr;

  inline void CallCreditCalcFnPtr(CreditCalcFnPtr fn_ptr, const CreditConditions* conds, CreditData* data) {
		return fn_ptr(conds, data);
  }

  inline void CallCreditFreeDataFnPtr(CreditFreeDataFnPtr fn_ptr, CreditData* data) {
		return fn_ptr(data);
  }
*/
import "C"
import (
	"github.com/pancakeswya/GoSmartCalc/pkg/cconv"
	"github.com/pancakeswya/GoSmartCalc/pkg/dll"
	"unsafe"
)

const (
	CreditTypeAnnuit = int(C.kCreditTypeAnnuit)
	CreditTypeDiff   = int(C.kCreditTypeDiff)
)

const (
	CreditTermTypeMonth = int(C.kCreditTermTypeMonth)
	CreditTermTypeYear  = int(C.kCreditTermTypeYear)
)

type CreditConditions struct {
	Sum        float64
	IntRate    float64
	Term       int
	TermType   int
	CreditType int
}

type CreditData struct {
	Total    float64
	Overpay  float64
	Payments []float64
}

type CreditCalcFn func(CreditConditions) CreditData

type CreditCalc struct {
	Calculate CreditCalcFn
}

func NewCredit(dl dll.Dll) (*CreditCalc, error) {
	var creditCalcFnPtr C.CreditCalcFnPtr
	var creditFreeDataFnPtr C.CreditFreeDataFnPtr
	{
		ptr, err := dl.GetSymbolPtr("CreditCalculate")
		if err != nil {
			return nil, err
		}
		creditCalcFnPtr = C.CreditCalcFnPtr(ptr)

		ptr, err = dl.GetSymbolPtr("CreditFreeData")
		if err != nil {
			return nil, err
		}
		creditFreeDataFnPtr = C.CreditFreeDataFnPtr(ptr)
	}
	bc := &CreditCalc{
		Calculate: func(conds CreditConditions) CreditData {
			cconds := C.CreditConditions{
				sum:         C.double(conds.Sum),
				int_rate:    C.double(conds.IntRate),
				term:        C.short(conds.Term),
				term_type:   C.int(conds.TermType),
				credit_type: C.int(conds.CreditType),
			}
			var cdata C.CreditData
			C.CallCreditCalcFnPtr(creditCalcFnPtr, &cconds, &cdata)
			defer C.CallCreditFreeDataFnPtr(creditFreeDataFnPtr, &cdata)
			return CreditData{
				Total:    float64(cdata.total),
				Overpay:  float64(cdata.overpay),
				Payments: cconv.CDoubleArray2Go(unsafe.Pointer(cdata.payments), uint64(cdata.payments_size)),
			}
		},
	}
	return bc, nil
}
