#pragma once

// ══════════════════════════════════════════════════════════════════════════════
// UnifiedTypeAnalyzer（UTA）— SSA/CFG/Shape 管线的核心类型推导引擎
// ══════════════════════════════════════════════════════════════════════════════
//
// 【整体架构】
//   UTA 是整个 SSA 管线的类型分析核心。它接受 SSA 化后的函数（SSAFunction）
//   与控制流图（CFGFunction），对每个函数做一次"流敏感"（flow-sensitive）的
//   类型推导——即在 CFG 的每个基本块出入口维护一个"类型环境"（VarEnv），
//   沿着控制流传播并在 φ 节点处合并（Meet），最终产出：
//
//     1. 每个 SSA 版本的静态类型（TypeEnv / ir.ssa_version_types）
//     2. 每个 AST 节点对应的推导类型（ir.main_ssa_types）
//     3. 每个变量所有版本的"最宽" final shape（ir.var_final_shapes）
//     4. 每个 table constructor 的目标 shape（ir.ctor_target_shapes）
//     5. 函数摘要 FuncSummary（参数类型 / 返回类型 / 逃逸信息 / HM 多态签名）
//     6. 逃逸分析结果（ir.escape_vars）
//
// 【在管线中的位置】
//   ParseResult
//       ↓  语法分析
//   SyntaxTree (AST)
//       ↓  CFGBuilder
//   CFGFunction（基本块 + 控制流边）
//       ↓  SSABuilder（Cytron 1991）
//   SSAFunction（φ 节点 + 版本号）
//       ↓  ★★★ UnifiedTypeAnalyzer ★★★  ← 本文件
//   InferResult（所有类型的"真相源"）
//       ↓  CGen
//   目标 C 代码
//
// 【调用关系】（由 TypeInferencer::RunSSAAnalysis 驱动）
//   对每个函数：
//     ① uta.Analyze(...)          — 主分析：工作表 + φ 推导 + 逃逸分析
//     ② uta.ComputeCtorTargetShapes(...) — Phase 3a：table 构造子 → target shape
//     ③ uta.BuildSummary(...)     — §7：为当前函数写 FuncSummary
//   顶层 chunk 额外调一次 Analyze（没有 BuildSummary / ComputeCtorTargetShapes）。
//
// 【设计要点】
//   - 流敏感：类型环境随语句顺序在基本块内流动，块间通过 in/out 传播。
//   - 工作表迭代（§6）：带 widening 的不动点迭代，保证循环上 shape 收敛。
//   - 常量传播（§12.6）：形如 a[key] 的访问用 ConstEnv 在编译期解析 key。
//   - 过程间分析（§7）：用 FuncSummary 推断函数调用的返回类型。
//   - 逃逸分析（§8）：标记变量是否逃逸，用于代码生成决定是否走栈分配。
//   - Hindley-Milner 多态签名：对无具体类型参数的函数生成 (params → ret) 签名，
//     供调用点实例化，实现跨函数类型推导（如 make(x) 返回 Record{val:x的类型}）。
//
// 【生命周期】
//   每个 Analyze 调用独立使用一个逻辑上的"分析实例"。调用开始时：
//     - hm_arena_ 被 Reset（所有之前分配的 Type* 指针失效）
//     - hm_table_ 被 Reset（HM 变量表清空）
//     - cur_param_hm_vars_ 清空
//   TypeInferencer 保证在 Analyze 前清理上一轮写入 FuncSummary 的悬空 HM 指针。
// ══════════════════════════════════════════════════════════════════════════════

