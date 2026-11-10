#pragma once

#include "compile/compile_common.h"
#include "compile/register_limit.h"
#include "fakelua.h"

namespace fakelua {

class State;

// 解释器后端：复用 compile/ 的 FunctionRegisterUsage 生成字节码，并负责
// 在 C JIT 注册完成之后把函数登记到 VM、执行 __fakelua_init。
class InterpCodegen : public FunctionRegisterUsage {
public:
    explicit InterpCodegen(State *s);

    // 注册 JIT_INTERP 并执行 init。须在 TCC/GCC 注册完成之后调用，且同一实例已 Build。
    void RegisterAndInit();

    // Build + RegisterAndInit（仅解释器单测等便捷路径）。
    void Generate(const ParseResult &pr, const AnalysisResult &ar, const CompileConfig &cfg);
};

}// namespace fakelua
