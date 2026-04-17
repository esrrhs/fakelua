#include "jit/gcc_handle.h"

#if !defined(_WIN32)
#include <dlfcn.h>
#endif

namespace fakelua {

GCCHandle::~GCCHandle() {
#if !defined(_WIN32)
    if (dl_handle_) {
        dlclose(dl_handle_);
        dl_handle_ = nullptr;
    }
#endif
}

}// namespace fakelua
