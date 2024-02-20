package calc

/*
  #include <stdlib.h>

  #define __GO
  #include "cc/core/deposit_calc.h"
  #undef __GO

  typedef typeof(&DepositCalculate) DepositCalcFnPtr;
  typedef typeof(&DepositFreeData) DepositFreeDataFnPtr;

  inline void CallDepositCalcFnPtr(DepositCalcFnPtr fn_ptr, const DepositConditions* conds, DepositData* data) {
		return fn_ptr(conds, data);
  }
  inline void CallDepositFreeDataFnPtr(DepositFreeDataFnPtr fn_ptr, DepositData* data) {
		return fn_ptr(data);
  }
*/
import "C"
import (
	"github.com/pancakeswya/GoSmartCalc/pkg/cconv"
	"github.com/pancakeswya/GoSmartCalc/pkg/dll"
	"slices"
	"unsafe"
)

const (
	DepositPayFreqEvDay      = int(C.kDepositPayFreqEvDay)
	DepositPayFreqEvWeek     = int(C.kDepositPayFreqEvWeek)
	DepositPayFreqEvMon      = int(C.kDepositPayFreqEvMon)
	DepositPayFreqEvQuart    = int(C.kDepositPayFreqEvQuart)
	DepositPayFreqEvHalfYear = int(C.kDepositPayFreqEvHalfYear)
	DepositPayFreqEvYear     = int(C.kDepositPayFreqEvYear)
)

const (
	DepositTermTypeDay   = int(C.kDepositTermTypeDay)
	DepositTermTypeMonth = int(C.kDepositTermTypeMonth)
	DepositTermTypeYear  = int(C.kDepositTermTypeYear)
)

type DepositPayout struct {
	Date [3]int
	Sum  float64
}

type DepositTransaction struct {
	Payout DepositPayout
	Freq   int
}

type DepositData struct {
	Replen     []DepositPayout
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

type DepositConditions struct {
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
	Fund         []DepositTransaction
	Wth          []DepositTransaction
}

type DepositCalcFn func(DepositConditions) DepositData

type DepositCalc struct {
	Calculate DepositCalcFn
}

func NewDeposit(dl dll.Dll) (*DepositCalc, error) {
	var depositCalcFnPtr C.DepositCalcFnPtr
	var depositFreeDataFnPtr C.DepositFreeDataFnPtr
	{
		ptr, err := dl.GetSymbolPtr("DepositCalculate")
		if err != nil {
			return nil, err
		}
		depositCalcFnPtr = C.DepositCalcFnPtr(ptr)

		ptr, err = dl.GetSymbolPtr("DepositFreeData")
		if err != nil {
			return nil, err
		}
		depositFreeDataFnPtr = C.DepositFreeDataFnPtr(ptr)
	}
	dc := &DepositCalc{
		Calculate: func(conds DepositConditions) DepositData {
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
				fund_size: C.size_t(len(conds.Fund)),

				wth:      goTransaction2C(conds.Wth),
				wth_size: C.size_t(len(conds.Wth)),
			}
			var cdata C.DepositData
			C.CallDepositCalcFnPtr(depositCalcFnPtr, &cconds, &cdata)
			defer func() {
				C.free(unsafe.Pointer(cconds.fund))
				C.free(unsafe.Pointer(cconds.wth))
				C.CallDepositFreeDataFnPtr(depositFreeDataFnPtr, &cdata)
			}()
			return DepositData{
				Replen:   cPayout2Go(cdata.replen, cdata.replen_size),
				PayDates: cconv.CInt2dArray2Go(unsafe.Pointer(cdata.pay_dates), uint64(cdata.pay_dates_size), 3),
				Payment:  cconv.CDoubleArray2Go(unsafe.Pointer(cdata.payment), uint64(cdata.payment_size)),
				Tax:      cconv.CDoubleArray2Go(unsafe.Pointer(cdata.tax), uint64(cdata.tax_size)),
				StartDate: [3]int{
					int(cdata.start_date[0]),
					int(cdata.start_date[1]),
					int(cdata.start_date[2])},
				FinishDate: [3]int{
					int(cdata.finish_date[0]),
					int(cdata.finish_date[1]),
					int(cdata.finish_date[2])},
				EffRate: float64(cdata.eff_rate),
				PercSum: float64(cdata.perc_sum),
				TaxSum:  float64(cdata.tax_sum),
				Total:   float64(cdata.total),
			}
		},
	}
	return dc, nil
}

func goTransaction2C(goTransactions []DepositTransaction) *C.DepositTransaction {
	transLen := C.size_t(len(goTransactions))
	cTransactions := (*C.DepositTransaction)(C.malloc(transLen * C.size_t(unsafe.Sizeof(C.DepositTransaction{}))))
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

func cPayout2Go(cPayouts *C.DepositPayout, len C.size_t) []DepositPayout {
	payouts := unsafe.Slice(cPayouts, len)
	slices.Reverse(payouts)
	goPayouts := make([]DepositPayout, len)
	for i, payout := range payouts {
		goPayouts[i] = DepositPayout{
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
