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
        case 0:
            return reinterpret_cast<CVar (*)()>(addr)();
        case 1:
            return reinterpret_cast<CVar (*)(CVar)>(addr)(arg_arr[0]);
        case 2:
            return reinterpret_cast<CVar (*)(CVar, CVar)>(addr)(arg_arr[0], arg_arr[1]);
        case 3:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar)>(addr)(arg_arr[0], arg_arr[1], arg_arr[2]);
        case 4:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar)>(addr)(arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3]);
        case 5:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4]);
        case 6:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5]);
        case 7:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6]);
        case 8:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7]);
        case 9:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8]);
        case 10:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9]);
        case 11:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10]);
        case 12:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11]);
        case 13:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12]);
        case 14:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13]);
        case 15:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14]);
        case 16:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15]);
        case 17:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16]);
        case 18:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17]);
        case 19:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18]);
        case 20:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19]);
        case 21:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20]);
        case 22:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21]);
        case 23:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22]);
        case 24:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23]);
        case 25:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24]);
        case 26:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24], arg_arr[25]);
        case 27:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24], arg_arr[25], arg_arr[26]);
        case 28:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24], arg_arr[25], arg_arr[26], arg_arr[27]);
        case 29:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24], arg_arr[25], arg_arr[26], arg_arr[27], arg_arr[28]);
        case 30:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24], arg_arr[25], arg_arr[26], arg_arr[27], arg_arr[28], arg_arr[29]);
        case 31:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24], arg_arr[25], arg_arr[26], arg_arr[27], arg_arr[28], arg_arr[29], arg_arr[30]);
        case 32:
            return reinterpret_cast<CVar (*)(CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar)>(addr)(
                    arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7], arg_arr[8], arg_arr[9], arg_arr[10], arg_arr[11], arg_arr[12], arg_arr[13], arg_arr[14], arg_arr[15], arg_arr[16], arg_arr[17], arg_arr[18], arg_arr[19], arg_arr[20], arg_arr[21], arg_arr[22], arg_arr[23], arg_arr[24], arg_arr[25], arg_arr[26], arg_arr[27], arg_arr[28], arg_arr[29], arg_arr[30], arg_arr[31]);
        default:
            __builtin_unreachable();
    }
}

}// namespace fakelua
