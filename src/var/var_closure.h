#pragma once

#include "fakelua.h"
#include <cstdint>

namespace fakelua {

// VarClosure 完整类型定义 —— 仅供 C++ 内部及 .cpp 文件使用
// fakelua.h 中仅保留 class VarClosure; 前置声明

class VarClosure {
public:
    void *func_ptr;
    int upvalue_count;
    int expected_arg_count;
    bool is_vararg;
    const char *code_str;
    CVar *upvalues[0];
};

// code_str 在 func_ptr 非空时借用低地址标记创建该闭包的引擎（1=INTERP，2=GCC，0=TCC/原生）；
// 仅当 func_ptr 为空且指针值 > 2 时才是指向 load() 源码文本的字符串指针。
inline const char *const kGccClosureMagic = reinterpret_cast<const char *>(static_cast<uintptr_t>(2));

inline bool IsGccClosure(const VarClosure *cl) {
    return cl != nullptr && cl->func_ptr != nullptr && cl->code_str == kGccClosureMagic;
}

inline bool IsSourceLoadClosure(const VarClosure *cl) {
    return cl != nullptr && cl->func_ptr == nullptr && reinterpret_cast<uintptr_t>(cl->code_str) > 2;
}

}// namespace fakelua
