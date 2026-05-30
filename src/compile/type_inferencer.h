#pragma once

#include "compile/compile_common.h"
#include <optional>
#include <unordered_set>

namespace fakelua {

class TypeInferencer {
public:
    // 运行全局类型推断，并在返回的 InferResult 中填充数学参数特化信息。
    InferResult InferTypes(const ParseResult &pr, const CompileConfig &cfg);

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

    struct MathFuncInfo {
        SyntaxTreeInterfacePtr block;
        std::vector<std::string> params;
    };

    using MathFuncInfoMap = std::unordered_map<std::string, MathFuncInfo>;

    struct FuncRetInfo {
        std::vector<SyntaxTreeInterfacePtr> ret_exps;
        bool ends_with_return = false;
    };

    // node指针 → 推断类型的快照映射，用于特化发现的不动点迭代。
    using EvalTypeMap = EvalTypeSnapshot;

    // 辅助工具：往快照 map 中写入类型并直接返回（单一出口，保证 map 与返回值一致）
    static inline InferredType RecordType(EvalTypeMap &m, SyntaxTreeInterface *n, InferredType t) {
        m[n] = t;
        return t;
    }


    // 试推断上下文：在 RunTrialInference 中创建，沿调用链传递，
    // 用于 ResolveCallReturnType 解析被调函数的返回类型。
    struct TrialInferenceContext {
        const std::unordered_map<std::string, std::vector<int>> *math_positions = nullptr;
        const std::unordered_map<std::string, std::vector<InferredType>> *assumed_ret = nullptr;
        const std::unordered_set<std::string> *pinned_vars = nullptr;
        bool skip_post_processing = false;
    };

    struct TraversalContext {
        EvalTypeMap &current_map;
        TypeEnvironment &env;
        std::unordered_map<std::string, InferredType> &file_level_types;
        bool in_funcbody = false;
        const TrialInferenceContext *ctx = nullptr;

        [[nodiscard]] bool IsTrialInference() const {
            return ctx != nullptr;
        }

        [[nodiscard]] bool IsPinnedVar(const std::string &name) const {
            return ctx && ctx->pinned_vars && ctx->pinned_vars->contains(name);
        }

        [[nodiscard]] bool SkipPostProcessing() const {
            return ctx && ctx->skip_post_processing;
        }
    };

    InferredType InferNode(const SyntaxTreeInterfacePtr &node, TraversalContext &tctx);

    InferredType InferExp(const std::shared_ptr<SyntaxTreeExp> &exp, TraversalContext &tctx);

    InferredType InferPrefixExp(const std::shared_ptr<SyntaxTreePrefixexp> &prefix_exp, TraversalContext &tctx);

    InferredType InferVar(const std::shared_ptr<SyntaxTreeVar> &var, TraversalContext &tctx);

    void InferBlock(const std::shared_ptr<SyntaxTreeBlock> &block, bool new_scope, TraversalContext &tctx);

    // 辅助分析不同类型语句的私有成员函数，用于拆分庞大的 Switch 分支
    InferredType InferLocalVar(const std::shared_ptr<SyntaxTreeLocalVar> &local_var, TraversalContext &tctx);
    InferredType InferAssign(const std::shared_ptr<SyntaxTreeAssign> &assign, TraversalContext &tctx);
    InferredType InferForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_loop, TraversalContext &tctx);
    InferredType InferForIn(const std::shared_ptr<SyntaxTreeForIn> &for_in, TraversalContext &tctx);
    InferredType InferWhile(const std::shared_ptr<SyntaxTreeWhile> &while_stmt, TraversalContext &tctx);
    InferredType InferRepeat(const std::shared_ptr<SyntaxTreeRepeat> &repeat_stmt, TraversalContext &tctx);
    InferredType InferIf(const std::shared_ptr<SyntaxTreeIf> &if_stmt, TraversalContext &tctx);

    // -----------------------------------------------------------------------
    // 数学参数特化发现（迭代不动点推断）
    // -----------------------------------------------------------------------

    // 多轮迭代识别数学参数，记录到 ir.math_param_positions，
    // 同时返回数学函数信息。
    MathFuncInfoMap IdentifyMathParams(const ParseResult &pr, InferResult &ir,
                                       const std::unordered_map<std::string, InferredType> &file_level_types);

    // 为所有数学函数生成初始特化快照，写入 ir.specialization_snapshots。
    // 每个函数生成 2^k 个快照（k = 数学参数个数）。
    void GenerateInitialSnapshots(InferResult &ir, const MathFuncInfoMap &math_func_info,
                                  const std::unordered_map<std::string, InferredType> &file_level_types);

