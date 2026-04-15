#include "vm.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include <stdarg.h>

namespace fakelua {

extern "C" void *FakeluaAllocTemp(State *s, size_t size) {
    return s->GetHeap().GetTempAllocator().Alloc(size);
}

extern "C" void FakeluaThrowError(State *s, const char *msg) {
    ThrowFakeluaException(msg);
}

extern "C" __attribute__((used)) CVar FakeluaCallByName(State *s, int jit_type, const char *name, int arg_num, ...) {
    const auto func = s->GetVM().GetFunction(std::string(name));
    if (func.Empty()) {
        ThrowFakeluaException(std::format("FakeluaCallByName: function '{}' not found", name));
    }
    void *addr = func.GetAddr(static_cast<JITType>(jit_type));
    if (!addr) {
        ThrowFakeluaException(std::format("FakeluaCallByName: function '{}' has no address for jit_type {}", name, jit_type));
    }

    // Maximum 8 arguments supported (matches the switch below).
    if (arg_num > 8) {
        ThrowFakeluaException(
                std::format("FakeluaCallByName: too many arguments ({}) for function '{}', max is 8", arg_num, name));
    }

    CVar arg_arr[8];
    va_list vl;
    va_start(vl, arg_num);
    for (int i = 0; i < arg_num; ++i) {
        arg_arr[i] = va_arg(vl, CVar);
    }
    va_end(vl);

    // Uses the same variadic function pointer cast pattern as Call() in fakelua.h.
    // All JIT-compiled functions accept/return CVar, so the ABI is uniform.
    auto fn = reinterpret_cast<CVar (*)(...)>(addr);
    switch (arg_num) {
        case 0: return fn();
        case 1: return fn(arg_arr[0]);
        case 2: return fn(arg_arr[0], arg_arr[1]);
        case 3: return fn(arg_arr[0], arg_arr[1], arg_arr[2]);
        case 4: return fn(arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3]);
        case 5: return fn(arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4]);
        case 6: return fn(arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5]);
        case 7: return fn(arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6]);
        case 8: return fn(arg_arr[0], arg_arr[1], arg_arr[2], arg_arr[3], arg_arr[4], arg_arr[5], arg_arr[6], arg_arr[7]);
        default:
            ThrowFakeluaException(
                    std::format("FakeluaCallByName: too many arguments ({}) for function '{}'", arg_num, name));
    }
}

}// namespace fakelua
