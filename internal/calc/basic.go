package calc

/*
  #define __GO
  #include "cc/core/basic_calc.h"
  #undef __GO

  typedef typeof(&BasicCalculateExpr) BasicCalcExprFnPtr;
  typedef typeof(&BasicCalculateEquation) BasicCalcEquationFnPtr;

  inline double CallBasicCalcExprPtr(BasicCalcExprFnPtr fn_ptr, const char* expr, int* err) {
	 return fn_ptr(expr, err);
  }

  inline double CallBasicCalcEquationPtr(BasicCalcEquationFnPtr fn_ptr, const char* expr, double x, int* err) {
	 return fn_ptr(expr, x, err);
  }
*/
import "C"
import (
	"errors"
	"github.com/pancakeswya/GoSmartCalc/pkg/dll"
)

var kErrorStr = [C.kBasicCalcErrorsLen]string{
	"success",
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
	var calcExprFnPtr C.BasicCalcExprFnPtr
	var calcEquationFnPtr C.BasicCalcEquationFnPtr
	{
		ptr, err := dl.GetSymbolPtr("BasicCalculateExpr")
		if err != nil {
			return nil, err
		}
		calcExprFnPtr = C.BasicCalcExprFnPtr(ptr)

		ptr, err = dl.GetSymbolPtr("BasicCalculateEquation")
		if err != nil {
			return nil, err
		}
		calcEquationFnPtr = C.BasicCalcEquationFnPtr(ptr)
	}
	bc := &BasicCalc{}
	bc.CalculateExpr = func(expr string) (float64, error) {
		var err C.int
		res := C.CallBasicCalcExprPtr(calcExprFnPtr, C.CString(expr), &err)
		if err != C.kBasicCalcErrorSuccess {
			return 0, newError(err)
		}
		return float64(res), nil
	}
	bc.CalculateEquation = func(expr string, x float64) (float64, error) {
		var err C.int
		res := C.CallBasicCalcEquationPtr(calcEquationFnPtr, C.CString(expr), C.double(x), &err)
		if err != C.kBasicCalcErrorSuccess {
			return 0, newError(err)
		}
		return float64(res), nil
	}
	return bc, nil
}

func newError(errNum C.int) error {
	errStr := kErrorStr[errNum]
	return errors.New(errStr)
}