#include "compile/cfg.h"
#include "compile/compile_common.h"
#include "compile/hm_type.h"
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

    // ── 构造 ────────────────────────────────────────────────────────────────
    // 传入全局 shape 注册表。registry 在分析过程中会不断插入新 shape（例如
    // table 构造子推导出的 record shape、shape meet 产生的新 shape 等）。
    UnifiedTypeAnalyzer(ShapeRegistry *registry) : registry_(registry) {
    }

    // ════════════════════════════════════════════════════════════════════════
    //  Public 方法 — 主分析入口与后处理阶段
    // ════════════════════════════════════════════════════════════════════════

    // ── 主分析入口 ──────────────────────────────────────────────────────
    // 对单个函数做一次完整的流敏感类型推导 + 逃逸分析。
    //
    // 调用时机：由 TypeInferencer::RunSSAAnalysis 在 CFG/SSA 构建完成后调用，
    // 每个用户函数和顶层 chunk 各一次。
    //
    // 完成的工作：
    //   1. 重置每函数状态（arena / HM 表 / 参数映射）
    //   2. 为每个形参创建 HM 类型变量（用于多态签名）
    //   3. 构建变量名 → 最新版本号的映射 (name_ver)
    //   4. 跑流敏感工作表（RunWorklist），得到每个块的 out_env
    //   5. φ 节点类型推导：从各前驱 out_env 取类型后 Meet 合并
    //   6. 逐块 TransferStmtConst 做精确类型传播并填充 ir.main_ssa_types
    //   7. 全函数 WalkSyntaxTree，推导所有 AST 子节点的类型
    //   8. 调用 ComputeVarFinalShapes / ComputeCtorTargetShapes
    //   9. 调用 ComputeEscape
    void Analyze(const std::string &func_name, const SyntaxTreeInterfacePtr &func_block, const CFGFunction &cfg, SSAFunction &ssa, InferResult &ir);

    // ── Phase 3a: 计算每个变量的 final shape ────────────────────────────
    // 对每个变量，取它所有 SSA 版本的类型做 Meet，得到"最宽" shape 写入
    // ir.var_final_shapes。这是 CGen 生成"变量声明用 struct 类型"的主要依据。
    //
    // 调用时机：在 Analyze 内部自动调用（尾部）；外部无需重复调用。
    void ComputeVarFinalShapes(const SSAFunction &ssa, InferResult &ir);

    // ── Phase 3a: 把 final shape 反向绑定到 table constructor AST 节点 ─
    // 遍历函数体 AST，找到 `local x = { ... }` / `x = { ... }` 这样的赋值，
    // 把等号右侧的 TableConstructor 节点映射到左侧变量 x 的 final shape_id，
    // 写入 ir.ctor_target_shapes。CGen 据此知道每个字面量表应该用哪个
    // C struct 来分配。
    //
    // 调用时机：由 RunSSAAnalysis 在 Analyze 之后调用（此时 var_final_shapes 已就绪）。
    void ComputeCtorTargetShapes(const SyntaxTreeInterfacePtr &func_block, const SSAFunction &ssa, InferResult &ir);

    // ── §7 函数摘要 ────────────────────────────────────────────────────
    // 基于当前所在函数的所有推导结果，填充 ir.func_summaries[func_name]：
    //   - param_types：每个形参的 SSA 类型（来自 version_types）
    //   - ret_type：所有 return 语句返回类型的 meet
    //   - param_escape：每个形参是否逃逸（从 ir.escape_vars 读取）
    //   - HM 多态签名：当 T_DYNAMIC 形参存在时生成 (params → ret) 签名
    //
    // 调用时机：由 RunSSAAnalysis 在 Analyze + ComputeCtorTargetShapes 之后调用。
    // 顶层 chunk 不调用（它没有可复用的摘要）。
    void BuildSummary(const std::string &func_name, const SyntaxTreeInterfacePtr &func_block, const SSAFunction &ssa, const CFGFunction &cfg, const std::unordered_map<int, SSATypeInfo> &version_types,
                      InferResult &ir);

