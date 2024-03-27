package creditcalc

/*
  #include "../cc/credit_calc.h"

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

type CalcFn func(Conditions) (Data, error)

type (
	Conditions struct {
		Sum        float64
		IntRate    float64
		Term       int
		TermType   int
		CreditType int
	}
	Data struct {
		Total    float64
		Overpay  float64
		Payments []float64
	}
	Calc struct {
		Calculate CalcFn
	}
)

// credit type
const (
	TypeAnnuit = int(C.kCreditTypeAnnuit)
	TypeDiff   = int(C.kCreditTypeDiff)
)

// credit term type
const (
	TermTypeMonth = int(C.kCreditTermTypeMonth)
	TermTypeYear  = int(C.kCreditTermTypeYear)
)

// functions api name
const (
	calculateFuncName   = "CreditCalculate"
	destroyDataFuncName = "CreditDestroyData"
)

// errors that may occur
var (
	ErrSuccess   = errors.New("success")
	ErrAllocFail = errors.New("allocation fail")

	errsCreditCalc = [...]error{
		ErrSuccess,
		ErrAllocFail,
	}
)

func New(dl dll.Dll) (*Calc, error) {
	ptr, err := dl.GetSymbolPtr(calculateFuncName)
	if err != nil {
		return nil, err
	}
	creditCalcFnPtr := C.CreditCalcFnPtr(ptr)

	ptr, err = dl.GetSymbolPtr(destroyDataFuncName)
	if err != nil {
		return nil, err
	}
	CreditDestroyDataFnPtr := C.CreditDestroyDataFnPtr(ptr)

	bc := &Calc{
		Calculate: func(conds Conditions) (Data, error) {
			cconds := C.CreditConditions{
				sum:         C.double(conds.Sum),
				int_rate:    C.double(conds.IntRate),
				term:        C.ushort(conds.Term),
				term_type:   C.CreditTermType(conds.TermType),
				credit_type: C.CreditType(conds.CreditType),
			}
			var cdata C.CreditData
			if cerr := C.CallCreditCalcFnPtr(creditCalcFnPtr, &cconds, &cdata); cerr != C.kCreditCalcErrorSuccess {
				return Data{}, errsCreditCalc[cerr]
			}
			defer C.CallCreditDestroyDataFnPtr(CreditDestroyDataFnPtr, &cdata)
			return Data{
				Total:    float64(cdata.total),
				Overpay:  float64(cdata.overpay),
				Payments: cconv.CDoubleArray2Go(unsafe.Pointer(cdata.payments), uint64(cdata.payments_size)),
			}, nil
		},
	}
	return bc, nil
}
