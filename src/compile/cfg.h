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

    void BuildBlock(const SyntaxTreeInterfacePtr &node, int current_block, int exit_block);
    void BuildStmt(const SyntaxTreeInterfacePtr &stmt, int current_block, int exit_block);

    void ComputeDominators();
    void ComputeDominanceFrontier();
};

}// namespace fakelua
