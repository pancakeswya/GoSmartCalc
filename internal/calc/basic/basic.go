package basiccalc

/*
  #include "../cc/basic_calc.h"

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
	CalcExprFn     func(string) (float64, error)
	CalcEquationFn func(string, float64) (float64, error)
)

type Calc struct {
	CalculateExpr     CalcExprFn
	CalculateEquation CalcEquationFn
}

const (
	calculateExprFuncName     = "BasicCalculateExpr"
	calculateEquationFuncName = "BasicCalculateEquation"
)

var (
	ErrSuccess                = errors.New("success")
	ErrAllocFail              = errors.New("allocation fail")
	ErrInvalidExpr            = errors.New("invalid expression syntax")
	ErrBracesNotMatch         = errors.New("braces not matching")
	ErrIncorrectNumberUsage   = errors.New("incorrect number usage")
	ErrIncorrectOperatorUsage = errors.New("incorrect operator usage")
	ErrIncorrectFunctionUsage = errors.New("incorrect function usage")
	ErrInvalidEquation        = errors.New("invalid equation")
	ErrInvalidExpression      = errors.New("invalid expression")

	errBasicCalcErrs = [...]error{
		ErrSuccess,
		ErrAllocFail,
		ErrInvalidExpr,
		ErrBracesNotMatch,
		ErrIncorrectNumberUsage,
		ErrIncorrectOperatorUsage,
		ErrIncorrectFunctionUsage,
		ErrInvalidEquation,
		ErrInvalidExpression,
	}
)

func New(dl dll.Dll) (*Calc, error) {
	ptr, err := dl.GetSymbolPtr(calculateExprFuncName)
	if err != nil {
		return nil, err
	}
	calcExprFnPtr := C.BasicCalcExprFnPtr(ptr)

	ptr, err = dl.GetSymbolPtr(calculateEquationFuncName)
	if err != nil {
		return nil, err
	}
	calcEquationFnPtr := C.BasicCalcEquationFnPtr(ptr)

	bc := &Calc{}
	bc.CalculateExpr = func(expr string) (float64, error) {
		var res C.double
		errCode := C.CallBasicCalcExprPtr(calcExprFnPtr, C.CString(expr), &res)
		if errCode != C.kBasicCalcErrorSuccess {
			return 0, errBasicCalcErrs[errCode]
		}
		return float64(res), nil
	}
	bc.CalculateEquation = func(expr string, x float64) (float64, error) {
		var res C.double
		xStr := strconv.FormatFloat(x, 'f', 10, 64)
		errCode := C.CallBasicCalcEquationPtr(calcEquationFnPtr, C.CString(expr), C.CString(xStr), &res)
		if errCode != C.kBasicCalcErrorSuccess {
			return 0, errBasicCalcErrs[errCode]
		}
		return float64(res), nil
	}
	return bc, nil
}
