#include "compile/cfg.h"

namespace fakelua {

// ─── Build：CFG 构建入口 ─────────────────────────────────────────────────────
//
// 整体流程：
// 1. 初始化 CFG 状态（清空块列表、参数等）
// 2. 创建 entry 块（函数入口）和 exit 块（函数出口）
// 3. 递归构建函数体，从 entry 块开始
// 4. 将函数体尾块连接到 exit 块
// 5. 计算支配关系和支配边界（为后续 SSA 构造做准备）
//
// 为什么需要单独的 entry 和 exit 块？
// - entry 块：作为 CFG 的唯一起点，便于从入口开始遍历和分析。
// - exit 块：作为所有 return/函数末尾的汇聚点，统一出口便于分析。
// - 即使函数没有显式 return，函数体末尾也会隐式连接到 exit。
CFGFunction CFGBuilder::Build(const SyntaxTreeInterfacePtr &func_block, const std::vector<std::string> &params, const std::string &func_name, bool /*is_vararg*/) {
    // ─── 步骤 1：重置 CFG 状态 ──────────────────────────────────────────
    cfg_.func_name = func_name;
    cfg_.blocks.clear();
    cfg_.param_indices.clear();
    cfg_.entry_id = -1;
    cfg_.exit_ids.clear();
    next_block_id_ = 0;

    // ─── 记录参数名到索引的映射 ────────────────────────────────────────
    // 参数索引用于后续类型推导时区分参数和普通局部变量
    for (size_t i = 0; i < params.size(); ++i) {
        cfg_.param_indices[params[i]] = static_cast<int>(i);
    }

    // ─── 步骤 2：创建 entry 和 exit 块 ─────────────────────────────────
    int entry = NewBlock();
    int exit = NewBlock();
    cfg_.entry_id = entry;
    cfg_.exit_ids.push_back(exit);

    // ─── 步骤 3：递归构建函数体 ────────────────────────────────────────
    // BuildBlock 返回函数体的"尾块" id（即最后一条语句所在的块）
    int tail = BuildBlock(func_block, entry, exit);

    // ─── 步骤 4：连接尾块到 exit ───────────────────────────────────────
    // 函数体末尾隐式连接到 exit 块（模拟函数返回）
    AddEdge(tail, exit);

    // ─── 步骤 5：计算支配关系 ──────────────────────────────────────────
    // 支配关系是 SSA 构造的基础，需要在 CFG 构建完成后立即计算
    ComputeDominators();
    ComputeDominanceFrontier();

    return std::move(cfg_);
}

// ─── NewBlock：创建新基本块 ─────────────────────────────────────────────────
//
// 每次调用分配一个递增的唯一 id，并将新块加入 cfg_.blocks。
// id 从 0 开始单调递增，保证块编号稳定且可预测。
int CFGBuilder::NewBlock() {
    int id = next_block_id_++;
    cfg_.blocks.push_back({id, {}, {}, {}});
    return id;
}

// ─── GetBlock：按 id 获取块引用 ─────────────────────────────────────────────
//
// 使用线性扫描而非 map，因为：
// - Lua 子集函数的块数量通常很少（< 100）
// - 避免了额外的内存开销和哈希计算
// - 在 debug 模式下通过 DEBUG_ASSERT 确保 id 有效
BasicBlock &CFGBuilder::GetBlock(int id) {
    for (auto &b: cfg_.blocks)
        if (b.id == id) return b;
    DEBUG_ASSERT(false);
    return cfg_.blocks[0];
}

// ─── AddEdge：添加控制流边 ──────────────────────────────────────────────────
//
// 同时更新：
// - from 块的 succ_ids（出边）
// - to 块的 pred_ids（入边）
//
// 去重检查：避免重复添加同一条边（在复杂控制流中可能多次调用 AddEdge）
void CFGBuilder::AddEdge(int from, int to) {
    auto &fb = GetBlock(from);
    if (std::find(fb.succ_ids.begin(), fb.succ_ids.end(), to) == fb.succ_ids.end()) {
        fb.succ_ids.push_back(to);
        GetBlock(to).pred_ids.push_back(from);
    }
}

// ─── BuildBlock：构建语句块 ─────────────────────────────────────────────────
//
// 遍历 SyntaxTreeBlock 中的每条语句，依次调用 BuildStmt。
// 每次 BuildStmt 返回当前的"尾块" id，作为下一条语句的 current_block。
//
// 为什么返回 current_block 而不是直接修改？
// - 因为 BuildStmt 可能分裂块（遇到控制流语句时），新的当前块可能不同。
// - 通过返回值传递"当前块"，让调用者始终知道下一条语句应该追加到哪个块。
int CFGBuilder::BuildBlock(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    if (!node) return current_block;
    if (node->Type() != SyntaxTreeType::Block) return current_block;

    auto block_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
    int cur = current_block;
    for (auto &stmt: block_ptr->Stmts()) {
        // 每条语句构建后，cur 可能改变（控制流语句会分裂块）
        cur = BuildStmt(stmt, cur, exit_block);
    }
    return cur;
}

// ─── BuildStmt：构建单条语句（块分裂的核心逻辑） ────────────────────────────
//
// 这是 CFG 构建中"块分裂"的入口点。根据语句类型决定如何处理：
//
// 1. 控制流语句（If/While/Repeat/ForLoop/ForIn）：
//    - 调用对应的 Build* 函数
//    - 这些函数会创建新块、添加边、递归构建子结构
//    - 返回"汇合块"作为新的 current_block
//
// 2. 普通语句（赋值、函数调用、return 等）：
//    - 直接追加到 current_block 的 stmts 列表
//    - 不分裂块，返回 current_block 不变
//
// 块分裂的设计意义：
// - 控制流语句是基本块的"边界"，因为它们改变了程序的执行路径。
// - 普通语句在同一个基本块内顺序执行，不需要分裂。
// - 这种分裂保证了每个基本块内部是纯顺序代码，便于后续优化和分析。
int CFGBuilder::BuildStmt(const SyntaxTreeInterfacePtr &stmt, int current_block, int exit_block) {
    if (!stmt) return current_block;

    switch (stmt->Type()) {
        case SyntaxTreeType::If:
            return BuildIf(stmt, current_block, exit_block);
        case SyntaxTreeType::While:
            return BuildWhile(stmt, current_block, exit_block);
        case SyntaxTreeType::Repeat:
            return BuildRepeat(stmt, current_block, exit_block);
        case SyntaxTreeType::ForLoop:
            return BuildForLoop(stmt, current_block, exit_block);
        case SyntaxTreeType::ForIn:
            return BuildForIn(stmt, current_block, exit_block);
        default: {
            // 普通语句：追加到 current_block，不分裂
            GetBlock(current_block).stmts.push_back(stmt);
            return current_block;
        }
    }
}

// ─── BuildIf：构建 if/elseif/else 链 ────────────────────────────────────────
//
// 这是 CFG 构建中最复杂的函数，核心是"汇合块（merge_block）"模式。
//
// 汇合块设计（为什么先创建 merge_block？）：
// ─────────────────────────────────────────────
// 1. 在构建任何分支之前，先创建一个 merge_block。
// 2. 将 merge_block 作为 exit_block 传给每个分支的 BuildBlock 调用。
//    这样，分支内部嵌套的控制流（如 if 内有 while）的"出口"就是 merge_block。
// 3. 构建完成后，将各分支的尾块显式连接到 merge_block。
// 4. 如果没有 else 分支，cond_block 直接 fall-through 到 merge_block。
//
// 为什么这样设计？
// - 保证所有分支最终汇聚到同一个点，便于后续数据流分析。
// - 避免分支内部需要"知道"外部的汇合点，降低耦合。
// - 无 else 时的 fall-through 边确保语义正确（条件为假时跳过所有分支）。
//
// 生成的 CFG 结构（文字描述）：
//
// [cond_block] 根据条件判断分出多条边：
//   - 条件为真   → [then_block]           → (then_tail)     → [merge_block]
//   - 条件为假   → [elseif_cond]           → [elseif_block]  → (elseif_tail) → [merge_block]
//   - 条件为假   → [else_block]（如有）    → (else_tail)     → [merge_block]
//   - 无 else    → cond_block fall-through → [merge_block]
//
// 所有分支最终汇聚到 merge_block。
//
int CFGBuilder::BuildIf(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    auto ifn = std::dynamic_pointer_cast<SyntaxTreeIf>(node);

    // ─── 步骤 1：条件判断块 ────────────────────────────────────────────
    // current_block 成为条件判断块，将 if 节点放入其中
    int cond_block = current_block;
    GetBlock(cond_block).stmts.push_back(node);

    // ─── 步骤 2：先创建汇合块 ──────────────────────────────────────────
    // 关键：在构建分支之前创建 merge_block，使其可作为 exit_block 传递
    int merge_block = NewBlock();

    // 收集各分支的尾块（用于最后统一连接到汇合块）
    std::vector<int> branch_tails;
    bool has_full_coverage = true;// 是否有 else（全覆盖）

    // ─── 步骤 3：构建 then 分支 ────────────────────────────────────────
    int then_block = NewBlock();
    AddEdge(cond_block, then_block);// 条件为真 → then 分支
    // 将 merge_block 作为 exit_block 传递，使 then 分支内的控制流能正确连接
    int then_tail = BuildBlock(ifn->Block(), then_block, merge_block);
    branch_tails.push_back(then_tail);

    // ─── 步骤 4：构建 elseif 链 ────────────────────────────────────────
    // 每个 elseif 有自己的条件块，从 cond_block 引出边
    auto elseiflist = ifn->ElseIfs();
    if (elseiflist && elseiflist->Type() == SyntaxTreeType::ElseIfList) {
        auto elist = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseiflist);
        for (size_t i = 0; i < elist->ElseifSize(); ++i) {
            int ecblock = NewBlock();
            AddEdge(cond_block, ecblock);// 条件为假 → elseif 条件判断
            int tail = BuildBlock(elist->ElseifBlock(i), ecblock, merge_block);
            branch_tails.push_back(tail);
        }
    }

