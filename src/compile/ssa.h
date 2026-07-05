#pragma once

#include "compile/cfg.h"
#include "compile/compile_common.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fakelua {

// SSA 形式的 phi 节点
struct PhiNode {
    int result_version = -1;
    std::vector<int> arg_versions;
};

// SSA 形式的一个基本块
struct SSABlock {
    int id = -1;
    std::vector<PhiNode> phis;
};

// 单个函数的 SSA 形式
struct SSAFunction {
    std::string func_name;
    std::vector<int> param_versions;  // 每个参数的初始 SSA 版本号
    std::unordered_map<int, PhiNode> phis;  // 基本块 id -> phi nodes list (keyed by result_version)
    std::unordered_map<const SyntaxTreeInterfacePtr::element_type *, int> def_versions;
    std::unordered_map<const SyntaxTreeInterfacePtr::element_type *, int> use_versions;
    std::unordered_map<int, std::vector<int>> def_versions_multi;  // 单个节点可能定义多个版本
    std::unordered_map<std::string, std::vector<int>> var_all_versions;  // 变量名 -> 所有 SSA 版本号
    std::unordered_map<int, int> version_to_block;  // 版本号 -> 定义所在块
};

// SSA Builder
class SSABuilder {
public:
    SSAFunction Build(const CFGFunction &cfg);

private:
    SSAFunction ssa_;
    int next_version_ = 0;
    std::unordered_map<std::string, std::vector<int>> var_stacks_;

    int NewVersion(const std::string &var_name, int block_id);
    int GetCurrVersion(const std::string &var_name);
};

}// namespace fakelua
