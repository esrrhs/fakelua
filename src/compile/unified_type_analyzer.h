#pragma once

#include "compile/cfg.h"
#include "compile/compile_common.h"
#include "compile/shape_type.h"
#include "compile/ssa.h"
#include "compile/syntax_tree.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fakelua {

class UnifiedTypeAnalyzer {
public:
    using SSATypeInfo = fakelua::SSATypeInfo;
    using SpecParam = fakelua::SpecParam;
    using ParamAssumption = std::unordered_map<int, SSATypeInfo>;

    UnifiedTypeAnalyzer(ShapeRegistry *registry) : registry_(registry) {}

    // ── 主分析入口 ──────────────────────────────────────────────────────
    void Analyze(const std::string &func_name,
                 const SyntaxTreeInterfacePtr &func_block,
                 const CFGFunction &cfg,
                 SSAFunction &ssa,
                 InferResult &ir,
                 int bitmask = -1,
                 const ParamAssumption &param_assumptions = {});

    // ── 发现可特化参数 ──────────────────────────────────────────────────
    std::vector<SpecParam> FindSpecializableParams(const SyntaxTreeInterfacePtr &func_block,
                                                   const CFGFunction &cfg,
                                                   const SSAFunction &ssa,
                                                   const InferResult &ir);

    // ── Phase 3a: 计算 widest shapes ────────────────────────────────────
    void ComputeVarFinalShapes(const SSAFunction &ssa, InferResult &ir);
    void ComputeCtorTargetShapes(const SyntaxTreeInterfacePtr &func_block,
                                 const SSAFunction &ssa,
                                 InferResult &ir);

    // ── §8 逃逸分析 ────────────────────────────────────────────────────
    void ComputeEscape(const std::string &func_name,
                       const SyntaxTreeInterfacePtr &func_block,
                       const SSAFunction &ssa,
                       InferResult &ir);

    // ── §7 函数摘要 ────────────────────────────────────────────────────
    void BuildSummary(const std::string &func_name,
                      const SyntaxTreeInterfacePtr &func_block,
                      const SSAFunction &ssa,
                      const std::unordered_map<int, SSATypeInfo> &version_types,
                      InferResult &ir);

    SSATypeInfo ApplyCallSummary(const std::string &callee_name,
                                  const InferResult &ir) const;

private:
    // ── 数据结构 ────────────────────────────────────────────────────────
    // 类型环境：变量名 → 类型信息。不用 SSA 版本号而用变量名，
    // 是因为 SSABuilder 仍是骨架（不生成 use/versions），
    // 但按变量名 meet + 转移仍可做流敏感分析。
    using VarEnv = std::unordered_map<std::string, SSATypeInfo>;
    using TypeEnv = std::unordered_map<int, SSATypeInfo>;
    using VarNameToVersion = std::unordered_map<std::string, int>;

    // ── 工作表主循环（§6）──────────────────────────────────────────────
    // 函数返回每个块最终的 out_env。
    std::unordered_map<int, VarEnv> RunWorklist(
        const CFGFunction &cfg,
        const SSAFunction &ssa,
        const ParamAssumption &param_assumptions,
        const InferResult &ir);

    // 记录 env 变更的回调（未用，保留占位）
    std::function<void(const std::string &, const SSATypeInfo &)> env_callback;

    // ── 构建辅助 ────────────────────────────────────────────────────────
    VarNameToVersion BuildVarNameVersionMap(const SSAFunction &ssa);

    // ── 递归推导带 local_env ─────────────────────────────────────────────
    // local_env: 当前环境的 var_name → SSATypeInfo 覆盖
    SSATypeInfo InferExprType(const SyntaxTreeInterfacePtr &expr,
                              const SSAFunction &ssa,
                              const TypeEnv &version_types,
                              const InferResult &ir,
                              const VarNameToVersion &name_ver,
                              const VarEnv *local_env = nullptr);

    int BuildShapeFromCtor(const SyntaxTreeInterfacePtr &tc,
                           const SSAFunction &ssa,
                           const TypeEnv &version_types,
                           const InferResult &ir,
                           const VarEnv *local_env = nullptr);

    // 对单个语句做 env 转移（不写 main_ssa_types；供 worklist 固定点迭代用）
    VarEnv TransferStmt(const SyntaxTreeInterfacePtr &stmt,
                        const VarEnv &env,
                        const SSAFunction &ssa,
                        const TypeEnv &version_types);

    // 从 env 出发，填充 stmts 列表中所有子表达式节点的 main_ssa_types
    void PopulateNodeTypesFromStmts(const std::vector<SyntaxTreeInterfacePtr> &stmts,
                                    const VarEnv &env,
                                    const SSAFunction &ssa,
                                    const TypeEnv &version_types,
                                    InferResult &ir);

    // 两个 var_env 取 meet
    static VarEnv MeetEnv(const VarEnv &a, const VarEnv &b);

    static SSATypeInfo Meet(const SSATypeInfo &a, const SSATypeInfo &b);

    static void LinkExprToTargetShape(const SyntaxTreeInterfacePtr &node,
                                      const std::string &target_name,
                                      const std::unordered_map<std::string, int> &var_final_shapes,
                                      std::unordered_map<const SyntaxTreeInterface *, int> &ctor_target_shapes);

    void WalkEscape(const std::string &func_name,
                    const SyntaxTreeInterfacePtr &node,
                    const SSAFunction &ssa,
                    const std::unordered_map<int, SSATypeInfo> &version_types,
                    std::unordered_map<std::string, bool> &escape_vars,
                    InferResult &ir);

    ShapeRegistry *registry_ = nullptr;
    std::string cur_func_name_;
};

}// namespace fakelua
