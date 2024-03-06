package calc

/*
  #include "cc/credit_calc.h"

  typedef typeof(&CreditCalculate) CreditCalcFnPtr;
  typedef typeof(&CreditDestroyData) CreditDestroyDataFnPtr;

  static inline CreditCalcError CallCreditCalcFnPtr(CreditCalcFnPtr fn_ptr, const CreditConditions* conds, CreditData* data) {
		return fn_ptr(conds, data);
  }

  static inline void CallCreditDestroyDataFnPtr(CreditDestroyDataFnPtr fn_ptr, CreditData* data) {
		return fn_ptr(data);
  }
*/
import "C"
import (
	"errors"
	"github.com/pancakeswya/GoSmartCalc/pkg/cconv"
	"github.com/pancakeswya/GoSmartCalc/pkg/dll"
	"unsafe"
)

type CreditCalcFn func(CreditConditions) (CreditData, error)

type (
	CreditConditions struct {
		Sum        float64
		IntRate    float64
		Term       int
		TermType   int
		CreditType int
	}
	CreditData struct {
		Total    float64
		Overpay  float64
		Payments []float64
	}
	CreditCalc struct {
		Calculate CreditCalcFn
	}
)

// credit type
const (
	NumCreditTypeAnnuit = int(C.kCreditTypeAnnuit)
	NumCreditTypeDiff   = int(C.kCreditTypeDiff)
)

// credit term type
const (
	NumCreditTermTypeMonth = int(C.kCreditTermTypeMonth)
	NumCreditTermTypeYear  = int(C.kCreditTermTypeYear)
)

// functions api name
const (
	strCreditCalculateFuncName   = "CreditCalculate"
	strCreditDestroyDataFuncName = "CreditDestroyData"
)

// errors that may occur
var (
	ErrCreditCalcSuccess   = errors.New("success")
	ErrCreditCalcAllocFail = errors.New("allocation fail")

	errsCreditCalc = [...]error{
		ErrCreditCalcSuccess,
		ErrCreditCalcAllocFail,
	}
)

func NewCredit(dl dll.Dll) (*CreditCalc, error) {
	ptr, err := dl.GetSymbolPtr(strCreditCalculateFuncName)
	if err != nil {
		return nil, err
	}
	creditCalcFnPtr := C.CreditCalcFnPtr(ptr)

	ptr, err = dl.GetSymbolPtr(strCreditDestroyDataFuncName)
	if err != nil {
		return nil, err
	}
	CreditDestroyDataFnPtr := C.CreditDestroyDataFnPtr(ptr)

	bc := &CreditCalc{
		Calculate: func(conds CreditConditions) (CreditData, error) {
			cconds := C.CreditConditions{
				sum:         C.double(conds.Sum),
				int_rate:    C.double(conds.IntRate),
				term:        C.ushort(conds.Term),
				term_type:   C.CreditTermType(conds.TermType),
				credit_type: C.CreditType(conds.CreditType),
			}
			var cdata C.CreditData
			if cerr := C.CallCreditCalcFnPtr(creditCalcFnPtr, &cconds, &cdata); cerr != C.kCreditCalcErrorSuccess {
				return CreditData{}, errsCreditCalc[cerr]
			}
			defer C.CallCreditDestroyDataFnPtr(CreditDestroyDataFnPtr, &cdata)
			return CreditData{
				Total:    float64(cdata.total),
				Overpay:  float64(cdata.overpay),
				Payments: cconv.CDoubleArray2Go(unsafe.Pointer(cdata.payments), uint64(cdata.payments_size)),
			}, nil
		},
	}
	return bc, nil
}
