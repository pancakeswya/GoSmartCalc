package credit

/*
  #cgo CFLAGS: -I../..
  #include <stdlib.h>

  #define __GO
  #include "cc/core/credit_calc.h"
  #undef __GO

  typedef typeof(&CalculateCredit) CalcCreditFnPtr;
  typedef typeof(&FreeCreditData) FreeCreditDataFnPtr;

  inline void CallCalcCreditFnPtr(CalcCreditFnPtr fn_ptr, const CreditConditions* conds, CreditData* data) {
		return fn_ptr(conds, data);
  }

  inline void CallFreeCreditDataFnPtr(FreeCreditDataFnPtr fn_ptr, CreditData* data) {
		return fn_ptr(data);
  }
*/
import "C"
import (
	"github.com/pancakeswya/GoSmartCalc/pkg/dll"
	"github.com/pancakeswya/GoSmartCalc/pkg/util"
	"unsafe"
)

const (
	TypeAnnuit = int(C.kCreditTypeAnnuit)
	TypeDiff   = int(C.kCreditTypeDiff)
)

const (
	TermTypeMonth = int(C.kCreditTermTypeMonth)
	TermTypeYear  = int(C.kCreditTermTypeYear)
)

type Conditions struct {
	Sum        float64
	IntRate    float64
	Term       int
	TermType   int
	CreditType int
}

type Data struct {
	Total    float64
	Overpay  float64
	Payments []float64
}

type CalcFn func(Conditions) Data

type Calc struct {
	Calculate CalcFn
}

func NewCalc(dl dll.Dll) (*Calc, error) {
	calcCreditFnPtr := (C.CalcCreditFnPtr)(dl.GetSymbolPtr("CalculateCredit"))
	if calcCreditFnPtr == nil {
		return nil, dl.Error()
	}
	freeCreditDataFnPtr := (C.FreeCreditDataFnPtr)(dl.GetSymbolPtr("FreeCreditData"))
	if freeCreditDataFnPtr == nil {
		return nil, dl.Error()
	}
	bc := &Calc{
		Calculate: func(conds Conditions) Data {
			cconds := C.CreditConditions{
				sum:         C.double(conds.Sum),
				int_rate:    C.double(conds.IntRate),
				term:        C.short(conds.Term),
				term_type:   C.int(conds.TermType),
				credit_type: C.int(conds.CreditType),
			}
			var cdata C.CreditData
			C.CallCalcCreditFnPtr(calcCreditFnPtr, &cconds, &cdata)
			defer C.CallFreeCreditDataFnPtr(freeCreditDataFnPtr, &cdata)
			return Data{
				Total:    float64(cdata.total),
				Overpay:  float64(cdata.overpay),
				Payments: util.CArray2Go(unsafe.Pointer(cdata.payments), uint64(cdata.payments_size)),
			}
		},
	}
	return bc, nil
}
