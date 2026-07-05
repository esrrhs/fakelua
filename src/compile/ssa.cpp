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

    // 为每个参数创建初始版本
    for (int i = 0; i < (int) cfg.blocks.size(); ++i) {
        const auto &blk = cfg.blocks[i];
        // todo: 完整实现 SSA 构造
    }

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
