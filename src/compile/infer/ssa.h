#pragma once

#include "compile/compile_common.h"
#include "compile/infer/cfg.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fakelua {

// ============================================================================
// SSA（Static Single Assignment，静态单赋值）形式
//
// 核心思想：每个变量在整个程序中只被赋值一次（每个"版本"唯一），
// 在控制流汇合点通过 φ 函数（phi function）选择不同前驱路径带来的
// 不同版本。这使得数据流分析更精确、更易于优化。
//
// 构造算法：Cytron et al., 1991 "Efficiently Computing Static Single
//                 Assignment Form and the Control Dependence Graph"
//
// 该算法分为两个阶段（phase）：
//
//   Phase 1 — φ 节点插入（Insertion of φ-functions）
//     1) 遍历所有基本块，收集每个变量在哪些块中被定义（def_blocks）。
//     2) 对每个变量 v，在 v 的定义块的"支配边界"（Dominance Frontier）
//        中插入 φ 节点。支配边界 DF(n) 表示从 n 出发"即将不再支配"的
//       块的集合——即控制流汇合点、不同来源的值可能在此相遇的地方。
//     3) 插入 φ 后，如果该块本身并未定义 v，则它仍然是一个"传播点"，
//        需要继续沿着它的支配边界再插入 φ（工作表循环自然完成这一传播）。
//
//   Phase 2 — 变量重命名（Renaming）
//     1) 按照支配树（Dominance Tree）进行 DFS，维护每个变量的"版本栈"
//        （stack_top[var_id] = 当前活跃的最新版本号）。
//     2) 进入块时，先为该块中所有 φ 的结果分配新版本，并压栈。
//     3) 处理块内语句：对每个"使用"（use）替换为当前栈顶版本；
//        对每个"定义"（def）分配新版本并压栈。
//     4) 离开块、要在后继块填写 φ 的参数时，依据栈顶把当前版本填入
//        φ.arg_versions[pred_index]。
//     5) 递归处理支配树子节点。
//
// 为什么沿支配边界传播 φ：
//   - 在块 B 定义（或传入了 φ 结果版本）的变量 v，其"活跃版本"会沿着
//     B 的支配范围一直传递到 B 所能支配的最后一个块。
//   - 在 DF(B) 中的块 Y，控制流不再被 B 独占：至少有一条路径到达 Y
//     不经过 B。因此在 B 中的定义可能与来自其他路径的值在 Y 相遇。
//   - 如果在 Y 中 v 还"活跃"，就必须在 Y 插入 φ 来显式合并。
//   - 插入 φ 后，Y 自身又成为一个新的 v 的"来源"（虽然是合并后的统一
//     版本），需要继续传播到 DF(Y)。工作表（worklist）保证这一点。
// ============================================================================


// ────────────────────────────────────────────────────────────────────────────
// φ 节点（Phi Node）
//
// 在 SSA 中，每个控制流汇合点（多个前驱的基本块）可能对同一变量持有
// 不同版本。φ 节点形如：
//
//     v_k = φ(v_i, v_j, v_m, ...)
//
// 表示：当沿着第 0 个前驱到达时取 v_i，沿第 1 个前驱到达时取 v_j，依
// 此类推。这样就把"多个可能的取值"统一为一个新的单赋值版本 v_k。
// ────────────────────────────────────────────────────────────────────────────
struct PhiNode {
    int var_id = -1;              // 变量索引（在 var_id_to_name_ 中的下标）
    std::string var_name;         // 变量名（调试 / dump 用，例如 "x"）
    int result_version = -1;      // 该 φ 产生的新版本号（形如 v_k）
    std::vector<int> arg_versions;// 各前驱块传入的版本号（按 pred_ids 顺序）
                                  // 意思是：arg_versions[i] 来自该块第 i 个前驱的版本
};

// ────────────────────────────────────────────────────────────────────────────
// SSAFunction：单个函数的 SSA 形式
//
// 存放一个函数在 SSA 化之后的所有信息，包括哪些块插入了 φ、每个语句
// 的 SSA 版本号、以及全局版本号到时变量名/块的反向索引。
// ────────────────────────────────────────────────────────────────────────────
struct SSAFunction {
    std::string func_name;          // 函数名（调试用）
    std::vector<int> param_versions;// 每个参数的初始 SSA 版本号
                                    // param_versions[i] 对应第 i 个形参的入口版本