private:
public:
    // ════════════════════════════════════════════════════════════════════════
    //  环境（Env）数据结构定义
    // ════════════════════════════════════════════════════════════════════════
    //
    // UTA 在分析过程中维护若干"环境"映射，每种环境承载不同维度的分析信息：

    // ── VarEnv ─────────────────────────────────────────────────────────
    // 变量名 → 类型信息 的映射。这是工作表在基本块出入口维护的核心数据。
    //
    // 语义：在当前基本块的"当前程序点"，已知每个变量（Lua 变量名）对应的
    // 推导类型（InferredType + shape_id）。它是"流敏感"的直接载体——
    // 沿块内语句向前推进时不断随赋值/声明更新。
    //
    // 生命周期：每个基本块各有一个 in/out VarEnv；块间通过 MeetEnv 合并。
    // 示例：{ "x" => {T_INT, -1}, "t" => {T_RECORD, 3} }
    using VarEnv = std::unordered_map<std::string, SSATypeInfo>;

    // ── ConstEnv ───────────────────────────────────────────────────────
    // 变量名 → 编译期已知常量值（字符串形式）的映射。实现 §12.6 常量传播。
    //
    // 语义：当一条语句给某变量赋值为字面量（kString 或 kNumber）时，
    // 该变量进入 ConstEnv，值为该字面量的原始字符串表示。后续遇到
    // `a[key]` 且 key 是变量时，查 ConstEnv 可以把 key 解析为具体字段名，
    // 从而在闭 record 上也能命中字段偏移访问。
    //
    // 更新策略：仅当右侧是字面量时写入；右侧为变量/表达式时删除该键
    // （不再确定是常量）。生命周期与 VarEnv 同步，在 TransferStmtConst
    // 中一起流转。
    //
    // 示例：{ "K" => "protocol" } 表示变量 K 编译期已知等于 "protocol"
    using ConstEnv = std::unordered_map<std::string, std::string>;

    // ── TypeEnv ────────────────────────────────────────────────────────
    // SSA 版本号 → 类型信息 的映射。维护"每个 SSA 版本当前推导出的类型"。
    //
    // 语义：SSA 的一个核心不变量是"每个版本只赋值一次"，因此一个版本号
    // 对应唯一推导类型。TypeEnv 是 SSA 版本到类型的"全局"映射。
    // 在 RunWorklist 结束时它与 ssa.version_types / ir.ssa_version_types 合并。
    //
    // 示例：{ 5 => {T_INT, -1}, 8 => {T_RECORD, 3} }
    //       表示版本 5 对应 int、版本 8 对应 shape 3 的 record。
    using TypeEnv = std::unordered_map<int, SSATypeInfo>;

    // ── VarNameToVersion ───────────────────────────────────────────────
    // 变量名 → 最新版本号 的映射。
    //
    // 语义：给定变量名，快速查到它在 SSA 中的"当前最新版本"。
    // 由 BuildVarNameVersionMap 基于 ssa.var_all_versions（每个变量
    // 所有版本号的有序列表）构建，总是取最后一个（最新）。
    //
    // 用途：InferExprType 在推导变量表达式类型时作为 fallback 查找——
    // 当 local_env 和 ssa.use_versions 都无法确定版本时使用最新版本。
    //
    // 示例：{ "x" => 7, "t" => 12 }
    using VarNameToVersion = std::unordered_map<std::string, int>;

    // ── EscapeEnv ──────────────────────────────────────────────────────
    // 变量名 → 是否逃逸 的映射。承载单函数的逃逸分析结果。
    //
    // 语义：逃逸 = 变量的引用"离开了当前函数的作用域"，例如：
    //   - 被传给另一个函数作实参（可能在被调函数中存活）
    //   - 被 return 到调用者
    //   - 被写入一个本身是逃逸的变量
    // 逃逸的变量不能安全地栈分配，必须走堆 / 闭包 upvalue。
    //
    // 值含义：true 表示逃逸，false / 不存在 表示不逃逸（保守默认 false）。
    //
    // 生命周期：在 ComputeEscape 中先初始化为全部 false，再通过 EscapeTransfer
    // 遍历语句对逃逸场景标记 true。最终写入 ir.escape_vars[func_name]。
    //
    // 示例：{ "x" => false, "t" => true }  表示 t 逃逸、x 不逃逸。
    using EscapeEnv = std::unordered_map<std::string, bool>;

