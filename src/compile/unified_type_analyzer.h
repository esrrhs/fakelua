#pragma once

#include "compile/cfg.h"
#include "compile/compile_common.h"
#include "compile/shape_type.h"
#include "compile/ssa.h"
#include "compile/syntax_tree.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fakelua {

class UnifiedTypeAnalyzer {
public:
    using SSATypeInfo = fakelua::SSATypeInfo;

    UnifiedTypeAnalyzer(ShapeRegistry *registry) : registry_(registry) {}

    // ── 主分析入口 ──────────────────────────────────────────────────────
    void Analyze(const std::string &func_name,
                 const SyntaxTreeInterfacePtr &func_block,
                 const CFGFunction &cfg,
                 SSAFunction &ssa,
                 InferResult &ir);

    // ── Phase 3a: 计算 widest shapes ────────────────────────────────────
    void ComputeVarFinalShapes(const SSAFunction &ssa, InferResult &ir);
    void ComputeCtorTargetShapes(const SyntaxTreeInterfacePtr &func_block,
                                 const SSAFunction &ssa,
                                 InferResult &ir);

    // ── §7 函数摘要 ────────────────────────────────────────────────────
    void BuildSummary(const std::string &func_name,
                      const SyntaxTreeInterfacePtr &func_block,
                      const SSAFunction &ssa,
                      const std::unordered_map<int, SSATypeInfo> &version_types,
                      InferResult &ir);

private:
public:
    // ── 数据结构 ────────────────────────────────────────────────────────
    // 类型环境：变量名 → 类型信息。
    using VarEnv = std::unordered_map<std::string, SSATypeInfo>;
    using TypeEnv = std::unordered_map<int, SSATypeInfo>;
    using VarNameToVersion = std::unordered_map<std::string, int>;

private:
    // ── 工作表主循环（§6）──────────────────────────────────────────────
    std::unordered_map<int, VarEnv> RunWorklist(
        const CFGFunction &cfg,
        const SSAFunction &ssa,
        const InferResult &ir);

    // ── 构建辅助 ────────────────────────────────────────────────────────
    VarNameToVersion BuildVarNameVersionMap(const SSAFunction &ssa);

    // ── 递归推导 ────────────────────────────────────────────────────────
    SSATypeInfo InferExprType(const SyntaxTreeInterfacePtr &expr,
                              const SSAFunction &ssa,
                              const TypeEnv &version_types,
                              const InferResult &ir,
                              const VarNameToVersion &name_ver,
                              const VarEnv *local_env = nullptr);

    int BuildShapeFromCtor(const SyntaxTreeInterfacePtr &tc,
                           const SSAFunction &ssa,
                           const TypeEnv &version_types,
                           const InferResult &ir);

    // 对单个语句做 env 转移
    VarEnv TransferStmt(const SyntaxTreeInterfacePtr &stmt,
                        const VarEnv &env,
                        const SSAFunction &ssa,
                        const TypeEnv &version_types);

    // 从 env 出发，填充 stmts 列表中所有子表达式节点的类型到 ir.main_ssa_types
    void PopulateNodeTypesFromStmts(const std::vector<SyntaxTreeInterfacePtr> &stmts,
                                    const VarEnv &env,
                                    const SSAFunction &ssa,
                                    const TypeEnv &version_types,
                                    InferResult &ir,
                                    std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> *out_map = nullptr);

    // 两个 var_env 取 meet
    static VarEnv MeetEnv(const VarEnv &a, const VarEnv &b);
    static SSATypeInfo Meet(const SSATypeInfo &a, const SSATypeInfo &b);

    static void LinkExprToTargetShape(const SyntaxTreeInterfacePtr &node,
                                      const std::string &target_name,
                                      const std::unordered_map<std::string, int> &var_final_shapes,
                                      std::unordered_map<const SyntaxTreeInterface *, int> &ctor_target_shapes);

    ShapeRegistry *registry_ = nullptr;
    std::string cur_func_name_;
    int cur_bitmask_ = -1;  // reserved for future per-call-site specialization
};

}// namespace fakelua
