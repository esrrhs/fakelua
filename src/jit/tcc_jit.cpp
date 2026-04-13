#include "jit/tcc_jit.h"
#include "state/state.h"
#include "util/logging.h"

namespace fakelua {

TccJitter::TccJitter(State *s) : s_(s) {
}

void TccJitter::Compile(CompileResult &cr, const CompileConfig &cfg) {
    const auto handle = std::make_shared<TCCHandle>(s_);
    ::TCCState *s = handle->GetTCCState();

    if (tcc_compile_string(s, cr.c_code.c_str()) == -1) {
        ThrowFakeluaException(std::format("TCC compile failed, tcc_compile_string failed for {}", cr.file_name));
    }

    if (tcc_relocate(s, TCC_RELOCATE_AUTO) == -1) {
        ThrowFakeluaException(std::format("TCC compile failed, tcc_relocate failed for {}", cr.file_name));
    }

    for (const auto &[name, params_count]: cr.function_names) {
        void *func_ptr = tcc_get_symbol(s, name.c_str());
        if (!func_ptr) {
            ThrowFakeluaException(std::format("TCC compile failed, tcc_get_symbol failed for symbol {} in {}", name, cr.file_name));
        }
        s_->GetVM().RegisterFunction(VmFunction(name, params_count, func_ptr, handle));
        LOG_INFO("Registered function {} with {} params at address {}", name, params_count, func_ptr);
    }

    LOG_INFO("TCC JIT compilation finished for {}", cr.file_name);
}

}// namespace fakelua
