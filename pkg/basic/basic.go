package basic

/*
  #cgo CFLAGS: -I../..

  #define __GO
  #include "cc/core/basic_calc.h"
  #undef __GO

  typedef typeof(&BasicCalculateExpr) CalcExprFnPtr;
  typedef typeof(&BasicCalculateEquation) CalcEquationFnPtr;

  inline double CallCalcExprPtr(CalcExprFnPtr fn_ptr, const char* expr, int* err) {
	 return fn_ptr(expr, err);
  }

  inline double CallCalcEquationPtr(CalcEquationFnPtr fn_ptr, const char* expr, double x, int* err) {
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

type CalcExprFn func(string) (float64, error)
type CalcEquationFn func(string, float64) (float64, error)

type Calc struct {
	CalculateExpr     CalcExprFn
	CalculateEquation CalcEquationFn
}

func NewCalc(dl dll.Dll) (*Calc, error) {
	calcExprFnPtr := (C.CalcExprFnPtr)(dl.GetSymbolPtr("BasicCalculateExpr"))
	if calcExprFnPtr == nil {
		return nil, dl.Error()
	}
	bc := &Calc{}
	bc.CalculateExpr = func(expr string) (float64, error) {
		var err C.int
		res := C.CallCalcExprPtr(calcExprFnPtr, C.CString(expr), &err)
		if err != C.kBasicCalcErrorSuccess {
			return 0, newError(err)
		}
		return float64(res), nil
	}
	calcEquationFnPtr := (C.CalcEquationFnPtr)(dl.GetSymbolPtr("BasicCalculateEquation"))
	if calcEquationFnPtr == nil {
		return nil, dl.Error()
	}
	bc.CalculateEquation = func(expr string, x float64) (float64, error) {
		var err C.int
		res := C.CallCalcEquationPtr(calcEquationFnPtr, C.CString(expr), C.double(x), &err)
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
