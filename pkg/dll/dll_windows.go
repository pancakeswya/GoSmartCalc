//go:build windows

package dll

/*
#include <windows.h>
*/
import "C"
import (
	"errors"
	"unsafe"
)

type WindowsDll struct {
	Dll
	path   string
	handle C.HMODULE
}

func newDll(path string) *WindowsDll {
	return &WindowsDll{path: path}
}

func (dl *WindowsDll) GetSymbolPtr(name string) unsafe.Pointer {
	return unsafe.Pointer(C.GetProcAddress(dl.handle, C.CString(name)))
}

func (dl *WindowsDll) Open() error {
	dl.handle = C.LoadLibrary(C.CString(dl.path))
	if dl.handle == nil {
		return dl.Error()
	}
	return nil
}

func (dl *WindowsDll) Close() {
	C.FreeLibrary(dl.handle)
}

func (*WindowsDll) Error() error {
	errStr := ""
	return errors.New(errStr)
}
