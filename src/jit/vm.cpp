#include "vm.h"
#include "fakelua.h"
#include "jit/jit_error_boundary.h"
#include "state/state.h"
#include "util/common.h"
#include "var/var_closure.h"
#include "var/var_multi.h"
#include "var/var_type.h"
#include <stdarg.h>

namespace fakelua {

thread_local JitErrorBoundary *g_jit_error_boundary __attribute__((tls_model("initial-exec"))) = nullptr;

[[noreturn]] void JumpToJitErrorBoundary(std::string msg) {
    JitErrorBoundary *boundary = g_jit_error_boundary;
    boundary->msg = std::move(msg);
    FAKELUA_LONGJMP(boundary->buf, 1);
}

extern "C" void *FakeluaAlloc(State *state, size_t size, bool is_const) {
    return GuardJitEntry([&] { return state->GetHeap().GetAllocator(is_const).Alloc(size); });
}

extern "C" void FakeluaThrowError(State *state, const char *msg) {
    if (InJitFrame()) {
        // JIT 代码直接调用本函数，中间没有需要析构的 C++ 帧，可以直接跳转
        JumpToJitErrorBoundary(BuildFakeluaErrorMessage(msg));
    }
    ThrowFakeluaException(msg);
}

// JIT 代码（C 编译）不能直接调用 inter::AllocMultiCVar，需要 extern "C" 包装。
extern "C" __attribute__((used)) CVar FakeluaAllocMultiCVar(State *state, int count) {
    return inter::AllocMultiCVar(state, count);
}

extern "C" __attribute__((used)) void FakeluaSetMultiCVarElement(CVar *multi, int idx, CVar val) {
    inter::SetMultiCVarElement(*multi, idx, val);
}

static CVar CallByNameImpl(State *state, int jit_type, const char *name, int arg_num, const CVar *raw_arg_arr);

extern "C" __attribute__((used)) CVar FakeluaCallByName(State *state, int jit_type, const char *name, int arg_num, ...) {
    // 参数必须在 varargs 函数自身里取出，随后的分发交给普通函数处理。
    // arg_num 超限时一个都不能读：调用方并没有真的压入这么多参数，照着读会越过实参区
    // （Windows 上直接段错误）。越限的报错留给 CallByNameImpl，它在碰 raw_arg_arr
    // 之前就会抛出。
    CVar raw_arg_arr[kMaxFunctionInputParams];
    if (LIKELY(arg_num > 0 && arg_num <= static_cast<int>(kMaxFunctionInputParams))) {
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
    }

    // 调用方可能是 JIT 代码，异常不能穿过它的帧回到 C++
    return GuardJitEntry([&] { return CallByNameImpl(state, jit_type, name, arg_num, raw_arg_arr); });
}

static CVar CallByNameImpl(State *state, int jit_type, const char *name, int arg_num, const CVar *raw_arg_arr) {
    // ── 查找函数：优先 JIT，其次 C++ 原生 ─────────────────────────────────────
    // 用 string_view 查表，避免每次调用都堆分配 std::string
    const std::string_view func_name(name);
    const auto jit_func = state->GetVM().GetFunction(func_name);
    const bool has_jit = !jit_func.Empty();

    // 确定 is_vararg / expected_arg_count / fixed_arg_count
    bool is_vararg = false;
    int expected_arg_count = 0;
    int fixed_arg_count = 0;
    const NativeFuncEntry *native_entry = nullptr;

    if (has_jit) {
        is_vararg = jit_func.IsVararg();
        expected_arg_count = jit_func.GetArgCount();
        fixed_arg_count = is_vararg ? std::max(0, expected_arg_count - 1) : expected_arg_count;
    } else {
        native_entry = state->GetVM().FindNativeFunction(func_name);
        if (UNLIKELY(!native_entry)) {
            ThrowFakeluaException(std::format("FakeluaCallByName: function '{}' not found", name));
        }
        is_vararg = native_entry->is_vararg;
        expected_arg_count = native_entry->arg_count;
        fixed_arg_count = is_vararg ? std::max(0, expected_arg_count - 1) : expected_arg_count;
    }

    if (UNLIKELY(arg_num > static_cast<int>(kMaxFunctionInputParams))) {
        ThrowFakeluaException(std::format("FakeluaCallByName: too many arguments ({}) passed for function '{}', max is {}", arg_num, name, kMaxFunctionInputParams));
    }

    // ── 展开 Multi / 补齐参数 ─────────────────────────────────────────────────
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
                flat_args_buf[flat_count++] = m->GetCount() > 0 ? m->GetVars()[0] : (CVar) {static_cast<int>(VarType::Nil)};
            } else {
                flat_args_buf[flat_count++] = raw_arg_arr[i];
            }
        }

        if (UNLIKELY(is_vararg)) {
            for (int i = 0; i < fixed_arg_count; ++i) {
                temp_arg_arr[i] = i < flat_count ? flat_args_buf[i] : (CVar) {static_cast<int>(VarType::Nil)};
            }
            const int vararg_count = std::max(0, flat_count - fixed_arg_count);
            VarMulti *m = VarMulti::AllocTemp(state, vararg_count);
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
                temp_arg_arr[i] = i < flat_count ? flat_args_buf[i] : (CVar) {static_cast<int>(VarType::Nil)};
            }
        }
        arg_arr = temp_arg_arr;
    }

    // ── 分发 ─────────────────────────────────────────────────────────────────
    // 情形 1：C++ 原生函数
    if (!has_jit) {
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
                flat_args_buf[flat_count++] = m->GetCount() > 0 ? m->GetVars()[0] : (CVar) {static_cast<int>(VarType::Nil)};
            } else {
                flat_args_buf[flat_count++] = raw_arg_arr[i];
            }
        }
        return native_entry->callback(state, flat_args_buf, flat_count);
    }

    // 情形 2：JIT 编译的 lua 函数
    void *addr = jit_func.GetAddr(static_cast<JITType>(jit_type));
    if (UNLIKELY(!addr)) {
        ThrowFakeluaException(std::format("FakeluaCallByName: function '{}' has no address for jit_type {}", name, jit_type));
    }

    // 走 DispatchCall 而不是在这里再展开一遍调用阶梯：它自带 JIT 错误边界，
    // 被调用方出错时本函数的帧（持有 VmFunction 里的 shared_ptr）才能正常析构。
    return inter::DispatchCall(addr, arg_arr, expected_arg_count, static_cast<JITType>(jit_type));
}


}// namespace fakelua
