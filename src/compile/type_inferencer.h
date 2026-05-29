#pragma once

#include "compile/compile_common.h"
#include <optional>
#include <unordered_set>

namespace fakelua {

class TypeInferencer {
public:
    // 运行全局类型推断，并在返回的 InferResult 中填充数学参数特化信息。
    InferResult Process(const ParseResult &pr);

private:
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

    // 试推断上下文：在 RunTrialInference 中创建，沿调用链传递，
    // 用于 ResolveCallReturnType 解析被调函数的返回类型。
    struct TrialInferenceContext {
        const std::unordered_map<std::string, std::vector<int>> *math_positions = nullptr;
        const std::unordered_map<std::string, std::vector<InferredType>> *assumed_ret = nullptr;
    };

    InferredType InferNode(const SyntaxTreeInterfacePtr &node, EvalTypeMap &current_map, const TrialInferenceContext *ctx = nullptr);

    InferredType InferExp(const std::shared_ptr<SyntaxTreeExp> &exp, EvalTypeMap &current_map, const TrialInferenceContext *ctx = nullptr);

    InferredType InferPrefixExp(const std::shared_ptr<SyntaxTreePrefixexp> &prefix_exp, EvalTypeMap &current_map, const TrialInferenceContext *ctx = nullptr);

    InferredType InferVar(const std::shared_ptr<SyntaxTreeVar> &var, EvalTypeMap &current_map, const TrialInferenceContext *ctx = nullptr);

    void InferBlock(const std::shared_ptr<SyntaxTreeBlock> &block, bool new_scope, EvalTypeMap &current_map, const TrialInferenceContext *ctx = nullptr);

    // -----------------------------------------------------------------------
    // 数学参数特化发现（迭代不动点推断）
    // -----------------------------------------------------------------------

    // 多轮迭代识别数学参数，记录到 ir.math_param_positions，
    // 同时返回数学函数信息（函数名 → {函数体, 参数列表}）。
    std::unordered_map<std::string, std::pair<SyntaxTreeInterfacePtr, std::vector<std::string>>>
    IdentifyMathParams(const ParseResult &pr, InferResult &ir);

    // 为所有数学函数生成初始特化快照，写入 ir.specialization_snapshots。
    // 每个函数生成 2^k 个快照（k = 数学参数个数）。
    void GenerateInitialSnapshots(
            InferResult &ir,
            const std::unordered_map<std::string, std::pair<SyntaxTreeInterfacePtr, std::vector<std::string>>> &math_func_info);

    // 以 assumed_types 中给定的参数类型假设运行 InferBlock，迭代直到稳定（不动点），
    // 返回各 AST 节点 → InferredType 的快照。
    // math_positions / assumed_ret 非 null 时，InferNode(FunctionCall) 会通过
    // ResolveCallReturnType 将被调函数的特化返回类型注入推断结果，
    // 使函数调用节点及其下游局部变量在快照中获得精确类型。
    EvalTypeMap RunTrialInference(const SyntaxTreeInterfacePtr &func_block,
                                  const std::vector<std::string> &params,
                                  const std::unordered_map<std::string, InferredType> &assumed_types,
                                  const std::unordered_map<std::string, std::vector<int>> *math_positions = nullptr,
                                  const std::unordered_map<std::string, std::vector<InferredType>> *assumed_ret = nullptr);

    // 判断 exp 节点是否为算术表达式（结果可为 T_INT/T_FLOAT 的运算符）。
    // 包括算术/位运算二元运算符，以及一元负号和按位取反。
    [[nodiscard]] bool IsArithmeticExpr(const SyntaxTreeInterfacePtr &node) const;

    // 判断 exp 节点是否为比较表达式（操作数可为数值，运算符为 </<=/>/>==/~=）。
    // 比较运算符本身返回布尔值（T_DYNAMIC），但若两侧操作数均为数值类型，
    // TryCompileNativeBoolExpr 能生成原生 C 比较，避免 CVar 装拆箱。
    [[nodiscard]] bool IsNativeComparisonExpr(const SyntaxTreeInterfacePtr &node) const;

