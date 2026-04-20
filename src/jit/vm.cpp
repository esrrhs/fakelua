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

    // 最多支持 8 个参数（与下面的 switch 匹配）。
    if (arg_num > 8) {
        ThrowFakeluaException(
                std::format("FakeluaCallByName: too many arguments ({}) for function '{}', max is 8", arg_num, name));
    }

    // 严格校验参数个数：必须与目标函数签名匹配，否则会读取未初始化栈内存（UB）。
    if (arg_num != func.GetArgCount()) {
        ThrowFakeluaException(std::format("FakeluaCallByName: function '{}' expects {} argument(s), got {}",
                                          name, func.GetArgCount(), arg_num));
    }

    CVar arg_arr[8];
    va_list vl;
    va_start(vl, arg_num);
    for (int i = 0; i < arg_num; ++i) {
        arg_arr[i] = va_arg(vl, CVar);
    }
    va_end(vl);

    // 使用与 fakelua.h 中 Call() 相同的可变参数函数指针转换模式。
    // 所有 JIT 编译的函数都接受/返回 CVar，因此 ABI 是统一的。
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