    // ─── 步骤 5：构建 else 分支 ────────────────────────────────────────
    if (auto eb = ifn->ElseBlock()) {
        int else_block = NewBlock();
        AddEdge(cond_block, else_block);
        int tail = BuildBlock(eb, else_block, merge_block);
        branch_tails.push_back(tail);
    } else {
        // 没有 else 时，条件可能为假直接跳过，需要 fall-through 边
        has_full_coverage = false;
    }

    // ─── 步骤 6：连接各分支尾块到汇合块 ────────────────────────────────
    for (int tail: branch_tails) {
        AddEdge(tail, merge_block);
    }
    // 如果没有 else，条件块 fall-through 到汇合块
    // 这保证了条件为假时控制流能正确继续
    if (!has_full_coverage) {
        AddEdge(cond_block, merge_block);
    }

    // 返回 merge_block 作为新的当前块
    return merge_block;
}

// ─── BuildWhile：构建 while 循环 ────────────────────────────────────────────
//
// 生成的 CFG 结构：
//
//     [current_block] ──> [header] (条件判断)
//                            |
//                         body为真: [body]
//                            |
//                         body为假: [exit_block]
//                            |
//                         [body_tail] ──(回边)──> [header]
//
// 为什么返回 header 而不是 exit_block？
// - while 循环后的代码应该接在循环之后，即 exit_block 路径上。
// - 但循环后的第一条语句的"前驱"应该是 header（因为 exit 是从 header 出去的）。
// - 实际上这里返回 header 是因为循环后的代码会作为 header 的后继处理，
//   而 exit_block 已经通过 AddEdge(header, exit_block) 连接了。
int CFGBuilder::BuildWhile(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    auto w = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);

    // 循环头块：包含条件判断
    int header = NewBlock();
    AddEdge(current_block, header);
    GetBlock(header).stmts.push_back(node);

    // 循环体块
    int body = NewBlock();
    AddEdge(header, body);      // 条件为真 → 进入体
    AddEdge(header, exit_block);// 条件为假 → 退出循环

    // 递归构建循环体，将 header 作为 exit_block（循环体末尾回到 header）
    int body_tail = BuildBlock(w->Block(), body, header);
    AddEdge(body_tail, header);// 回边：体末尾回到 header 重新判断

    return header;
}

