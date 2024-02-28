package calc

/*
  #include "cc/basic_calc.h"

  typedef typeof(&BasicCalculateExpr) BasicCalcExprFnPtr;
  typedef typeof(&BasicCalculateEquation) BasicCalcEquationFnPtr;

  static inline BasicCalcError CallBasicCalcExprPtr(BasicCalcExprFnPtr fn_ptr, const char* expr, double* res) {
	 return fn_ptr(expr, res);
  }

  static inline BasicCalcError CallBasicCalcEquationPtr(BasicCalcEquationFnPtr fn_ptr, const char* expr, const char* x, double* res) {
	 return fn_ptr(expr, x, res);
  }
*/
import "C"
import (
	"errors"
	"github.com/pancakeswya/GoSmartCalc/pkg/dll"
	"strconv"
)

var kBasicCalcErrs = [C.kBasicCalcErrorsLen]string{
	"success",
	"allocation fail",
	"invalid expression syntax",
	"braces not matching",
	"incorrect number usage",
	"incorrect operator usage",
	"incorrect function usage",
	"invalid equation",
	"invalid expression",
}

type BasicCalcExprFn func(string) (float64, error)
type BasicCalcEquationFn func(string, float64) (float64, error)

type BasicCalc struct {
	CalculateExpr     BasicCalcExprFn
	CalculateEquation BasicCalcEquationFn
}

func NewBasic(dl dll.Dll) (*BasicCalc, error) {
	ptr, err := dl.GetSymbolPtr("BasicCalculateExpr")
	if err != nil {
		return nil, err
	}
	calcExprFnPtr := C.BasicCalcExprFnPtr(ptr)

	ptr, err = dl.GetSymbolPtr("BasicCalculateEquation")
	if err != nil {
		return nil, err
	}
	calcEquationFnPtr := C.BasicCalcEquationFnPtr(ptr)

	bc := &BasicCalc{}
	bc.CalculateExpr = func(expr string) (float64, error) {
		var res C.double
		cerr := C.CallBasicCalcExprPtr(calcExprFnPtr, C.CString(expr), &res)
		if cerr != C.kBasicCalcErrorSuccess {
			return 0, newBasicCalcError(cerr)
		}
		return float64(res), nil
	}
	bc.CalculateEquation = func(expr string, x float64) (float64, error) {
		var res C.double
		xStr := strconv.FormatFloat(x, 'f', 10, 64)
		cerr := C.CallBasicCalcEquationPtr(calcEquationFnPtr, C.CString(expr), C.CString(xStr), &res)
		if cerr != C.kBasicCalcErrorSuccess {
			return 0, newBasicCalcError(cerr)
		}
		return float64(res), nil
	}
	return bc, nil
}

func newBasicCalcError(errNum C.BasicCalcError) error {
	errStr := kBasicCalcErrs[errNum]
	return errors.New(errStr)
}
