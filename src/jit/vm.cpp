#include "vm.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "var/var_multi.h"
#include "var/var_type.h"
#include <stdarg.h>

namespace fakelua {

extern "C" void *FakeluaAllocTemp(State *state, size_t size) {
    return state->GetHeap().GetTempAllocator().Alloc(size);
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
#define CVAR_0
#define CVAR_1 CVar
#define CVAR_2 CVAR_1, CVar
#define CVAR_3 CVAR_2, CVar
#define CVAR_4 CVAR_3, CVar
#define CVAR_5 CVAR_4, CVar
#define CVAR_6 CVAR_5, CVar
#define CVAR_7 CVAR_6, CVar
#define CVAR_8 CVAR_7, CVar
#define CVAR_9 CVAR_8, CVar
#define CVAR_10 CVAR_9, CVar
#define CVAR_11 CVAR_10, CVar
#define CVAR_12 CVAR_11, CVar
#define CVAR_13 CVAR_12, CVar
#define CVAR_14 CVAR_13, CVar
#define CVAR_15 CVAR_14, CVar
#define CVAR_16 CVAR_15, CVar
#define CVAR_17 CVAR_16, CVar
#define CVAR_18 CVAR_17, CVar
#define CVAR_19 CVAR_18, CVar
#define CVAR_20 CVAR_19, CVar
#define CVAR_21 CVAR_20, CVar
#define CVAR_22 CVAR_21, CVar
#define CVAR_23 CVAR_22, CVar
#define CVAR_24 CVAR_23, CVar
#define CVAR_25 CVAR_24, CVar
#define CVAR_26 CVAR_25, CVar
#define CVAR_27 CVAR_26, CVar
#define CVAR_28 CVAR_27, CVar
#define CVAR_29 CVAR_28, CVar
#define CVAR_30 CVAR_29, CVar
#define CVAR_31 CVAR_30, CVar
#define CVAR_32 CVAR_31, CVar

#define ARG_0
#define ARG_1 arg_arr[0]
#define ARG_2 ARG_1, arg_arr[1]
#define ARG_3 ARG_2, arg_arr[2]
#define ARG_4 ARG_3, arg_arr[3]
#define ARG_5 ARG_4, arg_arr[4]
#define ARG_6 ARG_5, arg_arr[5]
#define ARG_7 ARG_6, arg_arr[6]
#define ARG_8 ARG_7, arg_arr[7]
#define ARG_9 ARG_8, arg_arr[8]
#define ARG_10 ARG_9, arg_arr[9]
#define ARG_11 ARG_10, arg_arr[10]
#define ARG_12 ARG_11, arg_arr[11]
#define ARG_13 ARG_12, arg_arr[12]
#define ARG_14 ARG_13, arg_arr[13]
#define ARG_15 ARG_14, arg_arr[14]
#define ARG_16 ARG_15, arg_arr[15]
#define ARG_17 ARG_16, arg_arr[16]
#define ARG_18 ARG_17, arg_arr[17]
#define ARG_19 ARG_18, arg_arr[18]
#define ARG_20 ARG_19, arg_arr[19]
#define ARG_21 ARG_20, arg_arr[20]
#define ARG_22 ARG_21, arg_arr[21]
#define ARG_23 ARG_22, arg_arr[22]
#define ARG_24 ARG_23, arg_arr[23]
#define ARG_25 ARG_24, arg_arr[24]
#define ARG_26 ARG_25, arg_arr[25]
#define ARG_27 ARG_26, arg_arr[26]
#define ARG_28 ARG_27, arg_arr[27]
#define ARG_29 ARG_28, arg_arr[28]
#define ARG_30 ARG_29, arg_arr[29]
#define ARG_31 ARG_30, arg_arr[30]
#define ARG_32 ARG_31, arg_arr[31]

#define VM_CASE(N)                                                                                                                                                                                     \
    case N:                                                                                                                                                                                            \
        return reinterpret_cast<CVar (*)(CVAR_##N)>(addr)(ARG_##N);

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
#undef ARG_0
#undef ARG_1
#undef ARG_2
#undef ARG_3
#undef ARG_4
#undef ARG_5
#undef ARG_6
#undef ARG_7
#undef ARG_8
#undef ARG_9
#undef ARG_10
#undef ARG_11
#undef ARG_12
#undef ARG_13
#undef ARG_14
#undef ARG_15
#undef ARG_16
#undef ARG_17
#undef ARG_18
#undef ARG_19
#undef ARG_20
#undef ARG_21
#undef ARG_22
#undef ARG_23
#undef ARG_24
#undef ARG_25
#undef ARG_26
#undef ARG_27
#undef ARG_28
#undef ARG_29
#undef ARG_30
#undef ARG_31
#undef ARG_32
#undef CVAR_0
#undef CVAR_1
#undef CVAR_2
#undef CVAR_3
#undef CVAR_4
#undef CVAR_5
#undef CVAR_6
#undef CVAR_7
#undef CVAR_8
#undef CVAR_9
#undef CVAR_10
#undef CVAR_11
#undef CVAR_12
#undef CVAR_13
#undef CVAR_14
#undef CVAR_15
#undef CVAR_16
#undef CVAR_17
#undef CVAR_18
#undef CVAR_19
#undef CVAR_20
#undef CVAR_21
#undef CVAR_22
#undef CVAR_23
#undef CVAR_24
#undef CVAR_25
#undef CVAR_26
#undef CVAR_27
#undef CVAR_28
#undef CVAR_29
#undef CVAR_30
#undef CVAR_31
#undef CVAR_32
        default:
            __builtin_unreachable();
    }
}


}// namespace fakelua