// ─── BuildRepeat：构建 repeat-until 循环 ────────────────────────────────────
//
// 生成的 CFG 结构：
//
//     [current_block] ──> [body]
//                            |
//                         [body_tail] ──> [cond] (条件判断)
//                                     条件为假: [body] (回边)
//                                     条件为真: [exit_block]
//
// 与 while 的区别：
// - repeat 的条件在体之后判断，所以 cond 块在 body 之后。
// - 条件为假时继续循环（回到 body），为真时退出。
int CFGBuilder::BuildRepeat(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    auto r = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);

    // 循环体块
    int body = NewBlock();
    AddEdge(current_block, body);

    // 条件判断块（在体之后）
    int cond = NewBlock();
    GetBlock(cond).stmts.push_back(node);

    AddEdge(cond, body);      // 条件为假 → 再次循环
    AddEdge(cond, exit_block);// 条件为真 → 退出

    // 递归构建循环体，将 cond 作为 exit_block
    int body_tail = BuildBlock(r->Block(), body, cond);
    AddEdge(body_tail, cond);// 体末尾进入条件判断

    return cond;
}

// ─── BuildForLoop：构建数值型 for 循环 ──────────────────────────────────────
//
// 生成的 CFG 结构：
//
//     [current_block] ──> [init] (start/end/step 求值)
//                           |
//                       边界满足: [body]
//                       边界不足: [exit_block]
//                           |
//                        [body_tail] ──(回边)──> [init]
//
// init 块包含 for 节点的初始化表达式，每次循环回到 init 重新判断边界。
int CFGBuilder::BuildForLoop(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    auto fl = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);

    // 初始化块：包含 start/end/step 求值
    int init = NewBlock();
    AddEdge(current_block, init);
    GetBlock(init).stmts.push_back(node);

    // 循环体块
    int body = NewBlock();
    AddEdge(init, body);      // 边界满足 → 进入体
    AddEdge(init, exit_block);// 边界不满足 → 退出

    // 递归构建循环体，将 init 作为 exit_block
    int body_tail = BuildBlock(fl->Block(), body, init);
    AddEdge(body_tail, init);// 循环末尾回到 init 重新判断

    return init;
}

