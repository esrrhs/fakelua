#pragma once

namespace fakelua {

// 每函数寄存器槽上限：与 JIT_INTERP 的 Inst::a/b/c（uint8_t，下标 0..255）对齐，
// 作为语言/编译期约束对所有后端生效。
// 计量的是 FunctionRegisterUsage 高峰并发槽（形参 + AllocLocal，含预处理插入的
// __fakelua_tmp_* / __fakelua_lhs_* + AllocReg 表达式临时），不是用户 local 声明数，
// 也不是 C 后端临时变量数。常量池走 sbx(int32)，不受此上限约束。
inline constexpr int kMaxFunctionRegisterIndex = 255;
inline constexpr int kMaxFunctionRegisters = kMaxFunctionRegisterIndex + 1;

}// namespace fakelua
