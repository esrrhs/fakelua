#include "compile/cfg.h"

namespace fakelua {

CFGFunction CFGBuilder::Build(const SyntaxTreeInterfacePtr &func_block,
                                const std::vector<std::string> &params,
                                const std::string &func_name,
                                bool /*is_vararg*/) {
    cfg_.func_name = func_name;
    cfg_.blocks.clear();
    cfg_.param_indices.clear();
    cfg_.entry_id = -1;
    cfg_.exit_ids.clear();
    next_block_id_ = 0;

    // 记录参数
    for (size_t i = 0; i < params.size(); ++i) {
        cfg_.param_indices[params[i]] = static_cast<int>(i);
    }

    // 创建 entry 和 exit 块
    int entry = NewBlock();
    int exit = NewBlock();
    cfg_.entry_id = entry;
    cfg_.exit_ids.push_back(exit);

    // 构建函数体
    BuildBlock(func_block, entry, exit);

    // 计算支配关系（可选，需要 SSA φ 插入时使用）
    ComputeDominators();
    ComputeDominanceFrontier();

    return std::move(cfg_);
}

int CFGBuilder::NewBlock() {
    int id = next_block_id_++;
    cfg_.blocks.push_back({id, {}, {}, {}});
    return id;
}

BasicBlock &CFGBuilder::GetBlock(int id) {
    for (auto &b : cfg_.blocks)
        if (b.id == id) return b;
    DEBUG_ASSERT(false);
    return cfg_.blocks[0];
}

void CFGBuilder::AddEdge(int from, int to) {
    auto &fb = GetBlock(from);
    if (std::find(fb.succ_ids.begin(), fb.succ_ids.end(), to) == fb.succ_ids.end()) {
        fb.succ_ids.push_back(to);
        GetBlock(to).pred_ids.push_back(from);
    }
}

void CFGBuilder::BuildBlock(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    if (!node) return;
    if (node->Type() != SyntaxTreeType::Block) return;

    auto block_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
    int cur = current_block;
    for (auto &stmt : block_ptr->Stmts()) {
        cur = BuildStmt(stmt, cur, exit_block);
    }
    AddEdge(cur, exit_block);
}

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
            // 普通语句：追加到 current_block
            GetBlock(current_block).stmts.push_back(stmt);
            return current_block;
        }
    }
}

int CFGBuilder::BuildIf(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    auto *ifn = static_cast<SyntaxTreeIf *>(node.get());

    // 1. 当前块成为条件判断块（条件表达式已在上一语句追加到 current_block 之前的块）
    //    这里条件表达式已经作为前缀被处理？实际上 IF 的 Exp() 是条件，独立追加到 cond block
    int cond_block = current_block;
    GetBlock(cond_block).stmts.push_back(node);  // 整个 IF 节点进入 cond block 供分析器读取

    // 2. then 块
    int then_block = NewBlock();
    AddEdge(cond_block, then_block);

    // 3. 处理 elseif 链
    //    先处理 if body
    auto if_block_body = ifn->Block();
    BuildBlock(if_block_body, then_block, exit_block);  // then 跳入 exit

    // elseif 链
    auto elseiflist = ifn->ElseIfs();
    if (elseiflist && elseiflist->Type() == SyntaxTreeType::ElseIfList) {
        auto elist = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseiflist);
        for (size_t i = 0; i < elist->ElseifSize(); ++i) {
            int ecblock = NewBlock();
            AddEdge(cond_block, ecblock);
            // 当前设计简化：条件 expr 也连接到每个 elseif
            // 分析时可以从 elseif node 获取条件
            auto ebody = elist->ElseifBlock(i);
            BuildBlock(ebody, ecblock, exit_block);
        }
    }

    // else 块
    if (auto eb = ifn->ElseBlock()) {
        int else_block = NewBlock();
        AddEdge(cond_block, else_block);
        BuildBlock(eb, else_block, exit_block);
    }

    // 返回条件块（不包含连接到 exit 的边——每个分支自行连接）
    // 但条件块没有 fall-through 到 exit: 必须分支返回。
    // 当前设计：条件的 fall-through 也连到 exit（不含 else 时）
    if (!ifn->ElseBlock() && (!elseiflist || elseiflist->Type() != SyntaxTreeType::ElseIfList ||
        std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseiflist)->ElseifSize() == 0)) {
        AddEdge(cond_block, exit_block);  // 没有 else 时的 fall-through
    }

    // 汇合块已经由各分支连接 exit_block 处理。返回 cond_block 以便后续语句追加。
    return cond_block;
}