private:
    // ════════════════════════════════════════════════════════════════════════
    //  Private 方法 — 工作表、推导、辅助
    // ════════════════════════════════════════════════════════════════════════

    // ── 工作表主循环（§6）──────────────────────────────────────────────
    // 在 CFG 上做"反向后序（RPO）+ 工作表"的流分析迭代，直到不动点。
    //
    // 返回：每个块的 out_env（BlockId → VarEnv）。
    //
    // 算法要点：
    //   · 单块情况直接 TransferStmtConst 跑一遍，无需求解。
    //   · 多块情况：
    //     1) 从 entry 开始 DFS 得到逆后序 rpo_order；
    //     2) entry 块 in_env seed 为 {param → T_DYNAMIC}；
    //     3) 工作表按 RPO 反复取块——合并各前驱 out_env → in_env，
    //        走块内所有 stmt → out_env；out 变化时把后继入表。
    //     4) 超过 kWidenIter 轮后启用 Widen，把不收敛的 record shape
    //        字段集截断以强制保证收敛；
    //     5) 最多 kMaxIters=64 轮。
    std::unordered_map<int, VarEnv> RunWorklist(const CFGFunction &cfg, const SSAFunction &ssa, const InferResult &ir);

    // ── 构建辅助 ────────────────────────────────────────────────────────
    // 从 SSAFunction.var_all_versions（变量名 → 所有版本号的列表）生成
    // VarNameToVersion（变量名 → 最新版本号）。总是取列表最后一个元素。
    VarNameToVersion BuildVarNameVersionMap(const SSAFunction &ssa);

    // ── 递归推导表达式类型 ──────────────────────────────────────────────
    // 对 AST 表达式节点做递归类型推导。这是类型系统对 AST 节点的"求值器"。
    //
    // 支持的表达式种类：
    //   · 字面量（number / string / nil / true/false）
    //   · 二元/一元运算（递归-Meet 子表达式类型）
    //   · 变量引用（优先 local_env，再 ssa.use_versions，再 name_ver fallback）
    //   · 字段读取 a.b / a[k]（§5.2.3，支持常量传播解析 a[k]）
    //   · table constructor（调用 BuildShapeFromCtor）
    //   · 函数调用（§7 过程间：查 FuncSummary 或实例化 HM 多态签名）
    //
    // 参数说明：
    //   · local_env：当前块的 VarEnv，变量查找的第一优先级。
    //   · const_env：常量传播环境，解析 a[k] 时查。
    //
    // 返回该表达式对应的 SSATypeInfo。
    SSATypeInfo InferExprType(const SyntaxTreeInterfacePtr &expr, const SSAFunction &ssa, const TypeEnv &version_types, const InferResult &ir, const VarNameToVersion &name_ver,
                              const VarEnv *local_env = nullptr, const ConstEnv *const_env = nullptr);

    // ── table constructor → shape ───────────────────────────────────────
    // 从 TableConstructor AST 节点提取字段列表，构造一个 ShapeType 并
    // 注册到 ShapeRegistry，返回 shape_id（失败返回 -1）。
    // 支持三种字段：kObject（name=value）、kArray（无显式键）、kExpr（[key]=value）。
    int BuildShapeFromCtor(const SyntaxTreeInterfacePtr &tc, const SSAFunction &ssa, const TypeEnv &version_types, const InferResult &ir);

    // ── 语句 env 转移（无常量传播）──────────────────────────────────────
    // 对单条语句执行 env 转移——从 in_env 出发推导出 out_env。
    // 仅处理能直接推导出变量类型的语句：LocalVar / Assign / ForLoop / ForIn。
    // 控制流语句（If / While / For）由 CFG 在外部基本块分裂时处理，不在此递归。
    //
    // 注意：此版本不维护 ConstEnv。在工作表迭代中调用（不需要常量传播）。
    VarEnv TransferStmt(const SyntaxTreeInterfacePtr &stmt, const VarEnv &env, const SSAFunction &ssa, const TypeEnv &version_types);

    // ── 带常量传播的 env 转移（§12.6 常量传播）────────────────────────
    // 与 TransferStmt 类似，但额外维护 ConstEnv：
    //   · 对 `local x = "literal"` 或 `x = 123` 这样的语句，把字面量写进 const_env；
    //   · 对 `x = <非字面量表达式>` 从 const_env 中删除 x（不再确定是常量）。
    // 该版本在 Analyze 第二轮精确类型传播（PopulateNodeTypesFromStmts）中被使用，
    // 因为字段访问 a[x] 需要常量 env 做字段解析。
    VarEnv TransferStmtConst(const SyntaxTreeInterfacePtr &stmt, const VarEnv &env, ConstEnv &const_env, const SSAFunction &ssa, const TypeEnv &version_types);

    // ── 从表达式提取字面量常量值 ──────────────────────────────────────────
    // 尝试从表达式节点提取编译期已知的字面量 string/number 值。
    //   成功 → value_out 被填充，返回 true；失败返回 false。
    // 是"常量传播"的入口判断——只有字面量来源的表达式才能进入 ConstEnv。
    // 静态方法，不依赖 UTA 实例状态。
    static bool ExtractLiteral(const SyntaxTreeInterfacePtr &exp, std::string &value_out);

    // ── 用 ConstEnv 解析 key 表达式为常量字符串 ────────────────────────
    // 对 a[key] 中的 key 表达式：先尝试字面量直接提取，失败则当 key 是变量时
    // 查 const_env。这是 §12.6 的核心——把 `a[key]` 解析为具体的字段名。
    // 静态方法，可被 InferExprType 中的 kSquare 分支调用。
    static bool ResolveKeyConstant(const SyntaxTreeInterfacePtr &key_expr, const ConstEnv &const_env, std::string &key_out);

    // ── 按块填充 AST 子节点的推导类型 ──────────────────────────────────
    // 给定块的 out_env（含类型 + 常量传播 env），遍历块内所有顶层语句，
    // 对每个 Exp / Var / PrefixExp / TableConstructor / FunctionCall
    // 节点调用 InferExprType，将结果写入 ir.main_ssa_types。
    // 可选 out_map 用于写入其他目标（测试/诊断用），默认写 ir.main_ssa_types。
    // 特殊处理 ForLoop 本身节点（写 cursor 类型给 CGen）。
    void PopulateNodeTypesFromStmts(const std::vector<SyntaxTreeInterfacePtr> &stmts, const VarEnv &env, const ConstEnv &const_env, const SSAFunction &ssa, const TypeEnv &version_types,
                                    InferResult &ir, std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> *out_map = nullptr);

    // ── Meet 操作 ────────────────────────────────────────────────────────
    // MeetEnv：对两个 VarEnv 按变量名逐一 Meet。
    //   在 φ 节点合并时调用：out_envs[pred] 的 meet 得到当前块 in_env。
    //
    // Meet：两个 SSATypeInfo 的 meet——
    //   · 类型部分用 ShapeRegistry::MeetType（T_DYNAMIC 吸收一切等）
    //   · shape_id 部分：两者相同保留；不同则调 registry_->Meet 合并 shape
    //     （registry 为 nullptr 时退化回 T_DYNAMIC）。
    // 两个都是 static 方法，便于在不含 registry 上下文的代码位置调用 MeetEnv。
    static VarEnv MeetEnv(const VarEnv &a, const VarEnv &b);
    static SSATypeInfo Meet(const SSATypeInfo &a, const SSATypeInfo &b, ShapeRegistry *reg = nullptr);

    // ── Phase 3a: 反向绑定 ctor → target shape ────────────────────────
    // 递归遍历 AST：对 `local x = { ... }` / `x = { ... }` 这样的赋值，
    // 把等号右侧 TableConstructor 节点映射到左侧变量 x 在 var_final_shapes
    // 中的 shape_id，写入 ctor_target_shapes。被 ComputeCtorTargetShapes 调用。
    static void LinkExprToTargetShape(const SyntaxTreeInterfacePtr &node, const std::string &target_name, const std::unordered_map<std::string, int> &var_final_shapes,
                                      std::unordered_map<const SyntaxTreeInterface *, int> &ctor_target_shapes);

    // ── §8 逃逸分析 ────────────────────────────────────────────────────
    // 计算当前函数各变量的逃逸状态。算法：
    //   1. 初始标记所有 local 变量为不逃逸；
    //   2. WalkSyntaxTree 遍历，对每一条"叶子"语句调用 EscapeTransfer；
    //   3. EscapeTransfer 按语句类型做逃逸判定（见 cpp 实现）。
    // 结果写入 escape_env（会进一步存到 ir.escape_vars[func_name]）。
    void ComputeEscape(const std::string &func_name, const SyntaxTreeInterfacePtr &func_block, const CFGFunction &cfg, EscapeEnv &escape_env);

    // ── 单条语句的逃逸转移 ──────────────────────────────────────────────
    // 语句级逃逸判定规则（规范 §8.1）：
    //   · FunctionCall：所有参数中的变量引用标记逃逸（保守：即使有摘要也标记）
    //   · Return：返回值中的变量引用标记逃逸
    //   · Assign：当 LHS 是 dynamic 变量时，RHS 中的变量引用标记逃逸
    //   · 其它语句：不做特殊处理
    // type_env 用于判断 LHS 变量是否为 T_DYNAMIC。
    void EscapeTransfer(const SyntaxTreeInterfacePtr &stmt, EscapeEnv &escape_env, const VarEnv &type_env);

    // ── HM 签名构建 ────────────────────────────────────────────────────────
    // 为当前函数生成 (params → ret) 的多态签名并写入 summary。
    // 策略：为每个参数创建一个新的 HM 类型变量（多态）；ret_hm_type 则
    // 基于推导的 ret_type 构造，其中 T_DYNAMIC 字段引用参数多态变量。
    // 这使得调用点可以用实参类型实例化，得到更具体的返回类型。
    void BuildHmSignature(FuncSummary &s, const CFGFunction &cfg, const SSATypeInfo &ret_type);

    // ── HM 签名实例化 ──────────────────────────────────────────────────
    // 在调用点把形参类型变量（按指针相等）替换为实参的具体类型，返回克隆的
    // 返回类型表达式。调用 ok=false 表示签名不可用（ret_hm_type 为空）。
    // 这是 §12 多态在调用点落实的关键步骤。
    Type *InstantiateHmSignature(const std::vector<Type *> &param_hm_types, Type *ret_hm_type, const std::vector<Type *> &arg_hm_types, bool &ok);

    // ── 形参变量替换（HM）─────────────────────────────────────────────
    // 克隆 HM 类型 t，把其中按指针出现在 from 中的形参变量逐一替换为 to 中
    // 对应位置的实参类型。纯函数：不修改原始 t，总是从 hm_arena_ 分配新 Type。
    Type *SubstituteHmVars(Type *t, const std::vector<Type *> &from, const std::vector<Type *> &to, bool &ok);

    // ════════════════════════════════════════════════════════════════════════
    //  成员变量 — 状态与外部依赖
    // ════════════════════════════════════════════════════════════════════════

    // ── HM unification support ──────────────────────────────────────────────
    // bump 分配的 arena，所有 HM Type 节点在其上分配。每个 Analyze 调用开始时
    // 整体 Reset，因此上一轮分配的指针在本轮全部失效（TypeInferencer 会
    // 负责清理 FuncSummary 中的悬空 HM 指针后再调用 Analyze）。
    TypeArena hm_arena_;

    // HM 变量表：管理 TY_VAR 的创建 / 绑定 / 统一。绑定关系也在 arena 上维护。
    // 同样在每个 Analyze 调用开始时 Reset。
    TypeVarTable hm_table_{hm_arena_};

    // 当前函数形参名 → HM 类型变量的映射。在 Analyze 开始时由
    // cur_param_hm_vars_[pname] = hm_table_.NewVar() 逐参数创建，
    // 供 BuildSummary 构造多态签名使用。每函数独立，每次 Analyze 清空。
    std::unordered_map<std::string, Type *> cur_param_hm_vars_;

    // ── 内部 HM 辅助（在 Analyze 的首次 InferExprType 调用时惰性推断）────
    // 把编译时 SSATypeInfo + 可选 shape 转为 HM Type*（不绑定变量）。
    // 例如 {T_RECORD, shape_id} 转为引用具体字段结构的 record Type。
    Type *SsaInfoToHm(const SSATypeInfo &info);

    // 把 HM Type* 回转为 SSATypeInfo：
    // 已绑定的变量取其绑定值；未绑定的自由变量回退为 T_DYNAMIC
    // （因为自由变量代表"未知类型"，无法精确表达为 SSATypeInfo）。
    SSATypeInfo HmToSsaInfo(Type *t);

    // 把一个 HM record/record_open 类型注入 ShapeRegistry，返回 shape_id。
    // 失败（非 record 类型 / registry 为空）时返回 -1。
    // 用于把多态签名在实例化后落到 shape 体系做 C 结构布局。
    int InjectHmRecordIntoRegistry(Type *record_ty);

    // 全局 shape 注册表指针——贯穿整个 InferResult 生命周期的"shape 真相源"。
    // 在 Analyze 开头由 ir.shape_registry 赋值（不存在则新建）。
    ShapeRegistry *registry_ = nullptr;

    // 当前分析的函数名（调试 / 日志用）。
    std::string cur_func_name_;

    // 预留位掩码：未来做"按调用点特化"时使用。当前始终为 -1。
    int cur_bitmask_ = -1;
};

}// namespace fakelua
