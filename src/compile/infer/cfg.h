#pragma once

#include "compile/syntax_tree.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fakelua {

// ─── 基本块 (Basic Block) ────────────────────────────────────────────────────
//
// 基本块是 CFG 的核心单元：一段顺序执行的语句序列，入口在块首、出口在块尾，
// 块内没有分支进入或退出（只有最后一条指令可能产生分支）。
//
// 为什么用 vector 存 id 而不是指针？
// - 块的生命周期由 CFGFunction::blocks 统一管理，id 在块重排/删除时稳定。
// - 线性扫描查找（FindBlock）在 Lua 子集这种块数量通常 < 100 的场景下足够快，
//   且避免了指针悬挂风险。
struct BasicBlock {
    int id = -1;                              // 全局唯一块编号（从 0 递增）
    std::vector<SyntaxTreeInterfacePtr> stmts;// 块内的语句序列（顺序执行）
    std::vector<int> pred_ids;                // 前驱块 id 列表（入边来源）
    std::vector<int> succ_ids;                // 后继块 id 列表（出边目标）
};

// ─── 单个函数的控制流图 (CFG) ────────────────────────────────────────────────
//
// CFGFunction 表示一个函数的控制流图，包含：
// - 所有基本块（blocks）
// - 入口块（entry_id）和出口块列表（exit_ids）
// - 支配关系（dominators）和支配边界（dominance_frontier），用于后续 SSA 构造
//
// 为什么需要 exit_ids 是 vector 而不是单个 exit？
// - 理论上函数可能有多个 return 点，每个 return 都连到 exit 块。
// - 当前实现中只有一个 exit 块，但保留 vector 为未来扩展留余地。
struct CFGFunction {
    std::string func_name;                             // 函数名（用于调试和 Dump）
    std::unordered_map<std::string, int> param_indices;// 参数名 -> 参数位置索引
    std::vector<BasicBlock> blocks;                    // 所有基本块（拥有所有权）
    int entry_id = -1;                                 // 入口块 id（函数起始）
    std::vector<int> exit_ids;                         // 出口块 id 列表（函数返回）

    // 按 id 查找块（线性扫描，通常块数量很少）
    // 返回 const 指针，不暴露内部可修改性
    [[nodiscard]] const BasicBlock *FindBlock(int id) const {
        for (const auto &b: blocks)
            if (b.id == id) return &b;
        return nullptr;
    }

    // 非 const 版本，供内部修改块内容使用
    [[nodiscard]] BasicBlock *FindBlock(int id) {
        for (auto &b: blocks)
            if (b.id == id) return &b;
        return nullptr;
    }

    // ─── 支配关系（Dominators）──────────────────────────────────────────────
    // dominators[b] = 块 b 的所有支配者集合
    // 定义：块 d 支配块 b，当且仅当从 entry 到 b 的每条路径都经过 d。
    // 性质：支配关系是自反的（每个块支配自身）、传递的。
    std::unordered_map<int, std::unordered_set<int>> dominators;

    // ─── 支配边界（Dominance Frontier）─────────────────────────────────────
    // dominance_frontier[n] = 块 n 的支配边界集合
    // 定义：DF(n) = { y | ∃p ∈ pred(y), n 支配 p 但 n 不严格支配 y }
    // 用途：SSA 构造中，需要在 DF(n) 中的每个块为 n 中定义的变量插入 φ 函数。
    std::unordered_map<int, std::unordered_set<int>> dominance_frontier;

    // 序列化为可读字符串（用于测试验证和调试输出）
    [[nodiscard]] std::string DumpToString() const;
};

// ─── CFG 构建器 ─────────────────────────────────────────────────────────────
//
// CFGBuilder 将 AST 形式的函数体（SyntaxTreeBlock）转换为 CFG（控制流图）。
//
// 核心设计思想：
// 1. 递归下降遍历 AST，遇到控制流语句（if/while/for/repeat）时分裂基本块。
// 2. 使用"汇合块（merge_block）"模式处理分支合并：
//    - 在构建 if/elseif/else 链之前，先创建 merge_block。
//    - 将 merge_block 作为 exit_block 传给每个分支的 BuildBlock 调用。
//    - 这样分支内部的 return 或末尾自然连接到 merge_block。
//    - 最后将各分支的尾块显式连到 merge_block。
// 3. 循环结构（while/for/repeat）通过回边（back edge）表示迭代。
//
// 为什么先创建 merge_block 再构建分支？
// - 分支内部可能嵌套控制流，需要知道"外部汇合点"才能正确连接。
// - 提前创建 merge_block 使得递归构建时各分支共享同一个汇合目标。
class CFGBuilder {
public:
    // 构建函数 block 的 CFG
    // 参数：
    //   func_block - 函数体的 AST 节点（SyntaxTreeBlock）
    //   params     - 函数参数名列表
    //   func_name  - 函数名
    //   is_vararg  - 是否可变参数（当前未使用，保留供扩展）
    CFGFunction Build(const SyntaxTreeInterfacePtr &func_block, const std::vector<std::string> &params, const std::string &func_name, bool is_vararg);

private:
    CFGFunction cfg_;      // 当前正在构建的 CFG
    int next_block_id_ = 0;// 下一个可用的块 id（单调递增）

