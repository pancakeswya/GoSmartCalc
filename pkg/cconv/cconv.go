package cconv

import "C"
import (
	"slices"
	"unsafe"
)

func CDoubleArray2Go(cArray unsafe.Pointer, len uint64) []float64 {
	array := unsafe.Slice((*C.double)(cArray), len)
	slices.Reverse(array)
	goArray := make([]float64, len)
	for i, num := range array {
		goArray[i] = float64(num)
	}
	return goArray
}
