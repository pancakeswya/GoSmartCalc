package dll

import (
	"errors"
	"os"
	"unsafe"
)

type Dll interface {
	GetSymbolPtr(string) (unsafe.Pointer, error)
	Open() error
	Close()
}

func New(path string) (Dll, error) {
	if _, err := os.Stat(path); os.IsNotExist(err) {
		return nil, errors.New("file does not exists")
	}
	return newDll(path), nil
}
