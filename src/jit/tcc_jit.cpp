#include "jit/tcc_jit.h"
#include "jit/tcc_handle.h"
#include "state/state.h"
#include "util/logging.h"

namespace fakelua {

TccJitter::TccJitter(State *s) : s_(s) {
}

void TccJitter::Compile(const ParseResult &pr, const GenResult &gr, const CompileConfig &cfg) {
    const auto handle = std::make_shared<TCCHandle>(s_, cfg);
    ::TCCState *s = handle->GetTCCState();

    if (tcc_compile_string(s, gr.c_code.c_str()) == -1) {
        ThrowFakeluaException(std::format("TCC compile failed, tcc_compile_string failed for {}", pr.file_name));
    }

    if (tcc_relocate(s) == -1) {
        ThrowFakeluaException(std::format("TCC compile failed, tcc_relocate failed for {}", pr.file_name));
    }

    for (const auto &[name, info]: gr.function_names) {
        void *func_ptr = tcc_get_symbol(s, name.c_str());
        if (!func_ptr) {
            ThrowFakeluaException(std::format("TCC compile failed, tcc_get_symbol failed for symbol {} in {}", name, pr.file_name));
        }
        // 生命周期绑定：func_ptr 指向 TCCState 内部的代码页，一旦 TCCState 销毁即失效。
        // 这里把 handle (shared_ptr<TCCHandle>) 一并交给 VmFunction 持有，VmFunction 又
        // 被存放到 State 的 Vm 中（见 src/jit/vm_function.h 的 handle_ 成员）。
        // 因此只要 VmFunction 还活着，func_ptr 就保证可用；State 析构才会一并释放。
        s_->GetVM().RegisterFunction(VmFunction(name, info.params_count, JIT_TCC, func_ptr, handle, info.is_vararg));
        LOG_INFO("Registered function {} with {} params (vararg: {}) at address {}", name, info.params_count, info.is_vararg, func_ptr);
    }

    void *init_ptr = tcc_get_symbol(s, kInitFunctionName);
    if (init_ptr) {
        inter::DispatchCall(init_ptr, nullptr, 0);
    }

    LOG_INFO("TCC JIT compilation finished for {}", pr.file_name);
}

}// namespace fakelua
