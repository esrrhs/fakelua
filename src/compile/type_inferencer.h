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
    struct FunctionSpecInfo {
        std::string name;
        SyntaxTreeInterfacePtr block;
        std::vector<std::string> params;
    };

    struct FuncRetInfo {
        std::vector<SyntaxTreeInterfacePtr> ret_exps;
        bool ends_with_return = false;
    };

    // node指针 → 推断类型的快照映射，用于特化发现的不动点迭代。
    using EvalTypeMap = EvalTypeSnapshot;

    InferredType InferNode(const SyntaxTreeInterfacePtr &node);

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
    // 返回各 AST 节点 → InferredType 的快照。
    EvalTypeMap RunTrialInference(const SyntaxTreeInterfacePtr &func_block,
                                  const std::vector<std::string> &params,
                                  const std::unordered_map<std::string, InferredType> &assumed_types);

    // 判断 exp 节点是否为算术表达式（结果可为 T_INT/T_FLOAT 的运算符）。
    // 包括算术/位运算二元运算符，以及一元负号和按位取反。
    [[nodiscard]] bool IsArithmeticExpr(const SyntaxTreeInterfacePtr &node) const;

    // 判断 exp 节点是否为比较表达式（操作数可为数值，运算符为 </<=/>/>==/~=）。
    // 比较运算符本身返回布尔值（T_DYNAMIC），但若两侧操作数均为数值类型，
    // TryCompileNativeBoolExpr 能生成原生 C 比较，避免 CVar 装拆箱。
    [[nodiscard]] bool IsNativeComparisonExpr(const SyntaxTreeInterfacePtr &node) const;

    // 判断 all_int（全参数=T_INT）相对于 baseline（全参数=T_DYNAMIC）是否有算术/比较改善。
    // 算术改善：算术节点从 T_DYNAMIC 变为 T_INT/T_FLOAT。
    // 比较改善：比较节点（</<=/>/>==/~=）两侧操作数从含 T_DYNAMIC 变为全部 T_INT/T_FLOAT
    //           （使 TryCompileNativeBoolExpr 能生成原生 C 比较）。
    // 同时检测对已知数学函数的调用：若某数学参数位置实参在 all_int 中有类型但 baseline 中为
    // T_DYNAMIC，则视为算术改善。
    [[nodiscard]] bool HasArithmeticImprovement(const EvalTypeMap &all_int, const EvalTypeMap &baseline,
                                                const SyntaxTreeInterfacePtr &func_block,
                                                const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const;

    // 判断 all_int 中某个参数被置回 T_DYNAMIC 后（without_p），是否有算术/比较退化。
    // 算术退化：算术节点从 T_INT/T_FLOAT 变为 T_DYNAMIC。
    // 比较退化：比较节点两侧操作数从全部 T_INT/T_FLOAT 变为含 T_DYNAMIC。
    // 同时检测对已知数学函数的调用：若去掉该参数导致某数学函数调用的实参失去类型，则视为退化。
    [[nodiscard]] bool ParamAffectsArithmetic(const EvalTypeMap &all_int, const EvalTypeMap &without_p,
                                              const SyntaxTreeInterfacePtr &func_block,
                                              const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const;
    // 检查 func_block 中是否存在对已知数学函数的调用，其数学参数位置的实参在 typed_map 中有类型
    // (T_INT/T_FLOAT) 但在 compare_map 中类型不同（即发生了改善或退化）。
    [[nodiscard]] bool HasMathCallImprovement(const SyntaxTreeInterfacePtr &func_block,
                                              const EvalTypeMap &typed_map,
                                              const EvalTypeMap &compare_map,
                                              const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const;

    [[nodiscard]] bool HasArithmeticNodeTypeChange(const EvalTypeMap &typed_map,
                                                   const EvalTypeMap &compare_map,
                                                   const SyntaxTreeInterfacePtr &func_block,
                                                   bool require_compare_dynamic) const;

    [[nodiscard]] bool HasComparisonOperandTypeChange(const EvalTypeMap &typed_map,
                                                       const EvalTypeMap &compare_map,
                                                       const SyntaxTreeInterfacePtr &func_block) const;

    [[nodiscard]] bool HasForLoopTypeChange(const EvalTypeMap &typed_map,
                                            const EvalTypeMap &compare_map,
                                            const SyntaxTreeInterfacePtr &func_block) const;

    [[nodiscard]] std::vector<FunctionSpecInfo> CollectFunctionSpecInfos(const CompileResult &cr) const;

    std::vector<int> FindMathParamIndices(const FunctionSpecInfo &info,
                                          const EvalTypeMap &baseline,
                                          const EvalTypeMap &all_int,
                                          const std::unordered_map<std::string, std::vector<int>> &known_math_positions);

    void GenerateFunctionSpecializationSnapshots(CompileResult &cr,
                                                 const FunctionSpecInfo &info,
                                                 const std::vector<int> &math_indices);

    [[nodiscard]] std::unordered_map<std::string, FuncRetInfo> BuildFunctionReturnCache(
            const std::unordered_map<std::string, std::pair<SyntaxTreeInterfacePtr, std::vector<std::string>>> &math_func_info) const;

    [[nodiscard]] std::unordered_map<std::string, std::vector<InferredType>> InferSpecializationReturnTypes(
            const CompileResult &cr,
            const std::unordered_map<std::string, std::pair<SyntaxTreeInterfacePtr, std::vector<std::string>>> &math_func_info,
            const std::unordered_map<std::string, FuncRetInfo> &func_ret_cache) const;

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

    // 扫描函数块顶层的 local 声明，将由数学函数调用初始化的局部变量
    // 的类型（T_INT/T_FLOAT）追加到 spec_ctx 中，支持链式传播：
    //   local x = f(n)  → x 的类型由 EvalReturnExpType(f(n)) 推出
    //   local y = x + 1 → y 的类型由 x（已在 spec_ctx 中）推出
    // 仅处理顶层 LocalVar 语句（不递归进入嵌套函数体）。
    void BuildLocalVarExtensions(
            const SyntaxTreeInterfacePtr &func_block,
            const EvalTypeSnapshot &snapshot,
            std::unordered_map<std::string, InferredType> &spec_ctx,
            const std::unordered_map<std::string, std::vector<int>> &math_param_positions,
            const std::unordered_map<std::string, std::vector<InferredType>> &assumed_ret) const;

private:
    TypeEnvironment env_;
    int funcbody_depth_ = 0;
    // 当前推断遍次中所有已推断节点的类型映射：节点指针 → 推断类型。
    // 替代原先内嵌在 AST 节点的 eval_type_ 字段，避免 AST 与推断过程耦合。
    // 在 Process() 开头清空，随 InferNode 调用逐步填充，最终复制给 cr.main_eval_types。
    // RunTrialInference 在每轮开始时清除 func_block 节点的条目，试推断结束后
    // 该映射反映最后一轮的推断结果（直到下一次 Process() 或 RunTrialInference 覆盖）。
    EvalTypeMap current_map_;

    // 不动点迭代轮次上限（实际通常 2 轮即可收敛）。
    static constexpr int kMaxSpecIterations = 16;
};

}// namespace fakelua