    // block_id → 该块入口的 φ 节点列表
    // 注意：一个块中可能为多个变量都插入了 φ
    std::unordered_map<int, std::vector<PhiNode>> block_phis;

    // AST 节点 → 定义的版本号
    // 指向某个 LocalVar / Assign / ForLoop / ForIn 语句的指针被映射到
    // 该语句所"定义"得到的最新版本号。
    std::unordered_map<const SyntaxTreeInterface *, int> def_versions;

    // AST 节点 → 使用的版本号列表
    // 指向某个语句的指针被映射到该语句中"使用的各个变量"对应的版本号
    // （顺序与出现于表达式树中的顺序一致，便于后续代码生成时替换）。
    std::unordered_map<const SyntaxTreeInterface *, std::vector<int>> use_versions;

    // 变量名 → 该变量所有 SSA 版本号的全集（按分配顺序）
    // 这是生成与维护 SSA 的主要线索之一。
    std::unordered_map<std::string, std::vector<int>> var_all_versions;

    // 版本号 → 定义该版本的基本块 id
    // 用于判断某个版本"在哪里诞生"，对后续的 use-def 链构建很关键。
    std::unordered_map<int, int> version_to_block;

    // 版本号 → 变量名
    // 反向索引：给出一个版本号，知道它属于哪个变量。
    std::unordered_map<int, std::string> version_to_name;

    // 版本号 → 类型信息（φ 类型推导 / 统一类型分析阶段填充）
    std::unordered_map<int, SSATypeInfo> version_types;

    int next_version = 0;// 下一个待分配的全局版本号（单调递增）

    // 序列化为可读字符串（用于测试验证）
    [[nodiscard]] std::string DumpToString() const;
};

// ────────────────────────────────────────────────────────────────────────────
// SSA构建器
//
// 输入：一棵 CFG（控制流图）形式的函数；
// 输出：该函数的 SSA 形式（SSAFunction）。
//
// 内部数据结构：
//   var_name_to_id_   : 变量名 → 整型 id（压缩变量名为下标，便于用 vector 索引）
//   var_id_to_name_   : id → 变量名（反向映射）
//   var_def_blocks_   : 每个变量被定义到的所有基本块 id 集合
//   dom_tree_children_: 支配树的子节点邻接表（entry_id 为根）
//
// 重命名阶段的关键：
//   stack_top[var_id] 始终指向当前变量 var_id 的"当前活跃版本"。
//   进入块 → 处理 φ（分配新版本、压栈）；
//          → 处理语句（use 替换为栈顶、def 分配新版本并压栈）；
//   离开块 → 在后继块的 φ 参数中填入当前栈 top。
//   这样每个版本都被精确地关联到"它的定义所能到达的所有使用"。
// ────────────────────────────────────────────────────────────────────────────
class SSABuilder {
public:
    // 主入口：把 CFGFunction 转为 SSAFunction
    SSAFunction Build(const CFGFunction &cfg);

private:
    SSAFunction ssa_;     // 本次构建的结果
    int next_version_ = 0;// 冗余保留字段（实际以 ssa_.next_version 为准）

    // 变量名 → 变量索引（压缩为连续 int）
    std::unordered_map<std::string, int> var_name_to_id_;
    // 变量索引 → 变量名
    std::vector<std::string> var_id_to_name_;

    // 变量索引 → 定义该变量的基本块集合
    // var_def_blocks_[vid] 给出定义变量 vid 的所有块
    std::vector<std::unordered_set<int>> var_def_blocks_;

    // 支配树子节点（idom 树）
    // dom_tree_children_[b] = b 的直接支配子节点列表
    std::unordered_map<int, std::vector<int>> dom_tree_children_;

    // 把变量名映射到唯一的整型 id（不存在则创建）
    int GetVarId(const std::string &name);

