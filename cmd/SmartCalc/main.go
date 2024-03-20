package main

import (
	"fmt"
	"github.com/pancakeswya/GoSmartCalc/internal/calc"
	"github.com/pancakeswya/GoSmartCalc/pkg/dll"
)

func main() {
	dl, err := dll.New("internal/calc/cc/build/Debug/calc.dll")
	if err != nil {
		fmt.Println(err)
		return
	}
	if err := dl.Open(); err != nil {
		fmt.Println(err)
		return
	}
	defer dl.Close()
	bc, err := calc.NewBasic(dl)
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
	cc, err := calc.NewCredit(dl)
	if err != nil {
		fmt.Println(err)
		return
	}
	data, err := cc.Calculate(calc.CreditConditions{
		Sum:        1000000,
		IntRate:    5,
		Term:       15,
		TermType:   1,
		CreditType: 1,
	})
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(data)
	dc, err := calc.NewDeposit(dl)
	if err != nil {
		fmt.Println(err)
	}

	ddata, err := dc.Calculate(calc.DepositConditions{
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
		Fund: []calc.DepositTransaction{
			{
				Payout: calc.DepositPayout{
					Date: [3]int{
						2023, 8, 13,
					},
					Sum: 5000,
				},
				Freq: 2,
			},
			{
				Payout: calc.DepositPayout{
					Date: [3]int{
						2023, 12, 12,
					},
					Sum: 4000,
				},
				Freq: 0,
			},
		},
		Wth: []calc.DepositTransaction{
			{
				Payout: calc.DepositPayout{
					Date: [3]int{
						2024, 4, 5,
					},
					Sum: 10000,
				},
				Freq: 1,
			},
		},
	})
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(ddata)
}
