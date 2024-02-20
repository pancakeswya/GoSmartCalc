package cconv

import "C"
import (
	"slices"
	"unsafe"
)

func CInt2dArray2Go(cArray2d unsafe.Pointer, rows uint64, cols uint64) [][]int {
	array2d := unsafe.Slice((**C.int)(cArray2d), rows)
	goArray2d := make([][]int, rows)
	for i, array := range array2d {
		goArray2d[i] = make([]int, cols)
		row := unsafe.Slice(array, cols)
		for j := uint64(0); j < cols; j++ {
			goArray2d[i][j] = int(row[j])
		}
	}
	return goArray2d
}

func CDoubleArray2Go(cArray unsafe.Pointer, len uint64) []float64 {
	array := unsafe.Slice((*C.double)(cArray), len)
	slices.Reverse(array)
	goArray := make([]float64, len)
	for i, num := range array {
		goArray[i] = float64(num)
	}
	return goArray
}
