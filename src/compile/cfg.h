#pragma once

#include "compile/syntax_tree.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fakelua {

// 基本块
struct BasicBlock {
    int id = -1;
    std::vector<SyntaxTreeInterfacePtr> stmts;
    std::vector<int> pred_ids;
    std::vector<int> succ_ids;
};

// 单个函数的控制流图
struct CFGFunction {
    std::string func_name;
    std::unordered_map<std::string, int> param_indices;  // 参数名 -> 参数索引
    std::vector<BasicBlock> blocks;
    int entry_id = -1;
    std::vector<int> exit_ids;

    // 按 id 查找块（线性扫描，通常块数量很少）
    [[nodiscard]] const BasicBlock *FindBlock(int id) const {
        for (const auto &b : blocks)
            if (b.id == id) return &b;
        return nullptr;
    }
    [[nodiscard]] BasicBlock *FindBlock(int id) {
        for (auto &b : blocks)
            if (b.id == id) return &b;
        return nullptr;
    }
    std::unordered_map<int, std::unordered_set<int>> dominators;      // id -> dominated set
    std::unordered_map<int, std::unordered_set<int>> dominance_frontier;
};

// CFG builder - 从函数 block 构建控制流图
class CFGBuilder {
public:
    // 构建函数 block 的 CFG
    CFGFunction Build(const SyntaxTreeInterfacePtr &func_block,
                      const std::vector<std::string> &params,
                      const std::string &func_name,
                      bool is_vararg);

private:
    CFGFunction cfg_;
    int next_block_id_ = 0;

    int NewBlock();
    BasicBlock &GetBlock(int id);
    void AddEdge(int from, int to);

    // 构建函数体：从 entry 块开始，exit 块为终止块
    void BuildBlock(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block);

    // 构建单条语句：可能分裂基本块（if/while/for/repeat）
    // 返回"当前块"（可能已被分裂）
    int BuildStmt(const SyntaxTreeInterfacePtr &stmt, int current_block, int exit_block);

    // 构建 if/elseif/else 链
    int BuildIf(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block);

    // 构建 while 循环
    int BuildWhile(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block);

    // 构建 repeat-until 循环
    int BuildRepeat(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block);

    // 构建 for 循环（数值型）
    int BuildForLoop(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block);

    // 构建 for-in 循环
    int BuildForIn(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block);

    void ComputeDominators();
    void ComputeDominanceFrontier();
};

}// namespace fakelua
