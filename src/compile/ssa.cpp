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
    // 建立 index → name 的反向映射，使 var_all_versions 按参数名索引
    std::unordered_map<int, std::string> idx_to_name;
    for (const auto &[pname, pidx] : cfg.param_indices) {
        idx_to_name[pidx] = pname;
    }
    int param_count = (int)cfg.param_indices.size();
    ssa_.param_versions.resize(param_count, -1);
    for (int i = 0; i < param_count; ++i) {
        auto it = idx_to_name.find(i);
        const std::string &pname = (it != idx_to_name.end()) ? it->second : std::to_string(i);
        int ver = NewVersion(pname, /*block_id=*/0);
        ssa_.param_versions[i] = ver;
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