    // 数学参数发现的统一类型变化检测器。
    // 仅做一次 WalkSyntaxTree 遍历，在同一回调中按节点类别分派四类检测规则：
    //   1. 算术表达式：节点自身类型变化（T_INT/T_FLOAT ↔ T_DYNAMIC）；
    //   2. 有序比较（< <= > >=）：左右操作数类型变化（均数值 ↔ 含 T_DYNAMIC）；
    //   3. for-loop：循环节点自身类型变化；
    //   4. 已知数学函数调用：数学参数位置实参类型变化。
    // improvement_mode=true 时检测"改善"（compare_map 节点为 T_DYNAMIC），
    // improvement_mode=false 时检测"退化"（compare_map 节点与 typed_map 不同）。
    // 注意：仅关注数学参数发现相关槽位，不检测任意节点的类型变化。
    [[nodiscard]] bool CheckArithmeticTypeChanges(const EvalTypeMap &typed_map, const EvalTypeMap &compare_map,
                                                  const SyntaxTreeInterfacePtr &func_block,
                                                  bool improvement_mode,
                                                  const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const;

    [[nodiscard]] std::vector<FunctionSpecInfo> CollectFunctionSpecInfos(const ParseResult &pr) const;

    std::vector<int> FindMathParamIndices(const FunctionSpecInfo &info,
                                          const EvalTypeMap &baseline,
                                          const EvalTypeMap &all_int,
                                          const std::unordered_map<std::string, std::vector<int>> &known_math_positions);

    [[nodiscard]] std::unordered_map<std::string, FuncRetInfo> BuildFunctionReturnCache(
            const std::unordered_map<std::string, std::pair<SyntaxTreeInterfacePtr, std::vector<std::string>>> &math_func_info) const;

    void InferSpecializationReturnTypes(
            InferResult &ir,
            const std::unordered_map<std::string, std::pair<SyntaxTreeInterfacePtr, std::vector<std::string>>> &math_func_info,
            const std::unordered_map<std::string, FuncRetInfo> &func_ret_cache);

    // 检查 block_node 的所有执行路径是否均以 return 语句结束。
    // 能识别 if-else（所有分支均返回）的情况，不递归进入嵌套函数体。
    [[nodiscard]] bool AllPathsReturn(const SyntaxTreeInterfacePtr &block_node) const;

    // 从 block_node 中浅层收集每条 return 语句的第一个返回表达式。
    // 返回 true 表示所有路径均以 return 结束（无隐式 nil 返回路径）。
    bool CollectReturnExps(const SyntaxTreeInterfacePtr &block_node,
                           std::vector<SyntaxTreeInterfacePtr> &ret_exps) const;

    // 试推断期间，根据上下文提示解析函数调用的实际返回类型。
    // 上下文为 null 时（主推断遍）始终返回 T_DYNAMIC。
    [[nodiscard]] InferredType ResolveCallReturnType(
            const std::shared_ptr<SyntaxTreeFunctioncall> &fc,
            const EvalTypeMap &current_map,
            const TrialInferenceContext *ctx) const;

    // 从 RunTrialInference 生成的精确快照中直接读取 return 表达式节点的类型，
    // 汇总得出该特化版本的函数返回类型（T_INT / T_FLOAT / T_DYNAMIC）。
    [[nodiscard]] InferredType ComputeReturnTypeFromSnapshot(
            const EvalTypeSnapshot &snapshot,
            const FuncRetInfo &ret_info) const;

private:
    TypeEnvironment env_;
    // 是否正在推断函数体内部（true）还是文件顶层（false）。
    // 用于区分文件级 local 变量和函数体内局部变量，以决定是否写入 file_level_types_。
    bool in_funcbody_ = false;

    // 文件顶层（!in_funcbody_）的数值类型局部变量映射：变量名 → T_INT/T_FLOAT。
    // 在 InferNode LocalVar 阶段填充（仅记录数值字面量初始化的顶层 local 变量）。
    // RunTrialInference 在重置 env_ 后用此表重新注入这些常量，
    // 使函数体的试推断能看到正确的文件级常量类型，进而支持函数特化。
    std::unordered_map<std::string, InferredType> file_level_types_;

    // 试推断期间被固定（pinned）的变量名集合：这些变量对应当前特化版本的数学参数，
    // 其 env 类型在 InferNode(Assign) 中不可被降级（以模拟运行时类型检查的保证）。
    std::unordered_set<std::string> pinned_vars_;

    // 试推断期间跳过 InferBlock 的后处理（变量最终类型覆写初始化表达式类型）。
    // 后处理是为 CGen 提供声明类型信息，但会污染试推断快照中算术表达式节点的类型，
    // 导致 CheckArithmeticTypeChanges 无法正确检测算术改善/退化。
    bool skip_post_processing_ = false;

    // 不动点迭代轮次上限（实际通常 2 轮即可收敛）。
    static constexpr int kMaxSpecIterations = 16;
};

}// namespace fakelua
