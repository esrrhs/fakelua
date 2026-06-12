#include "vm.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
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

    // 最多支持 32 个参数（与下面的 switch 匹配）。
    if (UNLIKELY(arg_num > 32)) {
        ThrowFakeluaException(std::format("FakeluaCallByName: too many arguments ({}) for function '{}', max is 32", arg_num, name));
    }

    // 严格校验参数个数：必须与目标函数签名匹配，否则会读取未初始化栈内存（UB）。
    if (UNLIKELY(arg_num != func.GetArgCount())) {
        ThrowFakeluaException(
                std::format("FakeluaCallByName: function '{}' expects {} argument(s), got {}", name, func.GetArgCount(), arg_num));
    }

    CVar arg_arr[32];
    va_list args_list;
    va_start(args_list, arg_num);
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-pod-varargs"
#endif
    for (int i = 0; i < arg_num; ++i) {
        // NOLINTNEXTLINE(clang-analyzer-valist.Uninitialized)
        arg_arr[i] = va_arg(args_list, CVar);
    }
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
    va_end(args_list);

    switch (arg_num) {
#define VM_CASE(N, Types, Args) \
        case N: \
            return reinterpret_cast<CVar (*) Types>(addr) Args;

        VM_CASE(0, (), ())
        VM_CASE(1, (CVar), (arg_arr[0]))
        VM_CASE(2, (CVar, CVar), (arg_arr[0], arg_arr[1]))
        VM_CASE(3, (CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2]))
        VM_CASE(4, (CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3]))
        VM_CASE(5, (CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4]))
        VM_CASE(6, (CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5]))
        VM_CASE(7, (CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6]))
        VM_CASE(8, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7]))
        VM_CASE(9, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8]))
        VM_CASE(10, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9]))
        VM_CASE(11, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10]))
        VM_CASE(12, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11]))
        VM_CASE(13, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12]))
        VM_CASE(14, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13]))
        VM_CASE(15, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14]))
        VM_CASE(16, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15]))
        VM_CASE(17, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16]))
        VM_CASE(18, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17]))
        VM_CASE(19, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18]))
        VM_CASE(20, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19]))
        VM_CASE(21, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20]))
        VM_CASE(22, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21]))
        VM_CASE(23, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22]))
        VM_CASE(24, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23]))
        VM_CASE(25, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24]))
        VM_CASE(26, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24], arg_arr[25]))
        VM_CASE(27, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24], arg_arr[25], arg_arr[26]))
        VM_CASE(28, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24], arg_arr[25], arg_arr[26], arg_arr[27]))
        VM_CASE(29, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24], arg_arr[25], arg_arr[26], arg_arr[27], arg_arr[28]))
        VM_CASE(30, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24], arg_arr[25], arg_arr[26], arg_arr[27], arg_arr[28], arg_arr[29]))
        VM_CASE(31, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24], arg_arr[25], arg_arr[26], arg_arr[27], arg_arr[28], arg_arr[29], arg_arr[30]))
        VM_CASE(32, (CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar), (arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24], arg_arr[25], arg_arr[26], arg_arr[27], arg_arr[28], arg_arr[29], arg_arr[30], arg_arr[31]))
#undef VM_CASE
        default:
            __builtin_unreachable();
    }
}

}// namespace fakelua
