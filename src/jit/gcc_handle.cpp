#include "jit/gcc_handle.h"

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace fakelua {

GCCHandle::~GCCHandle() {
#if defined(_WIN32)
    if (dl_handle_) {
        FreeLibrary(static_cast<HMODULE>(dl_handle_));
        dl_handle_ = nullptr;
    }
#else
    if (dl_handle_) {
        dlclose(dl_handle_);
        dl_handle_ = nullptr;
    }
#endif
}

}// namespace fakelua