// ─── BuildForIn：构建 for-in 循环 ───────────────────────────────────────────
//
// 生成的 CFG 结构：
//
//     [current_block] ──> [init] (迭代器设置)
//                           |
//                       还有元素: [body]
//                       迭代结束: [exit_block]
//                           |
//                        [body_tail] ──(回边)──> [init]
//
int CFGBuilder::BuildForIn(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    auto fi = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);

    // 初始化块：迭代器设置
    int init = NewBlock();
    AddEdge(current_block, init);
    GetBlock(init).stmts.push_back(node);

    // 循环体块
    int body = NewBlock();
    AddEdge(init, body);      // 还有元素 → 进入体
    AddEdge(init, exit_block);// 迭代结束 → 退出

    // 递归构建循环体，将 init 作为 exit_block
    int body_tail = BuildBlock(fi->Block(), body, init);
    AddEdge(body_tail, init);// 体末尾回到 init 获取下一个元素

    return init;
}

// ─── ComputeDominators：计算支配集合 ────────────────────────────────────────
//
// 算法：迭代数据流分析（Iterative Data-Flow Analysis）
// ─────────────────────────────────────────────────
//
// 定义：
// - 块 d 支配块 b（d dom b），当且仅当从 entry 到 b 的每条路径都经过 d。
// - 每个块支配自身（自反性）。
//
// 数据流方程：
// - Dom(entry) = { entry }
// - Dom(n) = { n } ∪ (∩ Dom(p) for p ∈ pred(n))
//
// 即：块 n 的支配集 = 所有前驱支配集的交集，再加上 n 自身。
//
// 算法流程：
// ─────────
// 步骤 1：初始化
//   - entry 的支配集 = { entry }
//   - 其他所有块的支配集 = 所有块（最宽松的初始值）
//
// 步骤 2：迭代直到不动点
//   - 对每个非 entry 块 b：
//     - new_dom = 所有前驱 p 的 Dom(p) 的交集
//     - new_dom = new_dom ∪ { b }
//     - 如果 new_dom ≠ Dom(b)，更新 Dom(b) 并标记 changed
//
// 步骤 3：终止
//   - 当一次完整迭代中没有任何块的支配集改变时，达到不动点。
//
// 为什么这样收敛？
// - 支配集只能缩小（从所有块开始，每次取交集只会变小或不变）。
// - 每次迭代至少有一个块的支配集缩小（否则已收敛）。
// - 所以最多 |blocks| 轮迭代就会收敛。
//
// 时间复杂度：O(n² × k)，其中 n 是块数，k 是迭代轮数（通常很小）。
void CFGBuilder::ComputeDominators() {
    if (cfg_.blocks.empty()) return;

    // ─── 步骤 1：收集所有块 id ────────────────────────────────────────
    std::unordered_set<int> all;
    for (auto &b: cfg_.blocks) all.insert(b.id);

    // ─── 步骤 2：初始化支配集 ─────────────────────────────────────────
    // 每个块的支配集初始化为 all（最宽松的估计）
    for (auto &b: cfg_.blocks) {
        cfg_.dominators[b.id] = all;
    }
    // entry 的支配集 = { entry }（entry 只支配自身）
    cfg_.dominators[cfg_.entry_id] = {cfg_.entry_id};

    // ─── 步骤 3：迭代直到不动点 ───────────────────────────────────────
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto &b: cfg_.blocks) {
            // entry 的支配集不变，跳过
            if (b.id == cfg_.entry_id) continue;

            // 从所有块开始，逐步与前驱的支配集取交集
            std::unordered_set<int> new_dom = all;
            for (int pred_id: b.pred_ids) {
                auto it = cfg_.dominators.find(pred_id);
                if (it != cfg_.dominators.end()) {
                    // 取交集：只保留同时在前驱支配集中的元素
                    std::unordered_set<int> tmp;
                    for (int x: new_dom)
                        if (it->second.count(x)) tmp.insert(x);
                    new_dom = std::move(tmp);
                }
            }
            // 每个块支配自身
            new_dom.insert(b.id);

            // 如果支配集发生变化，标记 changed 继续迭代
            if (new_dom != cfg_.dominators[b.id]) {
                cfg_.dominators[b.id] = std::move(new_dom);
                changed = true;
            }
        }
    }
}

