#include "jit/gcc_handle.h"
#include "state/state.h"

#include <filesystem>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace fakelua {

GCCHandle::~GCCHandle() {
    if (s_ && lua_loc_) {
        s_->UnregisterLuaLoc(JIT_GCC, lua_loc_);
        lua_loc_ = nullptr;
    }
    // Unload the dynamic library first (files can't be deleted while mapped).
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

    // Best-effort cleanup of temporary compilation artifacts.
    std::error_code ec;
#if defined(USE_COV)
    // Keep the emitted .c so lcov can attribute arcs to a real source path.
    // .gcno/.gcda sit beside the .so basename and are intentionally left for harvest.
#else
    if (!c_file_.empty()) {
        std::filesystem::remove(c_file_, ec);
    }
#endif
    if (!so_file_.empty()) {
        std::filesystem::remove(so_file_, ec);
    }
    if (!log_file_.empty()) {
        std::filesystem::remove(log_file_, ec);
    }
}

}// namespace fakelua
