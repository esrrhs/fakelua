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

        [[nodiscard]] size_t GetScopeDepth() const {
            return scopes_.size();
        }

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
    MathFuncInfoMap IdentifyMathParams(const ParseResult &pr, InferResult &ir);

    // 为所有数学函数生成初始特化快照，写入 ir.specialization_snapshots。
    // 每个函数生成 2^k 个快照（k = 数学参数个数）。
    void GenerateInitialSnapshots(InferResult &ir, const MathFuncInfoMap &math_func_info);

    // 以 assumed_types 中给定的参数类型假设运行 InferBlock，迭代直到稳定（不动点），
    // 返回各 AST 节点 → InferredType 的快照。
    // math_positions / assumed_ret 非 null 时，InferNode(FunctionCall) 会通过
    // ResolveCallReturnType 将被调函数的特化返回类型注入推断结果，
    // 使函数调用节点及其下游局部变量在快照中获得精确类型。
    EvalTypeMap RunTrialInference(const SyntaxTreeInterfacePtr &func_block, const std::vector<std::string> &params, const std::unordered_map<std::string, InferredType> &assumed_types,
                                  const std::unordered_map<std::string, std::vector<int>> *math_positions = nullptr,
                                  const std::unordered_map<std::string, std::vector<InferredType>> *assumed_ret = nullptr, bool skip_post_processing = false);

    // 判断 exp 节点是否为算术表达式（结果可为 T_INT/T_FLOAT 的运算符）。
    // 包括算术/位运算二元运算符，以及一元负号 and 按位取反。
    [[nodiscard]] bool IsArithmeticExpr(const SyntaxTreeInterfacePtr &node) const;

    // 判断 exp 节点是否为原生比较表达式（运算符为 < / <= / > / >=）。
    // 不包含 == 和 ~=，因为它们可作用于任何 Lua 类型，不适合用于数学参数发现。
    // 比较运算符本身返回布尔值（T_DYNAMIC），但若两侧操作数均为数值类型，
    // TryCompileNativeBoolExpr 能生成原生 C 比较，避免 CVar 装拆箱。
    [[nodiscard]] bool IsNativeComparisonExpr(const SyntaxTreeInterfacePtr &node) const;

    // 数学参数发现的统一类型变化检测器。
    [[nodiscard]] bool CheckArithmeticTypeChanges(const EvalTypeMap &typed_map, const EvalTypeMap &compare_map, const SyntaxTreeInterfacePtr &func_block, bool improvement_mode,
                                                  const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const;

    // 拆分出的细粒度节点检测函数
    [[nodiscard]] bool CheckArithmeticNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map, const EvalTypeMap &compare_map, bool improvement_mode) const;
    [[nodiscard]] bool CheckComparisonNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map, const EvalTypeMap &compare_map, bool improvement_mode) const;
    [[nodiscard]] bool CheckForLoopNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map, const EvalTypeMap &compare_map, bool improvement_mode) const;
    [[nodiscard]] bool CheckCallNodeChange(const SyntaxTreeInterfacePtr &node, const EvalTypeMap &typed_map, const EvalTypeMap &compare_map,
                                           const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const;

    [[nodiscard]] std::vector<FunctionSpecInfo> CollectFunctionSpecInfos(const ParseResult &pr) const;

    std::vector<int> FindMathParamIndices(const FunctionSpecInfo &info, const EvalTypeMap &baseline, const EvalTypeMap &all_int,
                                          const std::unordered_map<std::string, std::vector<int>> &known_math_positions);

    [[nodiscard]] std::unordered_map<std::string, FuncRetInfo> BuildFunctionReturnCache(const MathFuncInfoMap &math_func_info) const;

    void InferSpecializationReturnTypes(InferResult &ir, const MathFuncInfoMap &math_func_info, const std::unordered_map<std::string, FuncRetInfo> &func_ret_cache);


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

    void CollectGlobalConstVars(const ParseResult &pr, const EvalTypeMap &current_map, InferResult &ir);

private:
    // 辅助工具：构造数学参数敏感性测试与特化的参数假设映射表
    [[nodiscard]] std::unordered_map<std::string, InferredType> MakeAssumedParamTypes(const std::vector<std::string> &params, const std::string &special_param, InferredType special_type,
                                                                                      InferredType default_type) const;

    [[nodiscard]] std::unordered_map<std::string, InferredType> MakeSpecializedParamTypes(const std::vector<std::string> &params, const std::vector<int> &math_indices, int bitmask) const;

    void DumpASTWithTypes(const SyntaxTreeInterfacePtr &node, const EvalTypeSnapshot &snapshot, int tab, std::ostream &os) const;

    // table 特化分析：流不敏感地收集每个变量被赋值过的所有 table constructor
    // 字段并集，为每个 constructor 节点标记其应 emit 的合并字段布局（含 optional 标记）。
    // CGen 在 CompileTableconstructor 中消费 ir.table_spec_infos 得到合并布局，
    // 使 if-else 两分支构造的不同 shape table 能统一到同一结构体，字段访问走 FL_SPEC。
    void AnalyzeTableShapes(const SyntaxTreeInterfacePtr &chunk, InferResult &ir);

    // 从 table constructor 字面量提取字段信息（key/kind/c_field_name/数值）。
    // 复用 CGen::GetTableFields 的 key 解析逻辑，但不依赖类型推断（type 留 T_DYNAMIC）。
    // 返回 false 表示该 constructor 不可特化（含非静态 key 或 vararg/funcall 数组值）。
    static bool BuildCtorFields(const SyntaxTreeInterfacePtr &tc, std::vector<TableFieldInfo> &out);

    // 将 src 的字段并集到 dst（按 key 描述符去重，已存在则保留 dst 中的条目）。
    static void MergeFieldsInto(std::vector<TableFieldInfo> &dst, const std::vector<TableFieldInfo> &src);

    // 字段 key 描述符（与 CGen::GetKeyDescriptor 一致，用于去重和签名）。
    static std::string FieldKeyDescriptor(const TableFieldInfo &f);

    // -----------------------------------------------------------------------
    // 流敏感 table 特化前向分析
    // -----------------------------------------------------------------------
    //
    // 为每个 Var 引用节点（kDot/kSquare 的 prefixexp 所指简单变量）标注
    // 「在该程序点，该变量的 spec 类型名是什么（空串 = dynamic）」。
    // CGen 只读这些标注，不再自己维护 table_spec_types_ 等流敏感状态。
    //
    // 算法：对每个函数体 + 顶层 chunk 顺序遍历语句，维护
    //   state.local  : 变量名 → spec_type_name（每函数清空）
    //   state.global : 顶层 chunk 变量名 → spec_type_name（跨函数持久）
    // 顺序语句按 CGen 现有语义更新 state；if-else 做快照/独立分支/phase-1 join
    // （各分支一致才保留，否则 erase 降级为 dynamic）。

    struct FlowState {
        std::unordered_map<std::string, std::string> local;
        std::unordered_map<std::string, std::string> global;

        FlowState() = default;
        FlowState(std::unordered_map<std::string, std::string> l, std::unordered_map<std::string, std::string> g)
            : local(std::move(l)), global(std::move(g)) {}
        explicit FlowState(std::unordered_map<std::string, std::string> g) : local(), global(std::move(g)) {}
    };

    // 在流状态中查找某变量名的 spec 类型名（local 优先，fallback 到 global；都无 = 空串）。
    static std::string LookupSpec(const FlowState &st, const std::string &name);

    // 已知 rhs 的语法形式，推断它会给被赋变量带来的 spec 类型名（空串 = dynamic）。
    // 覆盖三种情形：字面量构造器（查 table_spec_infos 得合并布局）、简单变量拷贝、其余降为空。
    static std::string SpecFromRhs(const SyntaxTreeInterfacePtr &exp, const FlowState &st, const InferResult &ir);

    // 对一棵表达式子树里所有被读取的 Var(kDot/kSquare) 引用节点做标注：
    // 把 receiver 指代变量在当前流状态下的 spec 类型名写入 ir.var_spec_annotations[node]。
    // FAKELUA_SET_TABLE(args[0]) 走独立分支，receiver 为 args[0] 那段 Exp。
    void AnnotateExprs(const SyntaxTreeInterfacePtr &node, FlowState &st, InferResult &ir);

    // 顺序处理一条语句：先标注该语句中所有 Var 引用再按语义更新流状态。
    void FlowStmt(const SyntaxTreeInterfacePtr &stmt, FlowState &st, InferResult &ir, bool is_top_level);

    // 顺序处理一个语句块。
    void FlowBlock(const SyntaxTreeInterfacePtr &block, FlowState &st, InferResult &ir, bool is_top_level);

    // 入口：对整个模块（顶层 chunk + 所有函数体）跑前向流分析，填充 ir.var_spec_annotations。
    void ComputeVarSpecAnnotations(const ParseResult &pr, InferResult &ir);

    // 按 CGen 的 JoinSpecSnapshots 思路，把若干分支终态汇合到 out 的 local/global 两 map。
    // 任一分支缺失该 key、或各分支值不一致 → 不写入（等价于降级为 dynamic）。
    static void JoinFlowStates(const std::vector<FlowState> &branch_states, FlowState &out);

    // 辅助：取 Function / LocalFunction 语句的 body block。
    static SyntaxTreeInterfacePtr FuncBodyBlock(const SyntaxTreeInterfacePtr &func);

    // 预计算 per-spec-type 字段布局元数据（ir.spec_type_metadata）。
    // 遍历 ir.table_spec_infos，按 spec 类型名（字段签名哈希）去重，为每个 spec 类型建好字段布局索引。
    // CGen 据此发射 typedef / getter / setter，不再自行计算字段布局。
    static void ComputeSpecTypeMetadata(InferResult &ir);

private:
    std::unordered_map<std::string, InferredType> file_level_types_;

    // 不动点迭代轮次上限（实际通常 2 轮即可收敛）。
    static constexpr int kMaxSpecIterations = 16;
};

}// namespace fakelua
