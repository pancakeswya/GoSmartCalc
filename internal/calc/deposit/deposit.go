package depositcalc

/*

  #include "../cc/deposit_calc.h"
  #include "../cc/util/vector.h"

  typedef typeof(&DepositCalculate) DepositCalcFnPtr;
  typedef typeof(&DepositDestroyData) DepositDestroyDataFnPtr;

  static inline DepositCalcError CallDepositCalcFnPtr(DepositCalcFnPtr fn_ptr, DepositConditions* conds, DepositData* data) {
		return fn_ptr(conds, data);
  }
  static inline void CallDepositDestroyDataFnPtr(DepositDestroyDataFnPtr fn_ptr, DepositData* data) {
		return fn_ptr(data);
  }

  static inline DepositTransaction* VectorNewTransactionWrap(void) {
		return VectorNew(DepositTransaction);
  }

  static inline DepositCalcError VectorPushTransWrap(DepositTransaction** transactions, DepositTransaction transaction) {
		if (!VectorPush(*transactions, transaction)) {
			return kDepositCalcErrorAllocationFail;
		}
		return kDepositCalcErrorSuccess;
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
	Payout struct {
		Date [3]int
		Sum  float64
	}
	Transaction struct {
		Payout Payout
		Freq   int
	}
	Data struct {
		Replen     []Payout
		PayDates   [][]int
		Payment    []float64
		Tax        []float64
		StartDate  [3]int
		FinishDate [3]int
		EffRate    float64
		PercSum    float64
		TaxSum     float64
		Total      float64
	}
	Conditions struct {
		TermType     int
		Term         int
		Cap          int
		PayFreq      int
		TaxRate      float64
		KeyRate      float64
		Sum          float64
		IntrRate     float64
		NonTakingRem float64
		StartDate    [3]int
		Fund         []Transaction
		Wth          []Transaction
	}
	Calc struct {
		Calculate CalcFn
	}
)

// functions api name
const (
	calculateFuncName   = "DepositCalculate"
	destroyDataFuncName = "DepositDestroyData"
)

// transaction payout frequency
const (
	TransactionFreqOnce       = int(C.kDepositTransactionFreqOnce)
	TransactionFreqEvMon      = int(C.kDepositTransactionFreqEvMon)
	TransactionFreqEv2Mon     = int(C.kDepositTransactionFreqEv2Mon)
	TransactionFreqQuart      = int(C.kDepositTransactionFreqQuart)
	TransactionFreqEvHalfYear = int(C.kDepositTransactionFreqEvHalfYear)
	TransactionFreqEvYear     = int(C.kDepositTransactionFreqEvYear)
)

// deposit payout frequency
const (
	PayFreqEvDay      = int(C.kDepositPayFreqEvDay)
	PayFreqEvWeek     = int(C.kDepositPayFreqEvWeek)
	PayFreqEvMon      = int(C.kDepositPayFreqEvMon)
	PayFreqEvQuart    = int(C.kDepositPayFreqEvQuart)
	PayFreqEvHalfYear = int(C.kDepositPayFreqEvHalfYear)
	PayFreqEvYear     = int(C.kDepositPayFreqEvYear)
)

// deposit term type
const (
	TermTypeDay   = int(C.kDepositTermTypeDay)
	TermTypeMonth = int(C.kDepositTermTypeMonth)
	TermTypeYear  = int(C.kDepositTermTypeYear)
)

var (
	ErrSuccess   = errors.New("success")
	ErrAllocFail = errors.New("allocation fail")

	errDepositCalcErrs = [...]error{
		ErrSuccess,
		ErrAllocFail,
	}
)

func New(dl dll.Dll) (*Calc, error) {
	ptr, err := dl.GetSymbolPtr(calculateFuncName)
	if err != nil {
		return nil, err
	}
	depositCalcFnPtr := C.DepositCalcFnPtr(ptr)

	ptr, err = dl.GetSymbolPtr(destroyDataFuncName)
	if err != nil {
		return nil, err
	}
	DepositDestroyDataFnPtr := C.DepositDestroyDataFnPtr(ptr)

	dc := &Calc{
		Calculate: func(conds Conditions) (Data, error) {
			cconds := C.DepositConditions{
				term_type: C.DepositTermType(conds.TermType),
				term:      C.ushort(conds.Term),
				capt:      C.int(conds.Cap),
				pay_freq:  C.DepositPayFreq(conds.PayFreq),

				tax_rate:       C.double(conds.TaxRate),
				key_rate:       C.double(conds.KeyRate),
				sum:            C.double(conds.Sum),
				intr_rate:      C.double(conds.IntrRate),
				non_taking_rem: C.double(conds.NonTakingRem),

				start_date: C.DateNew(
					C.int(conds.StartDate[0]),
					C.int(conds.StartDate[1]),
					C.int(conds.StartDate[2])),
			}
			var errCode C.DepositCalcError
			cconds.fund, errCode = goTransaction2C(conds.Fund)
			if errCode != C.kDepositCalcErrorSuccess {
				return Data{}, errDepositCalcErrs[errCode]
			}
			cconds.wth, errCode = goTransaction2C(conds.Wth)
			if errCode != C.kDepositCalcErrorSuccess {
				return Data{}, errDepositCalcErrs[errCode]
			}

			var cdata C.DepositData
			errCode = C.CallDepositCalcFnPtr(depositCalcFnPtr, &cconds, &cdata)
			if errCode != C.kDepositCalcErrorSuccess {
				return Data{}, errDepositCalcErrs[errCode]
			}
			defer func() {
				C.VectorDelete(unsafe.Pointer(cconds.fund))
				C.VectorDelete(unsafe.Pointer(cconds.wth))
				C.CallDepositDestroyDataFnPtr(DepositDestroyDataFnPtr, &cdata)
			}()
			return Data{
				Replen:   cPayout2Go(cdata.replen, C.VectorSize(unsafe.Pointer(cdata.replen))),
				PayDates: cDatesArray2Go(unsafe.Pointer(cdata.pay_dates), C.VectorSize(unsafe.Pointer(cdata.pay_dates))),
				Payment:  cconv.CDoubleArray2Go(unsafe.Pointer(cdata.payments), uint64(C.VectorSize(unsafe.Pointer(cdata.payments)))),
				Tax:      cconv.CDoubleArray2Go(unsafe.Pointer(cdata.taxes), uint64(C.VectorSize(unsafe.Pointer(cdata.taxes)))),
				StartDate: [3]int{
					int(C.DateGetYear(&cdata.start_date)),
					int(C.DateGetMonth(&cdata.start_date)),
					int(C.DateGetDay(&cdata.start_date))},
				FinishDate: [3]int{
					int(C.DateGetYear(&cdata.finish_date)),
					int(C.DateGetMonth(&cdata.finish_date)),
					int(C.DateGetDay(&cdata.finish_date))},
				EffRate: float64(cdata.eff_rate),
				PercSum: float64(cdata.perc_sum),
				TaxSum:  float64(cdata.tax_sum),
				Total:   float64(cdata.total),
			}, nil
		},
	}
	return dc, nil
}

func cDatesArray2Go(cDates unsafe.Pointer, len C.size_t) [][]int {
	dates := unsafe.Slice((*C.Date)(cDates), len)
	goArray2d := make([][]int, len)
	for i := range dates {
		goArray2d[i] = []int{
			int(C.DateGetYear(&dates[i])),
			int(C.DateGetMonth(&dates[i])),
			int(C.DateGetDay(&dates[i])),
		}
	}
	return goArray2d
}

func goTransaction2C(goTransactions []Transaction) (*C.DepositTransaction, C.DepositCalcError) {
	cTransactions := C.VectorNewTransactionWrap()
	if cTransactions == nil {
		return nil, C.kDepositCalcErrorAllocationFail
	}
	for _, goTransaction := range goTransactions {
		cerr := C.VectorPushTransWrap(&cTransactions, C.DepositTransaction{
			payout: C.DepositPayout{
				sum: C.double(goTransaction.Payout.Sum),
				date: C.DateNew(
					C.int(goTransaction.Payout.Date[0]),
					C.int(goTransaction.Payout.Date[1]),
					C.int(goTransaction.Payout.Date[2]),
				),
			},
			freq: C.DepositTransactionFreq(goTransaction.Freq),
		})
		if cerr != C.kDepositCalcErrorSuccess {
			return nil, cerr
		}
	}
	return cTransactions, C.kDepositCalcErrorSuccess
}

func cPayout2Go(cPayouts *C.DepositPayout, len C.size_t) []Payout {
	payouts := unsafe.Slice(cPayouts, len)
	goPayouts := make([]Payout, len)
	for i, payout := range payouts {
		cPayout := C.DepositPayout(payout)
		goPayouts[i] = Payout{
			Date: [3]int{
				int(C.DateGetYear(&cPayout.date)),
				int(C.DateGetMonth(&cPayout.date)),
				int(C.DateGetDay(&cPayout.date)),
			},
			Sum: float64(cPayout.sum),
		}
	}
	return goPayouts
}