    // 为某变量在指定块中分配一个新版本号（单调递增）
    //   内部会更新：
    //     ssa_.var_all_versions[var_name]  —— 追加新 ver
    //     ssa_.version_to_block[ver]       —— ver 诞生于 block_id
    //     ssa_.version_to_name[ver]        —— ver 属于 var_name
    //     ssa_.next_version                —— 递增
    int NewVersion(const std::string &var_name, int block_id);

    // ── Step 0: 遍历 CFG，收集每个块中定义的所有变量 ─────────────────────
    // 这一步只填充 var_def_blocks_，为后续 φ 插入阶段提供"种子"：每个在
    // 块中被赋值的变量，都会把自己的块加入 var_def_blocks_[vid]。
    void CollectDefBlocks(const CFGFunction &cfg);

    // ── 构建支配树（从支配关系推导） ────────────────────────────────────────
    // CFG 提供的是"每个块被哪些块支配"（dominators map）。
    // 我们借此推导"直接支配者"（immediate dominator, idom）：
    //   块 b 的直接支配者是严格支配 b 的块中，不被其他任何严格支配者
    //   支配的那个块。
    // 通过 idom 得到支配树，主要用于 Phase 2 重命名时做 DFS。
    void BuildDomTree(const CFGFunction &cfg);

    // ── Cytron 第一阶段：φ 节点插入 ────────────────────────────────────────
    // 核心过程：
    //   for each variable v:
    //     worklist ← def(v)                // 初始：所有定义 v 的块
    //     while worklist ≠ ∅:
    //       n ← pop(worklist)
    //       for each y ∈ DF(n):            // n 的支配边界
    //         if y 尚未有 φ for v:
    //           在 y 插入 φ_v
    //           if y ∉ def(v):
    //             worklist ← worklist ∪ {y}
    // 要点：
    //   - 插入 φ 后，即使 y 本身没有定义 v，它也成为 v 的一个新"来源"
    //    （合并了来自各前驱的版本），所以继续沿 DF(y) 传播。
    //   - 这样保证：任何"两股不同路径的 v 可能相遇"的汇合点，都配有 φ。
    void InsertPhis(const CFGFunction &cfg);

    // ── Cytron 第二阶段：变量重命名 ────────────────────────────────────────
    // 按照支配树进行 DFS，利用 stack_top 记录每个变量的当前活跃版本：
    //
    //   rename_dfs(b):
    //     // 1) 先处理 b 块中的所有 φ：给每个 φ 分配新版本并更新栈顶
    //     for each phi in block_phis[b]:
    //         v' = NewVersion(phi.var_name, b)
    //         phi.result_version = v'
    //         stack_top[phi.var_id] = v'
    //
    //     // 2) 处理 b 的块内语句
    //     rename_block_stmts(b)
    //
    //     // 3) 用当前栈顶回填所有后继块的 φ 参数
    //     for each successor s of b:
    //         pred_index = b 在 s->pred_ids 中的位置
    //         for each phi in block_phis[s]:
    //             phi.arg_versions[pred_index] = stack_top[phi.var_id]
    //
    //     // 4) 递归子节点
    //     for each c in dom_tree_children_[b]:
    //         rename_dfs(c)
    void RenameVariables(const CFGFunction &cfg);

    // 在单个块中处理语句的定义/使用
    //   参数：
    //     block_id   — 当前处理的基本块
    //     stmts      — 该块的语句列表
    //     stack_top  — 变量 id → 当前栈顶版本的映射（引用，会原地修改）
    void RenameBlockStmts(int block_id, const std::vector<SyntaxTreeInterfacePtr> &stmts, std::unordered_map<int, int> &stack_top);

    // 收集单个语句中定义的变量名（LHS）
    //   支持：LocalVar / Assign / ForLoop / ForIn
    static std::vector<std::string> GetDefNames(const SyntaxTreeInterfacePtr &stmt);

    // 收集单个语句中使用的变量名（RHS）
    //   递归进入表达式树（Exp / PrefixExp / FunctionCall / 表构造 等），
    //   但跳过该语句自身定义的左端（由调用方保证在 def 之前先筛除 lhs）。
    static std::vector<std::string> GetUseNames(const SyntaxTreeInterfacePtr &stmt);
};

}// namespace fakelua
