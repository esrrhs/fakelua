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

    // 计算支配关系
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
    for (auto &stmt : block_ptr->Stmts()) {
        BuildStmt(stmt, current_block, exit_block);
    }
    AddEdge(current_block, exit_block);
}

void CFGBuilder::BuildStmt(const SyntaxTreeInterfacePtr &stmt, int current_block, int exit_block) {
    if (!stmt) return;
    // 简化实现：把所有语句顺序放入 current_block
    GetBlock(current_block).stmts.push_back(stmt);
}

void CFGBuilder::ComputeDominators() {
    // 简化实现
    cfg_.dominators.clear();
}

void CFGBuilder::ComputeDominanceFrontier() {
    cfg_.dominance_frontier.clear();
}

}// namespace fakelua
