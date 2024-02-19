package deposit

/*
  #cgo CFLAGS: -I../../..
  #include <dlfcn.h>
  #include <stdlib.h>

  #define __GO
  #include "cc/core/deposit_calc.h"
  #undef __GO

  typedef typeof(&CalculateDeposit) CalcDepositFnPtr;

  inline DepositData CallDepositFnPtr(CalcDepositFnPtr fn_ptr, DepositConditions conds) {
		return fn_ptr(conds);
  }
*/
import "C"
import (
	"SmartCalc/pkg/core/dll"
	"SmartCalc/pkg/core/util"
	"slices"
	"unsafe"
)

const (
	PayFreqEvDay      = int(C.kDepositPayFreqEvDay)
	PayFreqEvWeek     = int(C.kDepositPayFreqEvWeek)
	PayFreqEvMon      = int(C.kDepositPayFreqEvMon)
	PayFreqEvQuart    = int(C.kDepositPayFreqEvQuart)
	PayFreqEvHalfYear = int(C.kDepositPayFreqEvHalfYear)
	PayFreqEvYear     = int(C.kDepositPayFreqEvYear)
)

const (
	TermTypeDay   = int(C.kDepositTermTypeDay)
	TermTypeMonth = int(C.kDepositTermTypeMonth)
	TermTypeYear  = int(C.kDepositTermTypeYear)
)

type Payout struct {
	Date [3]int
	Sum  float64
}

type Transaction struct {
	Payout Payout
	Freq   int
}

type Data struct {
	Replen     []Payout
	PayDates   [][3]int
	Payment    []float64
	Tax        []float64
	StartDate  [3]int
	FinishDate [3]int
	EffRate    float64
	PercSum    float64
	TaxSum     float64
	Total      float64
}

type Conditions struct {
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

type CalcFn func(Conditions) Data

type Calc struct {
	Calculate CalcFn
}

func NewCalc(dl dll.Dll) (*Calc, error) {
	calcDepositFnPtr := (C.CalcDepositFnPtr)(dl.GetSymbolPtr("CalculateDeposit"))
	if calcDepositFnPtr == nil {
		return nil, dl.Error()
	}
	dc := &Calc{
		Calculate: func(conds Conditions) Data {
			cconds := C.DepositConditions{
				term_type: C.short(conds.TermType),
				term:      C.short(conds.Term),
				cap:       C.int(conds.Cap),
				pay_freq:  C.int(conds.PayFreq),

				tax_rate:       C.double(conds.TaxRate),
				key_rate:       C.double(conds.KeyRate),
				sum:            C.double(conds.Sum),
				intr_rate:      C.double(conds.IntrRate),
				non_taking_rem: C.double(conds.NonTakingRem),

				start_date: [3]C.int{
					C.int(conds.StartDate[0]),
					C.int(conds.StartDate[1]),
					C.int(conds.StartDate[2])},

				fund:      goTransaction2C(conds.Fund),
				fund_size: C.ulong(len(conds.Fund)),

				wth:      goTransaction2C(conds.Wth),
				wth_size: C.ulong(len(conds.Wth)),
			}
			data := C.CallDepositFnPtr(calcDepositFnPtr, cconds)
			defer func() {
				C.free(unsafe.Pointer(data.replen))
				C.free(unsafe.Pointer(data.pay_dates))
				C.free(unsafe.Pointer(data.payment))
				C.free(unsafe.Pointer(data.tax))
			}()
			return Data{
				Replen:   cPayout2Go(data.replen, data.replen_size),
				PayDates: cDates2Go(data.pay_dates, data.pay_dates_size),
				Payment:  util.CArray2Go(unsafe.Pointer(data.payment), uint64(data.payment_size)),
				Tax:      util.CArray2Go(unsafe.Pointer(data.tax), uint64(data.tax_size)),
				StartDate: [3]int{
					int(data.start_date[0]),
					int(data.start_date[1]),
					int(data.start_date[2])},
				FinishDate: [3]int{
					int(data.finish_date[0]),
					int(data.finish_date[1]),
					int(data.finish_date[2])},
				EffRate: float64(data.eff_rate),
				PercSum: float64(data.perc_sum),
				TaxSum:  float64(data.tax_sum),
				Total:   float64(data.total),
			}
		},
	}
	return dc, nil
}

func goTransaction2C(goTransactions []Transaction) *C.DepositTransaction {
	transLen := C.ulong(len(goTransactions))
	cTransactions := (*C.DepositTransaction)(C.malloc(transLen * C.ulong(unsafe.Sizeof(C.DepositTransaction{}))))
	transactions := unsafe.Slice(cTransactions, transLen)
	for i := range transactions {
		transactions[i] = C.DepositTransaction{
			payout: C.DepositPayout{
				sum: C.double(goTransactions[i].Payout.Sum),
				date: [3]C.int{
					C.int(goTransactions[i].Payout.Date[0]),
					C.int(goTransactions[i].Payout.Date[1]),
					C.int(goTransactions[i].Payout.Date[2]),
				},
			},
			freq: C.int(goTransactions[i].Freq),
		}
	}
	return cTransactions
}

func cPayout2Go(cPayouts *C.DepositPayout, len C.ulong) []Payout {
	payouts := unsafe.Slice(cPayouts, len)
	slices.Reverse(payouts)
	goPayouts := make([]Payout, len)
	for i, payout := range payouts {
		goPayouts[i] = Payout{
			Date: [3]int{
				int(C.DepositPayout(payout).date[0]),
				int(C.DepositPayout(payout).date[1]),
				int(C.DepositPayout(payout).date[2]),
			},
			Sum: float64(C.DepositPayout(payout).sum),
		}
	}
	return goPayouts
}

func cDates2Go(cDates **C.int, len C.ulong) [][3]int {
	dates := unsafe.Slice(cDates, len)
	goDates := make([][3]int, len)
	for i, date := range dates {
		goDate := unsafe.Slice(date, 3)
		goDates[i] = [3]int{
			int(goDate[0]),
			int(goDate[1]),
			int(goDate[2])}
	}
	return goDates
}