// ─── ComputeDominanceFrontier：计算支配边界 ─────────────────────────────────
//
// 定义：
// - 块 n 的支配边界 DF(n) = { y | ∃p ∈ pred(y), n 支配 p 但 n 不严格支配 y }
//
// 直觉理解：
// - 支配边界是"支配关系停止"的地方。
// - 如果 n 支配 y 的某个前驱 p，但 n 不支配 y 自身，
//   说明从 p 到 y 的路径"跳出"了 n 的支配范围。
//
// 用途：
// - SSA 构造中，如果变量 v 在块 n 中定义，则需要在 DF(n) 中的每个块
//   为 v 插入 φ 函数。
//
// 算法流程：
// ─────────
// 对每个块 n（作为潜在的支配者）：
//   对每个块 y（作为被检查的目标，y ≠ n）：
//     - 如果 y 的前驱数 < 2，跳过（单前驱块不需要 φ）
//     - 检查 n 是否支配 y 的某个前驱 p：
//       - 如果 p == n，n 支配自身（自反性）
//       - 否则检查 n ∈ Dom(p)
//     - 如果 n 支配 y 的某个前驱，但 n 不严格支配 y（n ∉ Dom(y)）：
//       - 将 y 加入 DF(n)
//
// 为什么 y 需要至少 2 个前驱？
// - 单前驱块 y 只有一个入边，如果 n 支配这个前驱，则 n 必然支配 y。
// - 所以单前驱块不可能在支配边界中（除非 n == y，但已排除）。
//
// 时间复杂度：O(n² × p)，其中 n 是块数，p 是平均前驱数。
void CFGBuilder::ComputeDominanceFrontier() {
    cfg_.dominance_frontier.clear();

    // 标准算法：DF(n) = { y | ∃p ∈ pred(y), n 支配 p 但 n 不严格支配 y }
    // 注意：n 支配自身（reflexive），所以当 n ∈ pred(y) 且 n 不严格支配 y 时，y ∈ DF(n)
    for (const auto &n: cfg_.blocks) {
        for (const auto &y: cfg_.blocks) {
            // 跳过自身
            if (y.id == n.id) continue;

            // 只有多前驱块才需要 φ（单前驱块不可能在支配边界中）
            if (y.pred_ids.size() < 2) continue;

            // ─── 检查 n 是否支配 y 的某个前驱 ────────────────────────
            bool n_doms_pred_of_y = false;
            for (int p: y.pred_ids) {
                if (p == n.id) {
                    // n 支配自身（reflexive dominance）
                    n_doms_pred_of_y = true;
                    break;
                }
                // 检查 n 是否支配前驱 p（即 n ∈ Dom(p)）
                auto pdom = cfg_.dominators.find(p);
                if (pdom != cfg_.dominators.end() && pdom->second.count(n.id)) {
                    n_doms_pred_of_y = true;
                    break;
                }
            }
            if (!n_doms_pred_of_y) continue;

            // ─── 检查 n 是否不严格支配 y ─────────────────────────────
            // 严格支配：n 支配 y 且 n ≠ y
            // 如果 n 严格支配 y，则 y 不在 DF(n) 中
            auto ydom = cfg_.dominators.find(y.id);
            bool n_strict_doms_y = (ydom != cfg_.dominators.end() && ydom->second.count(n.id));
            if (!n_strict_doms_y) {
                // n 支配 y 的某个前驱，但不严格支配 y → y ∈ DF(n)
                cfg_.dominance_frontier[n.id].insert(y.id);
            }
        }
    }
}

// ─── DumpToString：序列化为可读字符串 ───────────────────────────────────────
//
// 输出格式：
//   CFG[func_name] entry=N exits=[M]
//     block id: preds=[...] succs=[...] stmts=K
//     ...
//
// 用于测试验证和调试，帮助可视化 CFG 结构。
std::string CFGFunction::DumpToString() const {
    std::ostringstream oss;
    oss << "CFG[" << func_name << "] entry=" << entry_id << " exits=[";
    for (size_t i = 0; i < exit_ids.size(); ++i) {
        if (i > 0) oss << ",";
        oss << exit_ids[i];
    }
    oss << "]\n";
    for (const auto &b: blocks) {
        oss << "  block " << b.id << ": preds=[";
        for (size_t i = 0; i < b.pred_ids.size(); ++i) {
            if (i > 0) oss << ",";
            oss << b.pred_ids[i];
        }
        oss << "] succs=[";
        for (size_t i = 0; i < b.succ_ids.size(); ++i) {
            if (i > 0) oss << ",";
            oss << b.succ_ids[i];
        }
        oss << "] stmts=" << b.stmts.size() << "\n";
    }
    return oss.str();
}

}// namespace fakelua
