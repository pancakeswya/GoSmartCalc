package dll

/*
  #cgo CFLAGS: -I../../..
  #include <dlfcn.h>
  #include "cc/core/defs.h"
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

func (dl *UnixDll) GetSymbolPtr(name string) unsafe.Pointer {
	return C.dlsym(dl.handle, C.CString(name))
}

func (dl *UnixDll) Open() error {
	dl.handle = C.dlopen(C.CString(dl.path), C.RTLD_LAZY)
	if dl.handle == nil {
		return dl.Error()
	}
	return nil
}

func (dl *UnixDll) Close() {
	C.dlclose(dl.handle)
}

func (*UnixDll) Error() error {
	errStr := C.GoString(C.dlerror())
	return errors.New(errStr)
}
