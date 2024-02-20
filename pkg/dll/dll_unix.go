//go:build unix

package dll

/*
  #include <dlfcn.h>
*/
import "C"
import (
	"errors"
	"unsafe"
)

type UnixDll struct {
	Dll
	path   string
	handle unsafe.Pointer
}

func newDll(path string) *UnixDll {
	return &UnixDll{path: path}
}

func (dl *UnixDll) GetSymbolPtr(name string) (unsafe.Pointer, error) {
	if ptr := C.dlsym(dl.handle, C.CString(name)); ptr != nil {
		return ptr, nil
	}
	return nil, lastError()
}

func (dl *UnixDll) Open() error {
	dl.handle = C.dlopen(C.CString(dl.path), C.RTLD_LAZY)
	if dl.handle == nil {
		return lastError()
	}
	return nil
}

func (dl *UnixDll) Close() {
	C.dlclose(dl.handle)
	dl.handle = nil
}

func lastError() error {
	err := C.dlerror()
	if err == nil {
		return nil
	}
	errStr := C.GoString(err)
	return errors.New(errStr)
}
