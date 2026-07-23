#include "var/var_closure.h"
#include "vm.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "util/dispatch_macro.h"
#include "var/var_multi.h"
#include "var/var_type.h"
#include <stdarg.h>

namespace fakelua {

extern "C" void *FakeluaAlloc(State *state, size_t size, bool is_const) {
    return state->GetHeap().GetAllocator(is_const).Alloc(size);
}

extern "C" void FakeluaThrowError(State *state, const char *msg) {
    ThrowFakeluaException(msg);
}



extern "C" __attribute__((used)) CVar FakeluaCallByName(State *state, int jit_type, const char *name, int arg_num, ...) {
    const auto func = state->GetVM().GetFunction(std::string(name));
    if (UNLIKELY(func.Empty())) {
        ThrowFakeluaException(std::format("FakeluaCallByName: function '{}' not found", name));
    }
    void *addr = func.GetAddr(static_cast<JITType>(jit_type));
    if (UNLIKELY(!addr)) {
        ThrowFakeluaException(std::format("FakeluaCallByName: function '{}' has no address for jit_type {}", name, jit_type));
    }

    const bool is_vararg = func.IsVararg();
    const int expected_arg_count = func.GetArgCount();
    const int fixed_arg_count = is_vararg ? expected_arg_count - 1 : expected_arg_count;
    if (UNLIKELY(arg_num > static_cast<int>(kMaxFunctionInputParams))) {
        ThrowFakeluaException(std::format("FakeluaCallByName: too many arguments ({}) passed for function '{}', max is {}", arg_num, name, kMaxFunctionInputParams));
    }

    CVar raw_arg_arr[kMaxFunctionInputParams];
    va_list args_list;
    va_start(args_list, arg_num);
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-pod-varargs"
#endif
    for (int i = 0; i < arg_num; ++i) {
        // NOLINTNEXTLINE(clang-analyzer-valist.Uninitialized)
        raw_arg_arr[i] = va_arg(args_list, CVar);
    }
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
    va_end(args_list);

    const CVar *arg_arr = nullptr;
    const bool last_is_multi = (arg_num > 0 && raw_arg_arr[arg_num - 1].type_ == static_cast<int>(VarType::Multi));
    CVar temp_arg_arr[kMaxFunctionInputParams];
    if (LIKELY(!is_vararg && arg_num == expected_arg_count && !last_is_multi)) {
        arg_arr = raw_arg_arr;
    } else {

        // 展开 Multi 参数到 flat_args
        CVar flat_args_buf[kMaxFunctionInputParams];
        int flat_count = 0;
        for (int i = 0; i < arg_num && flat_count < static_cast<int>(kMaxFunctionInputParams); ++i) {
            if (i == arg_num - 1 && raw_arg_arr[i].type_ == static_cast<int>(VarType::Multi)) {
                VarMulti *m = raw_arg_arr[i].data_.m;
                for (uint32_t j = 0; j < m->GetCount() && flat_count < static_cast<int>(kMaxFunctionInputParams); ++j) {
                    flat_args_buf[flat_count++] = m->GetVars()[j];
                }
            } else if (raw_arg_arr[i].type_ == static_cast<int>(VarType::Multi)) {
                VarMulti *m = raw_arg_arr[i].data_.m;
                flat_args_buf[flat_count++] = m->GetCount() > 0 ? m->GetVars()[0] : (CVar){static_cast<int>(VarType::Nil)};
            } else {
                flat_args_buf[flat_count++] = raw_arg_arr[i];
            }
        }

        if (UNLIKELY(is_vararg)) {
            for (int i = 0; i < fixed_arg_count; ++i) {
                temp_arg_arr[i] = i < flat_count ? flat_args_buf[i] : (CVar){static_cast<int>(VarType::Nil)};
            }
            const int vararg_count = flat_count - fixed_arg_count;
            VarMulti *m = VarMulti::AllocTemp(state, vararg_count > 0 ? vararg_count : 0);
            for (int i = 0; i < vararg_count; ++i) {
                m->GetVars()[i] = flat_args_buf[fixed_arg_count + i];
            }
            CVar vararg_cvar;
            vararg_cvar.type_ = static_cast<int>(VarType::Multi);
            vararg_cvar.flag_ = 0;
            vararg_cvar.data_.m = m;
            temp_arg_arr[fixed_arg_count] = vararg_cvar;
        } else {
            if (UNLIKELY(!last_is_multi && flat_count != expected_arg_count)) {
                ThrowFakeluaException(std::format("FakeluaCallByName: function '{}' expects {} argument(s), got {}", name, expected_arg_count, flat_count));
            }
            for (int i = 0; i < expected_arg_count; ++i) {
                temp_arg_arr[i] = i < flat_count ? flat_args_buf[i] : (CVar){static_cast<int>(VarType::Nil)};
            }
        }
        arg_arr = temp_arg_arr;
    }

    switch (expected_arg_count) {
#define VM_CASE(N) case N: return reinterpret_cast<CVar (*)(VarClosure * DISPATCH_CVAR_##N)>(addr)(nullptr DISPATCH_ARG_##N);

        VM_CASE(0)
        VM_CASE(1)
        VM_CASE(2)
        VM_CASE(3)
        VM_CASE(4)
        VM_CASE(5)
        VM_CASE(6)
        VM_CASE(7)
        VM_CASE(8)
        VM_CASE(9)
        VM_CASE(10)
        VM_CASE(11)
        VM_CASE(12)
        VM_CASE(13)
        VM_CASE(14)
        VM_CASE(15)
        VM_CASE(16)
        VM_CASE(17)
        VM_CASE(18)
        VM_CASE(19)
        VM_CASE(20)
        VM_CASE(21)
        VM_CASE(22)
        VM_CASE(23)
        VM_CASE(24)
        VM_CASE(25)
        VM_CASE(26)
        VM_CASE(27)
        VM_CASE(28)
        VM_CASE(29)
        VM_CASE(30)
        VM_CASE(31)
        VM_CASE(32)

#undef VM_CASE
#include "util/dispatch_macro_undef.h"
        default:
            __builtin_unreachable();
    }
}


}// namespace fakelua
