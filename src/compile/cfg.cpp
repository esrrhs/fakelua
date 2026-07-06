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
    int tail = BuildBlock(func_block, entry, exit);
    AddEdge(tail, exit);

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

int CFGBuilder::BuildBlock(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    if (!node) return current_block;
    if (node->Type() != SyntaxTreeType::Block) return current_block;

    auto block_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
    int cur = current_block;
    for (auto &stmt : block_ptr->Stmts()) {
        cur = BuildStmt(stmt, cur, exit_block);
    }
    return cur;
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

    // 1. 条件判断块
    int cond_block = current_block;
    GetBlock(cond_block).stmts.push_back(node);

    // 1. 先创建汇合块（这样可以将它作为 exit_block 传给分支）
    int merge_block = NewBlock();

    // 收集各分支的尾块（用于连接到汇合块）
    std::vector<int> branch_tails;
    bool has_full_coverage = true;  // 是否有 else（全覆盖）

    // 2. then 块
    int then_block = NewBlock();
    AddEdge(cond_block, then_block);
    int then_tail = BuildBlock(ifn->Block(), then_block, merge_block);
    branch_tails.push_back(then_tail);

    // 3. elseif 链
    auto elseiflist = ifn->ElseIfs();
    if (elseiflist && elseiflist->Type() == SyntaxTreeType::ElseIfList) {
        auto elist = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseiflist);
        for (size_t i = 0; i < elist->ElseifSize(); ++i) {
            int ecblock = NewBlock();
            AddEdge(cond_block, ecblock);
            int tail = BuildBlock(elist->ElseifBlock(i), ecblock, merge_block);
            branch_tails.push_back(tail);
        }
    }

    // 4. else 块
    if (auto eb = ifn->ElseBlock()) {
        int else_block = NewBlock();
        AddEdge(cond_block, else_block);
        int tail = BuildBlock(eb, else_block, merge_block);
        branch_tails.push_back(tail);
    } else {
        has_full_coverage = false;
    }

    // 5. 将各分支尾连接到汇合块
    for (int tail : branch_tails) {
        AddEdge(tail, merge_block);
    }
    // 如果没有 else，条件块 fall-through 到汇合块
    if (!has_full_coverage) {
        AddEdge(cond_block, merge_block);
    }

    return merge_block;
}

int CFGBuilder::BuildWhile(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    auto *w = static_cast<SyntaxTreeWhile *>(node.get());

    // 循环头块（条件判断）
    int header = NewBlock();
    AddEdge(current_block, header);
    GetBlock(header).stmts.push_back(node);

    // 循环体块
    int body = NewBlock();
    AddEdge(header, body);  // 条件为真 → 进入体
    AddEdge(header, exit_block);  // 条件为假 → 退出

    // 递归构建循环体，末尾回到 header
    int body_tail = BuildBlock(w->Block(), body, header);
    AddEdge(body_tail, header);

    return header;
}

int CFGBuilder::BuildRepeat(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    auto *r = static_cast<SyntaxTreeRepeat *>(node.get());

    int body = NewBlock();
    AddEdge(current_block, body);

    // 条件的块（在体之后）
    int cond = NewBlock();
    GetBlock(cond).stmts.push_back(node);

    AddEdge(cond, body);  // 条件为假 → 再次循环
    AddEdge(cond, exit_block);  // 条件为真 → 退出

    int body_tail = BuildBlock(r->Block(), body, cond);
    AddEdge(body_tail, cond);  // 体末尾进入条件

    return cond;
}

int CFGBuilder::BuildForLoop(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block) {
    auto *fl = static_cast<SyntaxTreeForLoop *>(node.get());

    // 初始化块（start/end/step 求值）
    int init = NewBlock();
    AddEdge(current_block, init);
    GetBlock(init).stmts.push_back(node);

    // 循环体块
    int body = NewBlock();
    AddEdge(init, body);  // 进入体
    AddEdge(init, exit_block);  // 边界不满足 → 退出

    int body_tail = BuildBlock(fl->Block(), body, init);
    AddEdge(body_tail, init);  // 循环末尾回到 init 重新判断

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

    int body_tail = BuildBlock(fi->Block(), body, init);
    AddEdge(body_tail, init);

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
    // 标准算法：DF(n) = { y | ∃p ∈ pred(y), n 支配 p 但 n 不严格支配 y }
    // 注意：n 支配自身（reflexive），所以当 n ∈ pred(y) 且 n 不严格支配 y 时，y ∈ DF(n)
    for (const auto &n : cfg_.blocks) {
        for (const auto &y : cfg_.blocks) {
            if (y.id == n.id) continue;
            if (y.pred_ids.size() < 2) continue;  // 只有多前驱块才需要 φ
            // 检查 n 是否支配 y 的某个前驱（包括 n 自身）
            bool n_doms_pred_of_y = false;
            for (int p : y.pred_ids) {
                if (p == n.id) {
                    // n 支配自身（reflexive dominance）
                    n_doms_pred_of_y = true;
                    break;
                }
                auto pdom = cfg_.dominators.find(p);
                if (pdom != cfg_.dominators.end() && pdom->second.count(n.id)) {
                    n_doms_pred_of_y = true;
                    break;
                }
            }
            if (!n_doms_pred_of_y) continue;
            // 检查 n 是否不严格支配 y
            auto ydom = cfg_.dominators.find(y.id);
            bool n_strict_doms_y = (ydom != cfg_.dominators.end() && ydom->second.count(n.id));
            if (!n_strict_doms_y) {
                cfg_.dominance_frontier[n.id].insert(y.id);
            }
        }
    }
}

std::string CFGFunction::DumpToString() const {
    std::ostringstream oss;
    oss << "CFG[" << func_name << "] entry=" << entry_id << " exits=[";
    for (size_t i = 0; i < exit_ids.size(); ++i) {
        if (i > 0) oss << ",";
        oss << exit_ids[i];
    }
    oss << "]\n";
    for (const auto &b : blocks) {
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
