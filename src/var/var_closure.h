#pragma once

#include "fakelua.h"

namespace fakelua {

// VarClosure 完整类型定义 —— 仅供 C++ 内部及 .cpp 文件使用
// fakelua.h 中仅保留 class VarClosure; 前置声明

class VarClosure {
public:
    void *func_ptr;
    int upvalue_count;
    int expected_arg_count;
    bool is_vararg;
    CVar *upvalues[0];
};

}// namespace fakelua
