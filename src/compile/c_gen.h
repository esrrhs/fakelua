#pragma once

#include "compile/compile_common.h"
#include "compile/func_body_compiler.h"
#include "fakelua.h"

namespace fakelua {

// CGen —— 编译单元级别的 C 代码编排器。
//
// 单一职责：将整个编译单元（ParseResult + InferResult）组织为完整的 C 源文件。
// 具体的语句和表达式编译工作委托给 FuncBodyCompiler。
class CGen {
public:
    explicit CGen(State *s);

    ~CGen() = default;

    GenResult Generate(const ParseResult &pr, const InferResult &ir, const CompileConfig &cfg);

private:
    GenResult Build(const ParseResult &pr, const InferResult &ir, const CompileConfig &cfg);

    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr);

    void GenerateHeader();

    void GenerateGlobal(const SyntaxTreeInterfacePtr &chunk);

    void GenerateDecls(const SyntaxTreeInterfacePtr &chunk, GenResult &gr);

    std::string CompileFuncName(const SyntaxTreeInterfacePtr &ptr);

    std::vector<std::string> CompileParList(const SyntaxTreeInterfacePtr &parlist);

    void GenerateImpl(const SyntaxTreeInterfacePtr &chunk, GenResult &gr);

    // 将函数体编译并追加到 impls_ 流。
    // spec_bitmask >= 0 → 数值特化模式；< 0 → 普通模式。
    void CompileFuncBody(const std::string &func_name,
                          const std::vector<std::string> &func_params,
                          const SyntaxTreeInterfacePtr &func_block,
                          int spec_bitmask);

    // 为含有数学参数的函数生成入口分发器（CVar 签名）。
    void GenerateEntryDispatcher(const std::string &func_name,
                                  const std::vector<std::string> &func_params,
                                  const std::vector<int> &math_param_indices);

    // 检查 block 的最后一条语句是否为 return。
    [[nodiscard]] static bool BlockEndsWithReturn(const SyntaxTreeInterfacePtr &block);

private:
    State *s_;
    std::string file_name_;

    std::stringstream headers_;
    std::stringstream globals_;
    std::stringstream decls_;
    std::stringstream impls_;

    bool in_global_init_ = false;
    std::unordered_set<std::string> global_const_vars_;
    int tmp_var_counter_ = 0;

    // 当前编译单元中声明的函数名（及参数数量）。
    // 在 GenerateDecls 期间填充，以便 CompileFunctioncall 能够区分
    // 同文件的直接调用和跨文件的 FakeluaCallByName 调用。
    std::unordered_map<std::string, int> local_func_names_;

    // cur_output_ 指向当前目标流（headers_、globals_、decls_、impls_）。
    std::ostream *cur_output_ = nullptr;

    // 来自 TypeInferencer::DiscoverMathParams 的数学参数分析结果。
    std::unordered_map<std::string, std::vector<int>> math_param_positions_;

    // 指向 CompileResult 中在 Build() 调用期间始终有效的数据。
    const std::unordered_map<std::string, std::vector<EvalTypeSnapshot>> *specialization_snapshots_ = nullptr;
    const std::unordered_map<std::string, std::vector<InferredType>> *specialization_return_types_ = nullptr;
    const EvalTypeSnapshot *main_eval_types_ = nullptr;

    // func_compiler_ 在构造时只接收 State*；SetContext() 在 Build() 中调用。
    // 在调用 Build() 前不应使用任何编译相关方法。
    FuncBodyCompiler func_compiler_;
};

}// namespace fakelua
