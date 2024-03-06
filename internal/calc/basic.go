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

type (
	BasicCalcExprFn     func(string) (float64, error)
	BasicCalcEquationFn func(string, float64) (float64, error)
)

type BasicCalc struct {
	CalculateExpr     BasicCalcExprFn
	CalculateEquation BasicCalcEquationFn
}

const (
	strCalculateExprFuncName     = "BasicCalculateExpr"
	strCalculateEquationFuncName = "BasicCalculateEquation"
)

var (
	ErrBasicCalcSuccess        = errors.New("success")
	ErrBasicCalcAllocFail      = errors.New("allocation fail")
	ErrBasicCalcInvalidExpr    = errors.New("invalid expression syntax")
	ErrBasicCalcBracesNotMatch = errors.New("braces not matching")
	ErrIncorrectNumberUsage    = errors.New("incorrect number usage")
	ErrIncorrectOperatorUsage  = errors.New("incorrect operator usage")
	ErrIncorrectFunctionUsage  = errors.New("incorrect function usage")
	ErrInvalidEquation         = errors.New("invalid equation")
	ErrInvalidExpression       = errors.New("invalid expression")

	errBasicCalcErrs = [...]error{
		ErrBasicCalcSuccess,
		ErrBasicCalcAllocFail,
		ErrBasicCalcInvalidExpr,
		ErrBasicCalcBracesNotMatch,
		ErrIncorrectNumberUsage,
		ErrIncorrectOperatorUsage,
		ErrIncorrectFunctionUsage,
		ErrInvalidEquation,
		ErrInvalidExpression,
	}
)

func NewBasic(dl dll.Dll) (*BasicCalc, error) {
	ptr, err := dl.GetSymbolPtr(strCalculateExprFuncName)
	if err != nil {
		return nil, err
	}
	calcExprFnPtr := C.BasicCalcExprFnPtr(ptr)

	ptr, err = dl.GetSymbolPtr(strCalculateEquationFuncName)
	if err != nil {
		return nil, err
	}
	calcEquationFnPtr := C.BasicCalcEquationFnPtr(ptr)

	bc := &BasicCalc{}
	bc.CalculateExpr = func(expr string) (float64, error) {
		var res C.double
		errCode := C.CallBasicCalcExprPtr(calcExprFnPtr, C.CString(expr), &res)
		if errCode != C.kBasicCalcErrorSuccess {
			return 0, errBasicCalcErrs[errCode]
		}
		return float64(res), nil
	}
	bc.CalculateEquation = func(expr string, x float64) (float64, error) {
		const (
			runeFloatFmt    = 'f'
			numFloatPrec    = 10
			numFloatBitSize = 64
		)
		var res C.double
		xStr := strconv.FormatFloat(x, runeFloatFmt, numFloatPrec, numFloatBitSize)
		errCode := C.CallBasicCalcEquationPtr(calcEquationFnPtr, C.CString(expr), C.CString(xStr), &res)
		if errCode != C.kBasicCalcErrorSuccess {
			return 0, errBasicCalcErrs[errCode]
		}
		return float64(res), nil
	}
	return bc, nil
}
