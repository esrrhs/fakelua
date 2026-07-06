#pragma once

#include "compile/cfg.h"
#include "compile/compile_common.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fakelua {

// SSA 形式的 φ 节点
struct PhiNode {
    int var_id = -1;              // 变量索引
    std::string var_name;          // 变量名（调试/dump 用）
    int result_version = -1;       // 该 φ 产生的新版本号
    std::vector<int> arg_versions; // 各前驱块传入的版本号（按 pred_ids 顺序）
};

// 单个函数的 SSA 形式
struct SSAFunction {
    std::string func_name;
    std::vector<int> param_versions;  // 每个参数的初始 SSA 版本号

    // block_id → 该块入口的 φ 节点列表
    std::unordered_map<int, std::vector<PhiNode>> block_phis;

    // AST 节点 → 定义的版本号
    std::unordered_map<const SyntaxTreeInterface *, int> def_versions;
    // AST 节点 → 使用的版本号列表
    std::unordered_map<const SyntaxTreeInterface *, std::vector<int>> use_versions;

    // 变量名 → 所有 SSA 版本号
    std::unordered_map<std::string, std::vector<int>> var_all_versions;
    // 版本号 → 定义所在块
    std::unordered_map<int, int> version_to_block;
    // 版本号 → 变量名
    std::unordered_map<int, std::string> version_to_name;
    // 版本号 → 类型信息（φ 类型推导后填充）
    std::unordered_map<int, SSATypeInfo> version_types;

    int next_version = 0;

    // 序列化为可读字符串（用于测试验证）
    [[nodiscard]] std::string DumpToString() const;
};

class SSABuilder {
public:
    SSAFunction Build(const CFGFunction &cfg);

private:
    SSAFunction ssa_;
    int next_version_ = 0;

    // 变量名 → 变量索引
    std::unordered_map<std::string, int> var_name_to_id_;
    std::vector<std::string> var_id_to_name_;

    // 变量索引 → 定义该变量的基本块集合
    std::vector<std::unordered_set<int>> var_def_blocks_;

    // 支配树子节点
    std::unordered_map<int, std::vector<int>> dom_tree_children_;

    int GetVarId(const std::string &name);
    int NewVersion(const std::string &var_name, int block_id);

    // 收集每个块中定义的所有变量
    void CollectDefBlocks(const CFGFunction &cfg);

    // 构建支配树（从支配关系推导）
    void BuildDomTree(const CFGFunction &cfg);

    // Cytron Step 1: 插入 φ 节点
    void InsertPhis(const CFGFunction &cfg);

    // Cytron Step 2: 变量重命名
    void RenameVariables(const CFGFunction &cfg);

    // 在单个块中处理语句的定义/使用
    void RenameBlockStmts(int block_id, const std::vector<SyntaxTreeInterfacePtr> &stmts,
                         std::unordered_map<int, int> &stack_top);

    // 收集单个语句中定义的变量名
    static std::vector<std::string> GetDefNames(const SyntaxTreeInterfacePtr &stmt);
    // 收集单个语句中使用的变量名（递归进入表达式，不含定义目标的 LHS）
    static std::vector<std::string> GetUseNames(const SyntaxTreeInterfacePtr &stmt);
};

}// namespace fakelua
