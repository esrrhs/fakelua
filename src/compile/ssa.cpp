#include "compile/ssa.h"
#include "compile/syntax_tree.h"

namespace fakelua {

// ── 辅助：变量 ID ────────────────────────────────────────────────────────
int SSABuilder::GetVarId(const std::string &name) {
    auto it = var_name_to_id_.find(name);
    if (it != var_name_to_id_.end()) return it->second;
    int id = (int)var_id_to_name_.size();
    var_name_to_id_[name] = id;
    var_id_to_name_.push_back(name);
    var_def_blocks_.emplace_back();
    return id;
}

int SSABuilder::NewVersion(const std::string &var_name, int block_id) {
    int ver = ssa_.next_version++;
    ssa_.var_all_versions[var_name].push_back(ver);
    ssa_.version_to_block[ver] = block_id;
    ssa_.version_to_name[ver] = var_name;
    return ver;
}

// ── 辅助：收集定义/使用名 ────────────────────────────────────────────────
std::vector<std::string> SSABuilder::GetDefNames(const SyntaxTreeInterfacePtr &stmt) {
    std::vector<std::string> names;
    if (!stmt) return names;
    switch (stmt->Type()) {
        case SyntaxTreeType::LocalVar: {
            auto *lv = static_cast<SyntaxTreeLocalVar *>(stmt.get());
            if (auto *nl = static_cast<SyntaxTreeNamelist *>(lv->Namelist().get())) {
                for (auto &n : nl->Names()) names.push_back(n);
            }
            break;
        }
        case SyntaxTreeType::Assign: {
            auto *as = static_cast<SyntaxTreeAssign *>(stmt.get());
            auto *vl = static_cast<SyntaxTreeVarlist *>(as->Varlist().get());
            if (vl) {
                for (auto &v : vl->Vars()) {
                    if (v->Type() == SyntaxTreeType::Var) {
                        auto *var = static_cast<SyntaxTreeVar *>(v.get());
                        if (var->GetVarKind() == VarKind::kSimple)
                            names.push_back(var->GetName());
                    }
                }
            }
            break;
        }
        case SyntaxTreeType::ForLoop: {
            auto *fl = static_cast<SyntaxTreeForLoop *>(stmt.get());
            names.push_back(fl->Name());
            break;
        }
        case SyntaxTreeType::ForIn: {
            auto *fi = static_cast<SyntaxTreeForIn *>(stmt.get());
            if (auto *nl = static_cast<SyntaxTreeNamelist *>(fi->Namelist().get())) {
                for (auto &n : nl->Names()) names.push_back(n);
            }
            break;
        }
        default:
            break;
    }
    return names;
}

std::vector<std::string> SSABuilder::GetUseNames(const SyntaxTreeInterfacePtr &stmt) {
    std::vector<std::string> names;
    if (!stmt) return names;

    // 递归遍历表达式树收集 Var(kSimple) 的名称
    std::function<void(const SyntaxTreeInterfacePtr&)> collect;
    collect = [&](const SyntaxTreeInterfacePtr &node) {
        if (!node) return;
        if (node->Type() == SyntaxTreeType::Var) {
            auto *v = static_cast<SyntaxTreeVar *>(node.get());
            if (v->GetVarKind() == VarKind::kSimple)
                names.push_back(v->GetName());
            return;
        }
        switch (node->Type()) {
            case SyntaxTreeType::Exp: {
                auto *e = static_cast<SyntaxTreeExp *>(node.get());
                auto k = e->GetExpKind();
                if (k == ExpKind::kBinop || k == ExpKind::kUnop) {
                    collect(e->Left());
                    collect(e->Right());
                } else if (k == ExpKind::kPrefixExp) {
                    collect(e->Right());
                }
                break;
            }
            case SyntaxTreeType::PrefixExp: {
                auto *pe = static_cast<SyntaxTreePrefixexp *>(node.get());
                if (pe->GetPrefixKind() == PrefixExpKind::kVar && pe->GetValue()) {
                    collect(pe->GetValue());
                } else if (pe->GetPrefixKind() == PrefixExpKind::kExp && pe->GetValue()) {
                    collect(pe->GetValue());
                }
                break;
            }
            case SyntaxTreeType::TableConstructor: {
                auto *tc = static_cast<SyntaxTreeTableconstructor *>(node.get());
                if (tc->Fieldlist()) collect(tc->Fieldlist());
                break;
            }
            case SyntaxTreeType::FieldList: {
                auto *fl = static_cast<SyntaxTreeFieldlist *>(node.get());
                for (auto &f : fl->Fields()) collect(f);
                break;
            }
            case SyntaxTreeType::Field: {
                auto *f = static_cast<SyntaxTreeField *>(node.get());
                if (f->Key()) collect(f->Key());
                if (f->Value()) collect(f->Value());
                break;
            }
            case SyntaxTreeType::FunctionCall: {
                auto *fc = static_cast<SyntaxTreeFunctioncall *>(node.get());
                if (fc->prefixexp()) collect(fc->prefixexp());
                if (fc->Args()) collect(fc->Args());
                break;
            }
            case SyntaxTreeType::Args: {
                auto *args = static_cast<SyntaxTreeArgs *>(node.get());
                if (args->GetArgsKind() == ArgsKind::kExpList && args->Explist())
                    collect(args->Explist());
                else if (args->Tableconstructor())
                    collect(args->Tableconstructor());
                break;
            }
            case SyntaxTreeType::ExpList: {
                auto *el = static_cast<SyntaxTreeExplist *>(node.get());
                for (auto &e : el->Exps()) collect(e);
                break;
            }
            case SyntaxTreeType::VarList: {
                auto *vl = static_cast<SyntaxTreeVarlist *>(node.get());
                for (auto &v : vl->Vars()) collect(v);
                break;
            }
            default:
                break;
        }
    };

    switch (stmt->Type()) {
        case SyntaxTreeType::LocalVar: {
            auto *lv = static_cast<SyntaxTreeLocalVar *>(stmt.get());
            if (lv->Explist()) collect(lv->Explist());
            break;
        }
        case SyntaxTreeType::Assign: {
            auto *as = static_cast<SyntaxTreeAssign *>(stmt.get());
            if (as->Explist()) collect(as->Explist());
            break;
        }
        case SyntaxTreeType::ForLoop: {
            auto *fl = static_cast<SyntaxTreeForLoop *>(stmt.get());
            if (fl->ExpBegin()) collect(fl->ExpBegin());
            if (fl->ExpEnd()) collect(fl->ExpEnd());
            if (fl->ExpStep()) collect(fl->ExpStep());
            break;
        }
        case SyntaxTreeType::ForIn: {
            auto *fi = static_cast<SyntaxTreeForIn *>(stmt.get());
            if (fi->Explist()) collect(fi->Explist());
            break;
        }
        case SyntaxTreeType::Return: {
            auto *ret = static_cast<SyntaxTreeReturn *>(stmt.get());
            if (ret->Explist()) collect(ret->Explist());
            break;
        }
        default:
            break;
    }
    return names;
}

// ── Step 0: 收集定义块 ─────────────────────────────────────────────────
void SSABuilder::CollectDefBlocks(const CFGFunction &cfg) {
    for (const auto &b : cfg.blocks) {
        for (const auto &s : b.stmts) {
            for (const auto &name : GetDefNames(s)) {
                int vid = GetVarId(name);
                var_def_blocks_[vid].insert(b.id);
            }
        }
    }
}

// ── 构建支配树子节点 ────────────────────────────────────────────────────
void SSABuilder::BuildDomTree(const CFGFunction &cfg) {
    for (const auto &b : cfg.blocks) {
        if (b.id == cfg.entry_id) continue;
        // 找到直接支配者（严格支配者中，被所有其他严格支配者支配的那个）
        int idom = -1;
        auto it = cfg.dominators.find(b.id);
        if (it == cfg.dominators.end()) continue;
        for (int d : it->second) {
            if (d == b.id) continue;  // 跳过自身
            // d 是严格支配者。检查 d 是否是"最近"的。
            bool is_idom = true;
            for (int d2 : it->second) {
                if (d2 == b.id || d2 == d) continue;
                auto it2 = cfg.dominators.find(d2);
                if (it2 != cfg.dominators.end() && it2->second.count(d)) {
                    // d2 被 d 支配 → d 比 d2 更远 → d 不是直接支配者
                    is_idom = false;
                    break;
                }
            }
            if (is_idom) { idom = d; break; }
        }
        if (idom >= 0) dom_tree_children_[idom].push_back(b.id);
    }
}

// ── Cytron Step 1: 插入 φ 节点 ────────────────────────────────────────
void SSABuilder::InsertPhis(const CFGFunction &cfg) {
    // 已在 Build 中完成的 var_name_to_id_ 和 var_def_blocks_

    int num_vars = (int)var_id_to_name_.size();
    // 对每个变量，遍历其定义块的支配边界，插入 φ 节点
    for (int vid = 0; vid < num_vars; ++vid) {
        auto it = var_def_blocks_[vid];
        std::vector<int> worklist(it.begin(), it.end());
        std::unordered_set<int> has_phi;  // 已有 φ 的块

        for (size_t i = 0; i < worklist.size(); ++i) {
            int n = worklist[i];
            auto df_it = cfg.dominance_frontier.find(n);
            if (df_it == cfg.dominance_frontier.end()) continue;
            for (int d : df_it->second) {
                if (has_phi.count(d)) continue;
                has_phi.insert(d);
                // 在 d 块插入 φ 节点
                auto *blk = cfg.FindBlock(d);
                int npreds = blk ? (int)blk->pred_ids.size() : 0;
                PhiNode phi;
                phi.var_id = vid;
                phi.var_name = var_id_to_name_[vid];
                phi.arg_versions.resize(npreds, -1);
                ssa_.block_phis[d].push_back(std::move(phi));

                // 如果 d 本身不定义该变量，也加入 worklist
                if (!var_def_blocks_[vid].count(d)) {
                    worklist.push_back(d);
                }
            }
        }
    }
}

// ── Cytron Step 2: 变量重命名 ─────────────────────────────────────────
void SSABuilder::RenameBlockStmts(
    int block_id, const std::vector<SyntaxTreeInterfacePtr> &stmts,
    std::unordered_map<int, int> &stack_top) {
    // 1. 处理 φ 节点：为每个 φ 分配结果版本
    auto phi_it = ssa_.block_phis.find(block_id);
    if (phi_it != ssa_.block_phis.end()) {
        for (auto &phi : phi_it->second) {
            int new_ver = NewVersion(phi.var_name, block_id);
            phi.result_version = new_ver;
            stack_top[phi.var_id] = new_ver;
        }
    }

    // 2. 处理块内语句
    for (const auto &stmt : stmts) {
        // 2a. 替换使用：将语句中引用的变量替换为当前栈顶版本
        for (const auto &uname : GetUseNames(stmt)) {
            auto vit = var_name_to_id_.find(uname);
            if (vit == var_name_to_id_.end()) continue;
            int vid = vit->second;
            auto sit = stack_top.find(vid);
            if (sit != stack_top.end()) {
                // 查找该语句中的 Var 节点并记录 use_version
                // （简化：记录到 use_versions，不修改 AST）
                // 在 Phase 1 中，我们仅收集版本号供调试
                (void)0;  // 实际替换在代码生成阶段
            }
        }
        // 2b. 处理定义：为每个定义变量分配新版本
        for (const auto &dname : GetDefNames(stmt)) {
            int vid = GetVarId(dname);
            int new_ver = NewVersion(dname, block_id);
            stack_top[vid] = new_ver;
            ssa_.def_versions[stmt.get()] = new_ver;
        }
    }
}

void SSABuilder::RenameVariables(const CFGFunction &cfg) {
    std::unordered_map<int, int> stack_top;  // var_id → 当前栈顶版本
    // DFS 遍历支配树
    std::function<void(int)> rename_dfs = [&](int bid) {
        RenameBlockStmts(bid, cfg.FindBlock(bid)->stmts, stack_top);

        // 处理后继块中的 φ 节点参数
        auto *blk = cfg.FindBlock(bid);
        if (blk) {
            for (int sid : blk->succ_ids) {
                auto phi_it = ssa_.block_phis.find(sid);
                if (phi_it == ssa_.block_phis.end()) continue;
                auto *sblk = cfg.FindBlock(sid);
                if (!sblk) continue;
                // 找到 bid 在 sblk->pred_ids 中的位置
                int pred_idx = -1;
                for (int j = 0; j < (int)sblk->pred_ids.size(); ++j) {
                    if (sblk->pred_ids[j] == bid) { pred_idx = j; break; }
                }
                if (pred_idx < 0) continue;
                for (auto &phi : phi_it->second) {
                    auto sit = stack_top.find(phi.var_id);
                    if (sit != stack_top.end()) {
                        if (pred_idx < (int)phi.arg_versions.size())
                            phi.arg_versions[pred_idx] = sit->second;
                    }
                }
            }
        }

        // 递归子节点
        auto cit = dom_tree_children_.find(bid);
        if (cit != dom_tree_children_.end()) {
            for (int child : cit->second) rename_dfs(child);
        }
    };
    rename_dfs(cfg.entry_id);
}

// ── 主入口 ──────────────────────────────────────────────────────────────
SSAFunction SSABuilder::Build(const CFGFunction &cfg) {
    ssa_ = {};
    ssa_.func_name = cfg.func_name;
    ssa_.next_version = 0;
    var_name_to_id_.clear();
    var_id_to_name_.clear();
    var_def_blocks_.clear();
    dom_tree_children_.clear();

    // 为参数创建初始版本
    std::unordered_map<int, std::string> idx_to_name;
    for (const auto &[pname, pidx] : cfg.param_indices) idx_to_name[pidx] = pname;
    int param_count = (int)cfg.param_indices.size();
    ssa_.param_versions.resize(param_count, -1);
    for (int i = 0; i < param_count; ++i) {
        auto it = idx_to_name.find(i);
        const std::string &pname = (it != idx_to_name.end()) ? it->second : std::to_string(i);
        GetVarId(pname);  // 注册变量
        int ver = NewVersion(pname, cfg.entry_id);
        ssa_.param_versions[i] = ver;
    }

    // Step 0: 收集定义块 + 构建支配树
    CollectDefBlocks(cfg);
    BuildDomTree(cfg);

    // Cytron Step 1: 插入 φ 节点
    InsertPhis(cfg);

    // Cytron Step 2: 变量重命名
    RenameVariables(cfg);

    return std::move(ssa_);
}

// ── DumpToString ─────────────────────────────────────────────────────────
std::string SSAFunction::DumpToString() const {
    std::ostringstream oss;
    oss << "SSA[" << func_name << "] params=[";
    for (size_t i = 0; i < param_versions.size(); ++i) {
        if (i > 0) oss << ",";
        oss << param_versions[i];
    }
    oss << "]\n";
    for (const auto &[bid, phis] : block_phis) {
        for (const auto &phi : phis) {
            oss << "  phi in block " << bid << ": " << phi.var_name << "v" << phi.result_version
                << " = φ(";
            for (size_t i = 0; i < phi.arg_versions.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << (phi.arg_versions[i] >= 0 ? std::to_string(phi.arg_versions[i]) : "?");
            }
            oss << ")\n";
        }
    }
    // 打印各变量的所有版本
    for (const auto &[vname, vers] : var_all_versions) {
        oss << "  var " << vname << ": ";
        for (size_t i = 0; i < vers.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << "v" << vers[i] << "@" << version_to_block.at(vers[i]);
        }
        oss << "\n";
    }
    return oss.str();
}

}// namespace fakelua
