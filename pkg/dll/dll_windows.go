//go:build windows

package dll

/*
#include <windows.h>

int MAKE_LANG_ID(int p, int s) {
#ifdef MAKELANGID
    return MAKELANGID(p, s);
#else
	return LANG_SYSTEM_DEFAULT;
#endif
}
*/
import "C"
import (
	"errors"
	"fmt"
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

func (dl *WindowsDll) GetSymbolPtr(name string) (unsafe.Pointer, error) {
	if ptr := unsafe.Pointer(C.GetProcAddress(dl.handle, C.CString(name))); ptr != nil {
		return ptr, nil
	}
	return nil, lastError()
}

func (dl *WindowsDll) Open() error {
	dl.handle = C.LoadLibrary(C.CString(dl.path))
	if dl.handle == nil {
		return lastError()
	}
	return nil
}

func (dl *WindowsDll) Close() {
	C.FreeLibrary(dl.handle)
	dl.handle = nil
}

func lastError() error {
	errCode := C.GetLastError()
	errStr := getErrorStr(uint64(errCode))
	return errors.New(errStr)
}

func getErrorStr(errCode uint64) string {
	var cMsg C.LPSTR
	ret := C.FormatMessage(
		C.FORMAT_MESSAGE_ALLOCATE_BUFFER|
			C.FORMAT_MESSAGE_FROM_SYSTEM|
			C.FORMAT_MESSAGE_IGNORE_INSERTS,
		nil,
		C.DWORD(errCode),
		C.ulong(C.MAKE_LANG_ID(C.LANG_NEUTRAL, C.SUBLANG_DEFAULT)),
		cMsg,
		0, nil)
	if ret == 0 {
		return fmt.Sprintf("Error %X", int(errCode))
	}

	if cMsg == nil {
		return fmt.Sprintf("Error %X", int(errCode))
	}

	goMsg := C.GoString(cMsg)

	return fmt.Sprintf("Error: %X %s", int(errCode), goMsg)
}