int CFGBuilder::BuildWhile(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    auto *w = static_cast<SyntaxTreeWhile *>(node.get());

    // 循环头块（条件判断）
    int header = NewBlock();
    AddEdge(current_block, header);
    GetBlock(header).stmts.push_back(node);  // while 进入 header 供分析器读取

    // 循环体块
    int body = NewBlock();
    AddEdge(header, body);  // 条件为真 → 进入体

    // exit 块的后继 = 条件为假跳出
    AddEdge(header, exit_block);

    // 递归构建循环体
    BuildBlock(w->Block(), body, header);  // 体末尾回到 header

    return header;  // 继续从 header 往外追加后续语句
}

int CFGBuilder::BuildRepeat(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    auto *r = static_cast<SyntaxTreeRepeat *>(node.get());

    int body = NewBlock();
    AddEdge(current_block, body);

    // 条件的块（在体之后）
    int cond = NewBlock();
    AddEdge(body, cond);  // 体末尾进入条件
    GetBlock(cond).stmts.push_back(node);

    AddEdge(cond, body);  // 条件为假 → 再次循环
    AddEdge(cond, exit_block);  // 条件为真 → 退出

    BuildBlock(r->Block(), body, cond);

    return cond;  // 后续语句追加到 cond 块
}

int CFGBuilder::BuildForLoop(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    auto *fl = static_cast<SyntaxTreeForLoop *>(node.get());

    // 初始化块（start/end/step 求值）
    int init = NewBlock();
    AddEdge(current_block, init);
    GetBlock(init).stmts.push_back(node);  // for 语句进入 init 块

    // 循环体块
    int body = NewBlock();
    AddEdge(init, body);  // 进入体
    AddEdge(init, exit_block);  // 边界不满足 → 退出

    // 循环末尾回到 init 块重新判断（数值 for）
    BuildBlock(fl->Block(), body, init);

    return init;
}

int CFGBuilder::BuildForIn(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    auto *fi = static_cast<SyntaxTreeForIn *>(node.get());

    int init = NewBlock();
    AddEdge(current_block, init);
    GetBlock(init).stmts.push_back(node);

    int body = NewBlock();
    AddEdge(init, body);
    AddEdge(init, exit_block);

    BuildBlock(fi->Block(), body, init);

    return init;
}

void CFGBuilder::ComputeDominators() {
    if (cfg_.blocks.empty()) return;

    // 收集所有块 id
    std::unordered_set<int> all;
    for (auto &b : cfg_.blocks) all.insert(b.id);

    // 每个块的支配集 = all（除 entry 外）
    // entry 的支配集 = {entry}
    for (auto &b : cfg_.blocks) {
        cfg_.dominators[b.id] = all;
    }
    cfg_.dominators[cfg_.entry_id] = {cfg_.entry_id};

    // 迭代直到不动点
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto &b : cfg_.blocks) {
            if (b.id == cfg_.entry_id) continue;
            std::unordered_set<int> new_dom = all;
            for (int pred_id : b.pred_ids) {
                auto it = cfg_.dominators.find(pred_id);
                if (it != cfg_.dominators.end()) {
                    std::unordered_set<int> tmp;
                    for (int x : new_dom)
                        if (it->second.count(x)) tmp.insert(x);
                    new_dom = std::move(tmp);
                }
            }
            new_dom.insert(b.id);
            if (new_dom != cfg_.dominators[b.id]) {
                cfg_.dominators[b.id] = std::move(new_dom);
                changed = true;
            }
        }
    }
}

void CFGBuilder::ComputeDominanceFrontier() {
    cfg_.dominance_frontier.clear();
    for (auto &b : cfg_.blocks) {
        if (b.pred_ids.size() >= 2) {
            for (int pred_id : b.pred_ids) {
                int runner = pred_id;
                // runner 不严格支配 b（pred 路径上第一个满足条件的块）
                auto rdom = cfg_.dominators.find(runner);
                if (rdom == cfg_.dominators.end()) continue;
                // runner 已经支配 b——沿支配树向上找严格支配节点
                // 简化：DF(n) = {y | n 支配某个 y 的 pred 但 n 不严格支配 y}
                for (auto &cb : cfg_.blocks) {
                    if (cb.id == runner) continue;
                    // 如果 runner 不是 cb 的严格支配者但 runner 支配 cb 的某个 pred
                    bool runner_strict_doms_cb = false;
                    bool runner_doms_pred_of_cb = false;
                    auto cdom = cfg_.dominators.find(cb.id);
                    if (cdom != cfg_.dominators.end()) {
                        runner_strict_doms_cb = (runner != cb.id && cdom->second.count(runner));
                    }
                    for (int p : cb.pred_ids) {
                        if (p == runner) continue;
                        auto pdom = cfg_.dominators.find(p);
                        if (pdom != cfg_.dominators.end() && pdom->second.count(runner))
                            runner_doms_pred_of_cb = true;
                    }
                    if (runner_doms_pred_of_cb && !runner_strict_doms_cb) {
                        cfg_.dominance_frontier[runner].insert(cb.id);
                    }
                }
            }
        }
    }
}

}// namespace fakelua
