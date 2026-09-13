#pragma once

#include "fakelua.h"
#include "gtest/gtest.h"

namespace fakelua {

// 在所有执行后端上调用。新增后端只需改 JITType 枚举，不必改各个测试文件。
template<typename Ret, typename... Args>
void CallAll(State *s, const std::string_view &name, Ret &&ret, Args &&...args) {
    for (const auto jit_type: AllJitTypes()) {
        SCOPED_TRACE(::testing::Message() << "CallAll jit=" << JitTypeName(jit_type) << " fn=" << name);
        Call(s, jit_type, name, ret, args...);
    }
}

// 需要 C++ 异常穿过执行帧的用例：跳过 TCC。
template<typename Exception = std::exception, typename Ret, typename... Args>
void CallThrow(State *s, const std::string_view &name, Ret &&ret, Args &&...args) {
    for (const auto jit_type: ExceptionJitTypes()) {
        SCOPED_TRACE(::testing::Message() << "CallThrow jit=" << JitTypeName(jit_type) << " fn=" << name);
        EXPECT_THROW(Call(s, jit_type, name, ret, args...), Exception);
    }
}

}// namespace fakelua
