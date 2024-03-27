package main

import (
	"fmt"
	"github.com/pancakeswya/GoSmartCalc/internal/calc/basic"
	"github.com/pancakeswya/GoSmartCalc/internal/calc/credit"
	"github.com/pancakeswya/GoSmartCalc/internal/calc/deposit"
	"github.com/pancakeswya/GoSmartCalc/pkg/dll"
)

func main() {
	dl, err := dll.New("internal/calc/cc/build/libcalc.so")
	if err != nil {
		fmt.Println(err)
		return
	}
	if err := dl.Open(); err != nil {
		fmt.Println(err)
		return
	}
	defer dl.Close()
	bc, err := basiccalc.New(dl)
	if err != nil {
		fmt.Println(err)
		return
	}
	res, err := bc.CalculateExpr("15/(7-(1+1))*3-(2+(1+1))*15/(7-(200+1))*3-(2+(1+1))*(15/(7-(1+1))*3-(2+(1+1))+15/(7-(1+1))*3-(2+(1+1)))")
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(res)
	cc, err := creditcalc.New(dl)
	if err != nil {
		fmt.Println(err)
		return
	}
	data, err := cc.Calculate(creditcalc.Conditions{
		Sum:        1000000,
		IntRate:    5,
		Term:       15,
		TermType:   creditcalc.TermTypeYear,
		CreditType: creditcalc.TypeDiff,
	})
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(data)
	dc, err := depositcalc.New(dl)
	if err != nil {
		fmt.Println(err)
	}

	ddata, err := dc.Calculate(depositcalc.Conditions{
		TermType:     depositcalc.TermTypeMonth,
		Term:         120,
		Cap:          1,
		PayFreq:      depositcalc.PayFreqEvDay,
		TaxRate:      13,
		KeyRate:      16,
		Sum:          1000000,
		IntrRate:     13.4,
		NonTakingRem: 0,
		StartDate:    [3]int{2024, 8, 13},
		Fund: []depositcalc.Transaction{
			{
				Payout: depositcalc.Payout{
					Date: [3]int{
						2024, 8, 13,
					},
					Sum: 5000,
				},
				Freq: depositcalc.TransactionFreqEvMon,
			},
			{
				Payout: depositcalc.Payout{
					Date: [3]int{
						2025, 4, 5,
					},
					Sum: 10000,
				},
				Freq: depositcalc.TransactionFreqEvMon,
			},
		},
		Wth: []depositcalc.Transaction{
			{
				Payout: depositcalc.Payout{
					Date: [3]int{
						2024, 12, 12,
					},
					Sum: 4000,
				},
				Freq: depositcalc.TransactionFreqOnce,
			},
		},
	})
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(ddata)
}