    // 创建一个新基本块，返回其 id
    int NewBlock();
    // 按 id 获取块引用（未找到则触发 DEBUG_ASSERT）
    BasicBlock &GetBlock(int id);
    // 添加一条从 from 到 to 的有向边（同时更新 succ 和 pred，去重）
    void AddEdge(int from, int to);

    // ─── 递归构建入口 ──────────────────────────────────────────────────────

    // 构建函数体：从 entry 块开始，顺序处理 block 中的每条语句。
    // 返回尾块 id（不自动连接 exit，exit_block 仅传递给控制流语句使用）。
    // 设计意图：尾块由调用者决定是否连接到 exit，保持灵活性。
    int BuildBlock(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block);

    // 构建单条语句：可能分裂基本块（if/while/for/repeat）
    // 返回"当前块"（可能已被分裂，即新的当前块）
    //
    // 块分裂逻辑：
    // - 普通语句（赋值、调用等）：直接追加到 current_block，不分裂。
    // - 控制流语句：创建新块，将控制流节点放入条件/判断块，
    //   然后递归构建子结构，返回汇合块作为新的 current_block。
    int BuildStmt(const SyntaxTreeInterfacePtr &stmt, int current_block, int exit_block);

    // ─── 控制流语句构建 ────────────────────────────────────────────────────

    // 构建 if/elseif/else 链
    // 结构：
    //   cond_block ──true──> then_block ──> ... ──> then_tail ──> merge_block
    //           ──false──> elseif_cond ──true──> elseif_block ──> ... ──> merge_block
    //           ──false──> else_block ──> ... ──> merge_block
    //           ──fall-through（无 else 时）──> merge_block
    //
    // 汇合块（merge_block）设计：
    // - 在构建任何分支之前创建，作为所有分支的共同后继。
    // - 将 merge_block 作为 exit_block 传给 BuildBlock，使分支内层控制流能正确连接。
    // - 无 else 时，cond_block 直接 fall-through 到 merge_block（保持语义正确）。
    int BuildIf(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block);

    // 构建 while 循环
    // 结构：
    //   current_block ──> header（条件判断）──true──> body ──> body_tail ──> header（回边）
    //                                ──false──> exit_block
    // 返回 header 作为新的当前块（循环后的代码接在 header 之后通过 exit 连接）
    int BuildWhile(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block);

    // 构建 repeat-until 循环
    // 结构：
    //   current_block ──> body ──> body_tail ──> cond（条件判断）──false──> body（回边）
    //                                                   ──true──> exit_block
    // 注意：repeat 的条件在体之后判断，所以 cond 块在 body 之后创建。
    int BuildRepeat(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block);

    // 构建 for 循环（数值型）
    // 结构：
    //   current_block ──> init（start/end/step 求值）──> body ──> body_tail ──> init（回边）
    //                                               ──边界不满足──> exit_block
    int BuildForLoop(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block);

    // 构建 for-in 循环
    // 结构：
    //   current_block ──> init（迭代器设置）──> body ──> body_tail ──> init（回边）
    //                                        ──迭代结束──> exit_block
    int BuildForIn(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block);

    // ─── 支配关系计算 ──────────────────────────────────────────────────────

    // 计算每个块的支配集合（迭代数据流分析）
    // 算法：worklist 风格的迭代直到不动点
    // - 初始化：entry 的支配集 = {entry}，其他所有块的支配集 = 所有块
    // - 迭代：对每个非 entry 块 b，new_dom = ∩(dom[p] for p in pred(b)) ∪ {b}
    // - 终止：没有任何块的支配集发生变化
    void ComputeDominators();

    // 计算每个块的支配边界（Dominance Frontier）
    // 用途：SSA 构造中确定 φ 函数的插入位置
    // 算法：对每个块 n，遍历每个多前驱块 y，
    //       若 n 支配 y 的某个前驱但 n 不严格支配 y，则 y ∈ DF(n)
    void ComputeDominanceFrontier();
};

}// namespace fakelua
