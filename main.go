package main

import (
	"SmartCalc/pkg/core/basic"
	"SmartCalc/pkg/core/credit"
	"SmartCalc/pkg/core/deposit"
	"SmartCalc/pkg/core/dll"
	"fmt"
)

func main() {
	dl, err := dll.New("cc/build/libcalc_core.so")
	if err != nil {
		fmt.Println(err)
		return
	}
	if err := dl.Open(); err != nil {
		fmt.Println(err)
		return
	}
	defer dl.Close()
	bc, err := basic.NewCalc(dl)
	if err != nil {
		fmt.Println(err)
		return
	}
	res, err := bc.CalculateEquation("x * 5 + x * 2", 5)
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(res)
	cc, err := credit.NewCalc(dl)
	if err != nil {
		fmt.Println(err)
		return
	}
	data := cc.Calculate(credit.Conditions{
		Sum:        1000000,
		IntRate:    5,
		Term:       15,
		TermType:   1,
		CreditType: 1,
	})
	fmt.Println(data)
	dc, err := deposit.NewCalc(dl)
	if err != nil {
		fmt.Println(err)
	}

	ddata := dc.Calculate(deposit.Conditions{
		TermType:     0,
		Term:         120,
		Cap:          0,
		PayFreq:      2,
		TaxRate:      13,
		KeyRate:      8.5,
		Sum:          1000000,
		IntrRate:     13.4,
		NonTakingRem: 0,
		StartDate:    [3]int{2023, 8, 13},
		Fund: []deposit.Transaction{
			{
				Payout: deposit.Payout{
					Date: [3]int{
						2023, 8, 13,
					},
					Sum: 5000,
				},
				Freq: 2,
			},
			{
				Payout: deposit.Payout{
					Date: [3]int{
						2023, 12, 12,
					},
					Sum: 4000,
				},
				Freq: 0,
			},
		},
		Wth: []deposit.Transaction{
			{
				Payout: deposit.Payout{
					Date: [3]int{
						2024, 4, 5,
					},
					Sum: 10000,
				},
				Freq: 1,
			},
		},
	})
	fmt.Println(ddata)
}