    // 以 assumed_types 中给定的参数类型假设运行 InferBlock，迭代直到稳定（不动点），
    // 返回各 AST 节点 → InferredType 的快照。
    // math_positions / assumed_ret 非 null 时，InferNode(FunctionCall) 会通过
    // ResolveCallReturnType 将被调函数的特化返回类型注入推断结果，
    // 使函数调用节点及其下游局部变量在快照中获得精确类型。
    EvalTypeMap RunTrialInference(const SyntaxTreeInterfacePtr &func_block, const std::vector<std::string> &params,
                                  const std::unordered_map<std::string, InferredType> &assumed_types,
                                  const std::unordered_map<std::string, InferredType> &file_level_types,
                                  const std::unordered_map<std::string, std::vector<int>> *math_positions = nullptr,
                                  const std::unordered_map<std::string, std::vector<InferredType>> *assumed_ret = nullptr,
                                  bool skip_post_processing = false);

    // 判断 exp 节点是否为算术表达式（结果可为 T_INT/T_FLOAT 的运算符）。
    // 包括算术/位运算二元运算符，以及一元负号 and 按位取反。
    [[nodiscard]] bool IsArithmeticExpr(const SyntaxTreeInterfacePtr &node) const;

    // 判断 exp 节点是否为比较表达式（操作数可为数值，运算符为 </<=/>/>==/~=）。
    // 比较运算符本身返回布尔值（T_DYNAMIC），但若两侧操作数均为数值类型，
    // TryCompileNativeBoolExpr 能生成原生 C 比较，避免 CVar 装拆箱。
    [[nodiscard]] bool IsNativeComparisonExpr(const SyntaxTreeInterfacePtr &node) const;

    // 数学参数发现的统一类型变化检测器。
    [[nodiscard]] bool CheckArithmeticTypeChanges(const EvalTypeMap &typed_map, const EvalTypeMap &compare_map,
                                                  const SyntaxTreeInterfacePtr &func_block, bool improvement_mode,
                                                  const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const;

    // 拆分出的细粒度节点检测函数
    [[nodiscard]] bool CheckArithmeticNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map,
                                                 const EvalTypeMap &compare_map, bool improvement_mode) const;
    [[nodiscard]] bool CheckComparisonNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map,
                                                 const EvalTypeMap &compare_map, bool improvement_mode) const;
    [[nodiscard]] bool CheckForLoopNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map,
                                              const EvalTypeMap &compare_map, bool improvement_mode) const;
    [[nodiscard]] bool CheckCallNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map, const EvalTypeMap &compare_map,
                                           const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const;

    [[nodiscard]] std::vector<FunctionSpecInfo> CollectFunctionSpecInfos(const ParseResult &pr) const;

    std::vector<int> FindMathParamIndices(const FunctionSpecInfo &info, const EvalTypeMap &baseline, const EvalTypeMap &all_int,
                                          const std::unordered_map<std::string, std::vector<int>> &known_math_positions,
                                          const std::unordered_map<std::string, InferredType> &file_level_types);

    [[nodiscard]] std::unordered_map<std::string, FuncRetInfo> BuildFunctionReturnCache(const MathFuncInfoMap &math_func_info) const;

    void InferSpecializationReturnTypes(InferResult &ir, const MathFuncInfoMap &math_func_info,
                                        const std::unordered_map<std::string, FuncRetInfo> &func_ret_cache,
                                        const std::unordered_map<std::string, InferredType> &file_level_types);


    // 检查 block_node 的所有执行路径是否均以 return 语句结束。
    // 能识别 if-else（所有分支均返回）的情况，不递归进入嵌套函数体。
    [[nodiscard]] bool AllPathsReturn(const SyntaxTreeInterfacePtr &block_node) const;

    // 从 block_node 中浅层收集每条 return 语句的第一个返回表达式。
    // 返回 true 表示所有路径均以 return 结束（无隐式 nil 返回路径）。
    bool CollectReturnExps(const SyntaxTreeInterfacePtr &block_node, std::vector<SyntaxTreeInterfacePtr> &ret_exps) const;

    // 试推断期间，根据上下文提示解析函数调用的实际返回类型。
    // 上下文为 null 时（主推断遍）始终返回 T_DYNAMIC。
    [[nodiscard]] InferredType ResolveCallReturnType(const std::shared_ptr<SyntaxTreeFunctioncall> &fc, const TraversalContext &tctx) const;

    // 从 RunTrialInference 生成的精确快照中直接读取 return 表达式节点的类型，
    // 汇总得出该特化版本的函数返回类型（T_INT / T_FLOAT / T_DYNAMIC）。
    [[nodiscard]] InferredType ComputeReturnTypeFromSnapshot(const EvalTypeSnapshot &snapshot, const FuncRetInfo &ret_info) const;

private:
    // 辅助工具：构造数学参数敏感性测试与特化的参数假设映射表
    [[nodiscard]] std::unordered_map<std::string, InferredType> MakeAssumedParamTypes(const std::vector<std::string> &params,
                                                                                      const std::string &special_param,
                                                                                      InferredType special_type,
                                                                                      InferredType default_type) const;

    [[nodiscard]] std::unordered_map<std::string, InferredType>
    MakeSpecializedParamTypes(const std::vector<std::string> &params, const std::vector<int> &math_indices, int bitmask) const;

private:

    // 不动点迭代轮次上限（实际通常 2 轮即可收敛）。
    static constexpr int kMaxSpecIterations = 16;
};

}// namespace fakelua
