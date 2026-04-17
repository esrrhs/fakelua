#include "jit/tcc_handle.h"
#include "state/state.h"
#include "util/logging.h"
#include <libtcc.h>

namespace fakelua {

TCCHandle::TCCHandle(State *s, const CompileConfig &cfg) {
    const auto &config = s->GetStateConfig();
    tcc_state_ = tcc_new();
    if (!tcc_state_) {
        ThrowFakeluaException("tcc_new() failed to create TCC state");
    }
    if (!cfg.debug_mode) {
        tcc_set_options(tcc_state_, "-O2");
    }
    for (const auto &path: config.tcc_config.include_paths) {
        tcc_add_sysinclude_path(tcc_state_, path.c_str());
    }
    for (const auto &path: config.tcc_config.library_paths) {
        tcc_add_library_path(tcc_state_, path.c_str());
    }
    for (const auto &path: config.tcc_config.libraries) {
        tcc_add_library(tcc_state_, path.c_str());
    }

    tcc_set_output_type(tcc_state_, TCC_OUTPUT_MEMORY);

    // Add symbols that the JIT code might call
    tcc_add_symbol(tcc_state_, "FakeluaAllocTemp", (void *) FakeluaAllocTemp);
    tcc_add_symbol(tcc_state_, "FakeluaThrowError", (void *) FakeluaThrowError);
    tcc_add_symbol(tcc_state_, "FakeluaCallByName", (void *) FakeluaCallByName);
    tcc_define_symbol(tcc_state_, "FAKELUA_JIT_TYPE", std::to_string(static_cast<int>(JIT_TCC)).c_str());
}

TCCHandle::~TCCHandle() {
    if (tcc_state_) {
        tcc_delete(tcc_state_);
        tcc_state_ = nullptr;
    }
}

}// namespace fakelua
