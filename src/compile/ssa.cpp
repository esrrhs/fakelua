#include "compile/ssa.h"

namespace fakelua {

SSAFunction SSABuilder::Build(const CFGFunction &cfg) {
    ssa_.func_name = cfg.func_name;
    ssa_.param_versions.clear();
    ssa_.phis.clear();
    ssa_.def_versions.clear();
    ssa_.use_versions.clear();
    ssa_.var_all_versions.clear();
    ssa_.version_to_block.clear();
    var_stacks_.clear();
    next_version_ = 0;

    // 为每个参数创建初始版本号（即使 CFG 构造是骨架，也供 RunWorklist/特化使用）
    int param_count = 0;
    for (const auto &[pname, pidx] : cfg.param_indices) {
        if (pidx + 1 > param_count) param_count = pidx + 1;
    }
    ssa_.param_versions.resize(param_count);
    for (int i = 0; i < param_count; ++i) {
        ssa_.param_versions[i] = NewVersion(std::to_string(i), /*block_id=*/0);
    }

    // todo: 完整实现 SSA 构造（Cytron 算法 + 变量重命名 + φ 节点）

    return std::move(ssa_);
}

int SSABuilder::NewVersion(const std::string &var_name, int /*block_id*/) {
    int ver = next_version_++;
    ssa_.var_all_versions[var_name].push_back(ver);
    return ver;
}

int SSABuilder::GetCurrVersion(const std::string &var_name) {
    auto it = var_stacks_.find(var_name);
    if (it != var_stacks_.end() && !it->second.empty())
        return it->second.back();
    return -1;
}

}// namespace fakelua
