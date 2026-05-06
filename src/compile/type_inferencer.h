#pragma once

#include "compile/compile_common.h"

namespace fakelua {

class TypeEnvironment {
public:
    TypeEnvironment();

    void EnterScope();

    void ExitScope();

    void Define(const std::string &name, InferredType type);

    bool Update(const std::string &name, InferredType type);

    [[nodiscard]] InferredType Lookup(const std::string &name) const;

private:
    static InferredType MergeType(InferredType old_type, InferredType new_type);

private:
    std::vector<std::unordered_map<std::string, InferredType>> scopes_;
};

class TypeInferencer {
public:
    // 运行全局类型推断，并在 cr.math_param_positions 中填充数学参数特化信息。
    void Process(CompileResult &cr);

private:
    // node指针 → 推断类型的快照映射，用于特化发现的不动点迭代。
    using EvalTypeMap = EvalTypeSnapshot;

    InferredType InferAndSetEvalType(const SyntaxTreeInterfacePtr &node);

    InferredType InferExp(const std::shared_ptr<SyntaxTreeExp> &exp);

    InferredType InferPrefixExp(const std::shared_ptr<SyntaxTreePrefixexp> &prefix_exp);

    InferredType InferVar(const std::shared_ptr<SyntaxTreeVar> &var);

    void InferBlock(const std::shared_ptr<SyntaxTreeBlock> &block, bool new_scope);

    // -----------------------------------------------------------------------
    // 数学参数特化发现（迭代不动点推断）
    // -----------------------------------------------------------------------

    // 遍历顶层函数，识别数学参数并写入 cr.math_param_positions。
    void DiscoverMathParams(CompileResult &cr);

    // 以 assumed_types 中给定的参数类型假设运行 InferBlock，迭代直到稳定（不动点），
    // 返回各 AST 节点 → InferredType 的快照。调用结束后 EvalType 处于最后一次推断的状态。
    EvalTypeMap RunTrialInference(const SyntaxTreeInterfacePtr &func_block,
                                  const std::vector<std::string> &params,
                                  const std::unordered_map<std::string, InferredType> &assumed_types);

    // 判断 exp 节点是否为算术二元运算（结果可为 T_INT/T_FLOAT 的运算符）。
    bool IsArithmeticBinop(const SyntaxTreeInterfacePtr &node) const;

    // 判断 all_int（全参数=T_INT）相对于 baseline（全参数=T_DYNAMIC）是否有算术表达式改善。
    bool HasArithmeticImprovement(const EvalTypeMap &all_int, const EvalTypeMap &baseline,
                                  const SyntaxTreeInterfacePtr &func_block) const;

    // 判断 all_int 中某个参数被置回 T_DYNAMIC 后（without_p），是否有算术表达式退化。
    bool ParamAffectsArithmetic(const EvalTypeMap &all_int, const EvalTypeMap &without_p,
                                const SyntaxTreeInterfacePtr &func_block) const;

    // 检查 block_node 的所有执行路径是否均以 return 语句结束。
    // 能识别 if-else（所有分支均返回）的情况，不递归进入嵌套函数体。
    [[nodiscard]] bool AllPathsReturn(const SyntaxTreeInterfacePtr &block_node) const;

    // 从 block_node 中浅层收集每条 return 语句的第一个返回表达式。
    // 返回 true 表示所有路径均以 return 结束（无隐式 nil 返回路径）。
    bool CollectReturnExps(const SyntaxTreeInterfacePtr &block_node,
                           std::vector<SyntaxTreeInterfacePtr> &ret_exps) const;

    // 在特化上下文和假定返回类型表下，递归计算返回表达式的类型。
    [[nodiscard]] InferredType EvalReturnExpType(
            const SyntaxTreeInterfacePtr &exp,
            const EvalTypeSnapshot &snapshot,
            const std::unordered_map<std::string, InferredType> &spec_ctx,
            const std::unordered_map<std::string, std::vector<int>> &math_param_positions,
            const std::unordered_map<std::string, std::vector<InferredType>> &assumed_ret) const;

private:
    TypeEnvironment env_;
    int funcbody_depth_ = 0;

    // 不动点迭代轮次上限（实际通常 2 轮即可收敛）。
    static constexpr int kMaxSpecIterations = 16;
};

}// namespace fakelua
