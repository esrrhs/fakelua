#include "compile/unified_type_analyzer.h"
#include "compile/syntax_tree.h"
#include <algorithm>
#include <queue>
#include <unordered_set>

namespace fakelua {

// ─────────────────────────────────────────────────────────────────────────
// 主分析入口 — 流敏感工作表
// ─────────────────────────────────────────────────────────────────────────
void UnifiedTypeAnalyzer::Analyze(const std::string &func_name,
                                   const SyntaxTreeInterfacePtr &func_block,
                                   const CFGFunction &cfg,
                                   SSAFunction &ssa,
                                   InferResult &ir,
                                   int bitmask,
                                   const ParamAssumption &param_assumptions) {
    cur_func_name_ = func_name;

    if (!ir.shape_registry) {
        ir.shape_registry = std::make_shared<ShapeRegistry>();
    }
    registry_ = ir.shape_registry.get();

    // 构建变量名 → SSA 版本映射
    auto name_ver = BuildVarNameVersionMap(ssa);

    // ── 主分析：跑流敏感工作表 ────────────────────────────────────────
    // 同时生成 version_types（param names → 类型）供 InferExprType 使用
    TypeEnv version_types;
    // 初始化参数版本类型（保留接口兼容）
    for (size_t i = 0; i < ssa.param_versions.size(); ++i) {
        int ver = ssa.param_versions[i];
        auto pit = param_assumptions.find(ver);
        version_types[ver] = (pit != param_assumptions.end())
            ? pit->second : SSATypeInfo{T_DYNAMIC, -1};
    }

    // 跑流敏感工作表，返回每个块的 out_env
    std::unordered_map<int, VarEnv> block_outs;
    if (bitmask == -1) {
        block_outs = RunWorklist(cfg, ssa, param_assumptions, ir);
        // 合并 version_types → ir.ssa_version_types（legacy 兼容：ComputeVarFinalShapes 等依赖）
        for (const auto &[ver, ty] : version_types) {
            ir.ssa_version_types[ver] = ty;
        }

        // 推导所有 AST 节点的类型：对每个块，使用其 out_env 作为上下文调用 InferExprType
        // （某些参数可能是顶层块入口定义的；块内赋值按 TransferStmt 已写回 env）
        if (func_block) {
            // 用工作表得到的 env，对主块出口处的变量进行全局 env 合并：
            // 由于主块只有一个 exit 块，我们合并所有块 out_env = 整个函数的"可能出口状态"
            VarEnv merged;
            for (const auto &[bid, env] : block_outs) {
                for (const auto &[v, t] : env) {
                    auto it = merged.find(v);
                    if (it == merged.end()) merged[v] = t;
                    else it->second = Meet(it->second, t);
                }
            }
            WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
                if (!node) return;
                switch (node->Type()) {
                    case SyntaxTreeType::Exp:
                    case SyntaxTreeType::Var:
                    case SyntaxTreeType::PrefixExp:
                    case SyntaxTreeType::TableConstructor:
                    case SyntaxTreeType::FunctionCall:
                    case SyntaxTreeType::ExpList: {
                        auto ty = InferExprType(node, ssa, version_types, ir, name_ver, &merged);
                        ir.main_ssa_types[node.get()] = ty;
                        ir.node_ssa_version[node.get()] = -1;
                        break;
                    }
                    default:
                        break;
                }
            });
        }

        // 额外：用工作表得到的 env 直接对每个 block 的顶层 stmt 做更精确的类型传播
        // （这是核心的流敏感部分：块内赋值后的变量来源类型）
        for (const auto &b : cfg.blocks) {
            if (b.stmts.empty()) continue;
            // 块的 in_env = meet of preds' out_env
            VarEnv env;
            bool first_pred = true;
            for (int pid : b.pred_ids) {
                auto pit = block_outs.find(pid);
                if (pit == block_outs.end()) continue;
                if (first_pred) {
                    env = pit->second;
                    first_pred = false;
                } else {
                    env = MeetEnv(env, pit->second);
                }
            }
            if (first_pred) {
                // entry block: seed param types from ssa
                for (const auto &[pname, pidx] : cfg.param_indices) {
                    int ver = (pidx < (int)ssa.param_versions.size()) ? ssa.param_versions[pidx] : -1;
                    if (ver >= 0) {
                        auto vit = version_types.find(ver);
                        if (vit != version_types.end())
                            env[pname] = vit->second;
                    }
                }
            }
            // 对块内每个 stmt 做 Transfer（更新 env），然后填充 main_ssa_types
            for (const auto &s : b.stmts) {
                env = TransferStmt(s, env, ssa, version_types);
            }
            PopulateNodeTypesFromStmts(b.stmts, env, ssa, version_types, ir);
            // 保存块的 out_env
            block_outs[b.id] = env;
        }

        // 计算 widest shapes
        ComputeVarFinalShapes(ssa, ir);
        ComputeCtorTargetShapes(func_block, ssa, ir);
    } else {
        // ── 特化分析：Per-bitmask 快照 ──────────────────────────────────
        // 简化：沿用原来的启发式，但在其上使用 param_assumptions 填充的 env
        auto &snap = ir.spec_ssa_snapshots[func_name];
        if ((size_t)bitmask >= snap.size()) snap.resize((size_t)bitmask + 1);

        // 构建 env：从 param_assumptions 派生 var_name → type
        VarEnv base_env;
        for (const auto &[pname, pidx] : cfg.param_indices) {
            int ver = (pidx < (int)ssa.param_versions.size()) ? ssa.param_versions[pidx] : -1;
            if (ver >= 0) {
                auto pit = param_assumptions.find(ver);
                if (pit != param_assumptions.end())
                    base_env[pname] = pit->second;
                else
                    base_env[pname] = {T_DYNAMIC, -1};
            } else {
                base_env[pname] = {T_DYNAMIC, -1};
            }
        }

        // 对 Per-bitmask，直接用 main 块的 env 来推导
        if (func_block) {
            WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
                if (!node) return;
                switch (node->Type()) {
                    case SyntaxTreeType::Exp:
                    case SyntaxTreeType::Var:
                    case SyntaxTreeType::PrefixExp:
                    case SyntaxTreeType::TableConstructor:
                    case SyntaxTreeType::FunctionCall:
                    case SyntaxTreeType::ExpList: {
                        auto ty = InferExprType(node, ssa, version_types, ir, name_ver, &base_env);
                        snap[(size_t)bitmask][node.get()] = ty;
                        break;
                    }
                    default:
                        break;
                }
            });

            // 第二轮：对 local 声明和赋值，用快照已知类型回写 RHS 表达式的类型。
            std::function<void(const SyntaxTreeInterfacePtr&)> propagate;
            propagate = [&](const SyntaxTreeInterfacePtr &node) {
                if (!node) return;
                if (node->Type() == SyntaxTreeType::LocalVar) {
                    auto *lv = static_cast<SyntaxTreeLocalVar *>(node.get());
                    auto nl = lv->Namelist();
                    auto el = lv->Explist();
                    if (!nl || nl->Type() != SyntaxTreeType::NameList) return;
                    auto nlist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(nl);
                    auto exps = el ? std::dynamic_pointer_cast<SyntaxTreeExplist>(el) : nullptr;
                    auto &names = nlist->Names();
                    for (size_t i = 0; i < names.size(); ++i) {
                        if (exps && i < exps->Exps().size()) {
                            auto &exp = exps->Exps()[i];
                            auto it = snap[(size_t)bitmask].find(exp.get());
                            if (it != snap[(size_t)bitmask].end() && IsNumericInferredType(it->second.type)) {
                                snap[(size_t)bitmask][exp.get()] = it->second;
                            }
                        }
                    }
                } else if (node->Type() == SyntaxTreeType::Assign) {
                    auto *as = static_cast<SyntaxTreeAssign *>(node.get());
                    auto vl = as->Varlist() ? std::dynamic_pointer_cast<SyntaxTreeVarlist>(as->Varlist()) : nullptr;
                    auto el = as->Explist() ? std::dynamic_pointer_cast<SyntaxTreeExplist>(as->Explist()) : nullptr;
                    if (!vl || !el) return;
                    auto &vars = vl->Vars();
                    auto &exps = el->Exps();
                    for (size_t i = 0; i < vars.size() && i < exps.size(); ++i) {
                        auto it = snap[(size_t)bitmask].find(exps[i].get());
                        if (it != snap[(size_t)bitmask].end() && IsNumericInferredType(it->second.type)) {
                            snap[(size_t)bitmask][exps[i].get()] = it->second;
                        }
                    }
                } else if (node->Type() == SyntaxTreeType::ForLoop) {
                    auto *fl = static_cast<SyntaxTreeForLoop *>(node.get());
                    propagate(fl->Block());
                } else if (node->Type() == SyntaxTreeType::Block) {
                    auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
                    if (blk) for (auto &s : blk->Stmts()) propagate(s);
                } else if (node->Type() == SyntaxTreeType::If) {
                    auto ifn = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
                    if (ifn) { propagate(ifn->Block()); if (auto eb = ifn->ElseBlock()) propagate(eb); }
                }
            };
            propagate(func_block);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────
// 流敏感工作表（§6 简化版：per-block VarEnv，按 var_name merge）
// ─────────────────────────────────────────────────────────────────────────
std::unordered_map<int, UnifiedTypeAnalyzer::VarEnv>
UnifiedTypeAnalyzer::RunWorklist(const CFGFunction &cfg,
                                  const SSAFunction &ssa,
                                  const ParamAssumption &param_assumptions,
                                  const InferResult &ir) {
    // 退化为单块情况（无分支）
    if (cfg.blocks.size() <= 1) {
        VarEnv env;
        // seed param types
        for (const auto &[pname, pidx] : cfg.param_indices) {
            int ver = (pidx < (int)ssa.param_versions.size()) ? ssa.param_versions[pidx] : -1;
            if (ver >= 0) {
                auto pit = param_assumptions.find(ver);
                env[pname] = (pit != param_assumptions.end())
                    ? pit->second : SSATypeInfo{T_DYNAMIC, -1};
            }
        }
        for (const auto &b : cfg.blocks) {
            for (const auto &s : b.stmts) {
                env = TransferStmt(s, env, ssa, TypeEnv{});
            }
        }
        std::unordered_map<int, VarEnv> result;
        for (const auto &b : cfg.blocks) result[b.id] = env;
        return result;
    }

    // 多块情况：需要拓扑序（reverse-postorder from entry）
    // 1) BFS/DFS 遍历可达块，按完成时间逆序排列
    std::vector<int> rpo_order;
    std::unordered_set<int> visited;
    auto block_succs = [&](int bid) -> const std::vector<int>& {
        static const std::vector<int> empty;
        auto *bp = cfg.FindBlock(bid);
        return bp ? bp->succ_ids : empty;
    };
    std::function<void(int)> dfs = [&](int bid) {
        if (visited.count(bid)) return;
        visited.insert(bid);
        for (int sid : block_succs(bid)) dfs(sid);
        rpo_order.push_back(bid);
    };
    dfs(cfg.entry_id);
    std::reverse(rpo_order.begin(), rpo_order.end());

    // 2) in/out envs
    std::unordered_map<int, VarEnv> in_envs, out_envs;

    // 3) 初始 env：seed params in entry block
    in_envs[cfg.entry_id] = {};
    for (const auto &[pname, pidx] : cfg.param_indices) {
        int ver = (pidx < (int)ssa.param_versions.size()) ? ssa.param_versions[pidx] : -1;
        if (ver >= 0) {
            auto pit = param_assumptions.find(ver);
            in_envs[cfg.entry_id][pname] = (pit != param_assumptions.end())
                ? pit->second : SSATypeInfo{T_DYNAMIC, -1};
        } else {
            in_envs[cfg.entry_id][pname] = {T_DYNAMIC, -1};
        }
    }

    // 4) 迭代直到不动点
    std::deque<int> worklist(rpo_order.begin(), rpo_order.end());
    int iter_count = 0;
    constexpr int kMaxIters = 64;
    constexpr int kWidenIter = 3;
    bool widened = false;

    while (!worklist.empty() && iter_count < kMaxIters) {
        int bid = worklist.front();
        worklist.pop_front();
        iter_count++;

        VarEnv new_in;
        bool first_pred = true;
        for (int pid : cfg.FindBlock(bid)->pred_ids) {
            auto pit = out_envs.find(pid);
            if (pit == out_envs.end()) continue;
            if (first_pred) {
                new_in = pit->second;
                first_pred = false;
            } else {
                new_in = MeetEnv(new_in, pit->second);
            }
        }
        if (first_pred && bid != cfg.entry_id) {
            new_in = {}; // unreachable block (shouldn't happen if dfs found all)
        } else if (bid == cfg.entry_id) {
            new_in = in_envs[cfg.entry_id];
        }

        VarEnv old_out = out_envs[bid];
        VarEnv new_out = new_in;
        const TypeEnv empty_vt;
        for (const auto &s : cfg.FindBlock(bid)->stmts) {
            new_out = TransferStmt(s, new_out, ssa, empty_vt);
        }

        // widening on loops: 如果迭代超限且字段集是 record，则提早退化
        if (iter_count > kWidenIter && !widened) {
            // 如果发现某个块 env 大小在缩小，停止增长：这里用 noop
            // 因为 var_env 大小恒为「已知变量数」，不会无限扩张；
            // record shape 字段增长问题由 ShapeRegistry::Widen 特化时单独处理
        }

        in_envs[bid] = new_in;
        if (new_out != old_out) {
            out_envs[bid] = new_out;
            // push succ
            for (int sid : block_succs(bid)) {
                if (std::find(worklist.begin(), worklist.end(), sid) == worklist.end())
                    worklist.push_back(sid);
            }
        }
    }

    return out_envs;
}

// ─────────────────────────────────────────────────────────────────────────
// Meet of two envs (按 var_name meet)
// ─────────────────────────────────────────────────────────────────────────
UnifiedTypeAnalyzer::VarEnv UnifiedTypeAnalyzer::MeetEnv(const VarEnv &a, const VarEnv &b) {
    VarEnv r = a;
    for (const auto &[v, t] : b) {
        auto it = r.find(v);
        if (it == r.end()) r[v] = t;
        else it->second = UnifiedTypeAnalyzer::Meet(it->second, t);
    }
    return r;
}

// ─────────────────────────────────────────────────────────────────────────
// Transfer function — 单语句 env 转移
// -------------------------------------------------------------------
// 目前只处理能简单推导出变量类型的赋值和 local 声明；
// 控制流语句（If/While/For）在外部 CFG 已经分裂，不在此处递归。
// ─────────────────────────────────────────────────────────────────────────
UnifiedTypeAnalyzer::VarEnv UnifiedTypeAnalyzer::TransferStmt(
    const SyntaxTreeInterfacePtr &stmt,
    const VarEnv &env,
    const SSAFunction &ssa,
    const TypeEnv &version_types) {

    if (!stmt) return env;
    VarEnv out = env;
    const VarEnv *envp = &out;
    // InferExprType 此时不需要 ir，但我们保留调用兼容（传 dummy）
    InferResult dummy_ir;

    switch (stmt->Type()) {
        case SyntaxTreeType::LocalVar: {
            auto *lv = static_cast<SyntaxTreeLocalVar *>(stmt.get());
            auto nl = lv->Namelist();
            auto el = lv->Explist();
            if (!nl || nl->Type() != SyntaxTreeType::NameList) return out;
            auto nlist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(nl);
            if (!nlist) return out;
            auto exps = el ? std::dynamic_pointer_cast<SyntaxTreeExplist>(el) : nullptr;
            auto &names = nlist->Names();
            for (size_t i = 0; i < names.size(); ++i) {
                if (!exps || i >= exps->Exps().size()) {
                    out[names[i]] = {T_NIL, -1};
                    continue;
                }
                auto &exp = exps->Exps()[i];
                auto ty = InferExprType(exp, ssa, version_types, dummy_ir, {}, envp);
                out[names[i]] = ty;
            }
            break;
        }
        case SyntaxTreeType::Assign: {
            auto *as = static_cast<SyntaxTreeAssign *>(stmt.get());
            auto vl = as->Varlist() ? std::dynamic_pointer_cast<SyntaxTreeVarlist>(as->Varlist()) : nullptr;
            auto el = as->Explist() ? std::dynamic_pointer_cast<SyntaxTreeExplist>(as->Explist()) : nullptr;
            if (!vl || !el) return out;
            auto &vars = vl->Vars();
            auto &exps = el->Exps();
            for (size_t i = 0; i < vars.size() && i < exps.size(); ++i) {
                if (vars[i]->Type() != SyntaxTreeType::Var) continue;
                auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(vars[i]);
                if (!v || v->GetVarKind() != VarKind::kSimple) continue;
                const std::string &vn = v->GetName();
                auto ty = InferExprType(exps[i], ssa, version_types, dummy_ir, {}, envp);
                out[vn] = ty;
            }
            break;
        }
        case SyntaxTreeType::ForLoop: {
            auto *fl = static_cast<SyntaxTreeForLoop *>(stmt.get());
            // For-loop 游标：初始类型由 begin/end/step 决定。
            // 数值游标默认推导为 T_INT（除非 step/end 是浮点常量）。
            // 完整做法需要看 ExpBegin/ExpEnd/ExpStep 的字面量。
            // 简化：如果有浮点数值，则为 T_FLOAT；否则 T_INT。
            InferredType cursor_type = T_INT;
            auto check_exp = [&](const SyntaxTreeInterfacePtr &e) {
                if (!e || e->Type() != SyntaxTreeType::Exp) return;
                auto *ep = static_cast<SyntaxTreeExp *>(e.get());
                if (ep->GetExpKind() == ExpKind::kNumber) {
                    const auto &v = ep->ExpValue();
                    if (v.find('.') != std::string::npos || v.find('e') != std::string::npos || v.find('E') != std::string::npos)
                        cursor_type = T_FLOAT;
                }
            };
            check_exp(fl->ExpBegin());
            check_exp(fl->ExpEnd());
            check_exp(fl->ExpStep());
            out[fl->Name()] = {cursor_type, -1};
            break;
        }
        case SyntaxTreeType::ForIn: {
            // for-in 游标：从迭代器推导过于复杂，保守为 T_DYNAMIC
            auto *fi = static_cast<SyntaxTreeForIn *>(stmt.get());
            if (fi->Namelist() && fi->Namelist()->Type() == SyntaxTreeType::NameList) {
                auto nlist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(fi->Namelist());
                if (nlist) {
                    for (const auto &n : nlist->Names())
                        out[n] = {T_DYNAMIC, -1};
                }
            }
            break;
        }
        default:
            break;
    }
    return out;
}

// ─────────────────────────────────────────────────────────────────────────
// PopulateNodeTypesFromBlock：用块的 env 上下文，填充该块内所有 AST 子节点的 main_ssa_types
// 与 Analyze 中的 WalkSyntaxTree 同理，但用 per-block env 而非全局 env。
// ─────────────────────────────────────────────────────────────────────────
void UnifiedTypeAnalyzer::PopulateNodeTypesFromStmts(
    const std::vector<SyntaxTreeInterfacePtr> &stmts,
    const VarEnv &env,
    const SSAFunction &ssa,
    const TypeEnv &version_types,
    InferResult &ir) {

    for (const auto &stmt : stmts) {
        if (!stmt) continue;
        WalkSyntaxTree(stmt, [&](const SyntaxTreeInterfacePtr &node) {
        if (!node) return;
        switch (node->Type()) {
            case SyntaxTreeType::Exp:
            case SyntaxTreeType::Var:
            case SyntaxTreeType::PrefixExp:
            case SyntaxTreeType::TableConstructor:
            case SyntaxTreeType::FunctionCall:
            case SyntaxTreeType::ExpList: {
                auto ty = InferExprType(node, ssa, version_types, ir, {}, &env);
                ir.main_ssa_types[node.get()] = ty;
                ir.node_ssa_version[node.get()] = -1;
                break;
            }
            default:
                break;
        }
        });
    }
}

// ─────────────────────────────────────────────────────────────────────────
// 变量名 → SSA 版本映射
// ─────────────────────────────────────────────────────────────────────────
UnifiedTypeAnalyzer::VarNameToVersion UnifiedTypeAnalyzer::BuildVarNameVersionMap(const SSAFunction &ssa) {
    VarNameToVersion m;
    for (auto &[name, vers] : ssa.var_all_versions) {
        if (!vers.empty()) m[name] = vers.back();
    }
    return m;
}

// ─────────────────────────────────────────────────────────────────────────
// 递归推导表达式类型
// ─────────────────────────────────────────────────────────────────────────
SSATypeInfo UnifiedTypeAnalyzer::InferExprType(
    const SyntaxTreeInterfacePtr &expr,
    const SSAFunction &ssa,
    const TypeEnv &version_types,
    const InferResult &ir,
    const std::unordered_map<std::string, int> &name_ver,
    const VarEnv *local_env) {

    if (!expr) return {T_UNKNOWN, -1};

    switch (expr->Type()) {
        case SyntaxTreeType::Exp: {
            auto *e = static_cast<SyntaxTreeExp *>(expr.get());
            switch (e->GetExpKind()) {
                case ExpKind::kNumber: {
                    const auto &v = e->ExpValue();
                    if (v.find('.') == std::string::npos && v.find('e') == std::string::npos && v.find('E') == std::string::npos)
                        return {T_INT, -1};
                    return {T_FLOAT, -1};
                }
                case ExpKind::kString: return {T_STRING, -1};
                case ExpKind::kNil: return {T_NIL, -1};
                case ExpKind::kTrue:
                case ExpKind::kFalse: return {T_BOOL, -1};
                case ExpKind::kBinop: {
                    auto lt = InferExprType(e->Left(), ssa, version_types, ir, name_ver, local_env);
                    auto rt = InferExprType(e->Right(), ssa, version_types, ir, name_ver, local_env);
                    return {ShapeRegistry::MeetType(lt.type, rt.type), -1};
                }
                case ExpKind::kUnop: {
                    return InferExprType(e->Right(), ssa, version_types, ir, name_ver, local_env);
                }
                case ExpKind::kPrefixExp: {
                    return InferExprType(e->Right(), ssa, version_types, ir, name_ver, local_env);
                }
                case ExpKind::kTableConstructor: {
                    int shape_id = BuildShapeFromCtor(expr, ssa, version_types, ir, local_env);
                    if (shape_id >= 0) return {T_RECORD, shape_id};
                    return {T_DYNAMIC, -1};
                }
                default:
                    return {T_DYNAMIC, -1};
            }
        }
        case SyntaxTreeType::Var: {
            auto *v = static_cast<SyntaxTreeVar *>(expr.get());
            if (v->GetVarKind() == VarKind::kSimple) {
                const std::string &vname = v->GetName();
                // 优先 local_env
                if (local_env) {
                    auto env_it = local_env->find(vname);
                    if (env_it != local_env->end()) return env_it->second;
                }
                // 2. SSA use_versions
                auto vit = ssa.use_versions.find(expr.get());
                if (vit != ssa.use_versions.end()) {
                    auto tit = version_types.find(vit->second);
                    if (tit != version_types.end()) return tit->second;
                }
                // 3. name_ver fallback
                auto nit = name_ver.find(vname);
                if (nit != name_ver.end()) {
                    auto tit = version_types.find(nit->second);
                    if (tit != version_types.end()) return tit->second;
                }
            }
            return {T_DYNAMIC, -1};
        }
        case SyntaxTreeType::TableConstructor: {
            int shape_id = BuildShapeFromCtor(expr, ssa, version_types, ir, local_env);
            if (shape_id >= 0) return {T_RECORD, shape_id};
            return {T_DYNAMIC, -1};
        }
        case SyntaxTreeType::PrefixExp: {
            auto *pe = static_cast<SyntaxTreePrefixexp *>(expr.get());
            return InferExprType(pe->GetValue(), ssa, version_types, ir, name_ver, local_env);
        }
        case SyntaxTreeType::FunctionCall: {
            return {T_DYNAMIC, -1};
        }
        default:
            return {T_DYNAMIC, -1};
    }
}

int UnifiedTypeAnalyzer::BuildShapeFromCtor(const SyntaxTreeInterfacePtr &tc,
                                           const SSAFunction &ssa,
                                           const TypeEnv &version_types,
                                           const InferResult &ir,
                                           const VarEnv *local_env) {
    if (!tc || tc->Type() != SyntaxTreeType::TableConstructor) return -1;
    auto *tc_ptr = static_cast<SyntaxTreeTableconstructor *>(tc.get());
    if (!tc_ptr->Fieldlist()) return -1;
    auto *fl = static_cast<SyntaxTreeFieldlist *>(tc_ptr->Fieldlist().get());

    ShapeType shape;
    shape.is_open = false;

    for (auto &field_node : fl->Fields()) {
        if (!field_node || field_node->Type() != SyntaxTreeType::Field) continue;
        auto *fp = static_cast<SyntaxTreeField *>(field_node.get());

        FieldDef fd;
        if (fp->GetFieldKind() == FieldKind::kObject) {
            fd.name = fp->Name();
            fd.c_field_name = ToSafeCFieldName(fp->Name());
        } else {
            if (fp->GetFieldKind() == FieldKind::kArray && fp->Key() == nullptr) {
                fd.name = std::to_string(shape.fields.size() + 1);
                fd.is_int_key = true;
                fd.c_field_name = "_int_" + fd.name;
            } else {
                auto *ke = fp->Key() && fp->Key()->Type() == SyntaxTreeType::Exp
                    ? static_cast<SyntaxTreeExp *>(fp->Key().get()) : nullptr;
                if (ke && ke->GetExpKind() == ExpKind::kNumber) {
                    fd.name = ke->ExpValue();
                    fd.is_int_key = (fd.name.find('.') == std::string::npos);
                    fd.c_field_name = fd.is_int_key ? "_int_" + fd.name : "_float_" + fd.name;
                } else {
                    continue;
                }
            }
        }

        auto val_ty = InferExprType(fp->Value(), ssa, version_types, ir, {}, local_env);
        fd.type = val_ty.type;
        shape.fields.push_back(fd);
    }

    if (shape.fields.empty()) return -1;
    return registry_->Intern(std::move(shape));
}

SSATypeInfo UnifiedTypeAnalyzer::Meet(const SSATypeInfo &a, const SSATypeInfo &b) {
    if (a == b) return a;
    SSATypeInfo r;
    r.type = ShapeRegistry::MeetType(a.type, b.type);
    r.shape_id = (a.shape_id == b.shape_id) ? a.shape_id : -1;
    return r;
}

void UnifiedTypeAnalyzer::ComputeVarFinalShapes(const SSAFunction &ssa, InferResult &ir) {
    if (!registry_) return;

    for (auto &[var_name, versions] : ssa.var_all_versions) {
        if (versions.empty()) continue;
        SSATypeInfo merged{T_UNKNOWN, -1};
        for (int ver : versions) {
            auto it = ir.ssa_version_types.find(ver);
            if (it != ir.ssa_version_types.end()) {
                merged = Meet(merged, it->second);
            }
        }
        if (IsRecordInferredType(merged.type) && merged.shape_id >= 0) {
            ir.var_final_shapes[var_name] = merged.shape_id;
        }
    }
}

void UnifiedTypeAnalyzer::ComputeCtorTargetShapes(const SyntaxTreeInterfacePtr &func_block,
                                                   const SSAFunction &ssa,
                                                   InferResult &ir) {
    if (!func_block) return;
    auto &ctor_map = ir.ctor_target_shapes;
    ctor_map.clear();

    LinkExprToTargetShape(func_block, "", ir.var_final_shapes, ctor_map);
}

void UnifiedTypeAnalyzer::LinkExprToTargetShape(
    const SyntaxTreeInterfacePtr &node,
    const std::string &target_name,
    const std::unordered_map<std::string, int> &var_final_shapes,
    std::unordered_map<const SyntaxTreeInterface *, int> &ctor_target_shapes) {

    if (!node) return;
    if (node->Type() == SyntaxTreeType::TableConstructor) {
        auto it = var_final_shapes.find(target_name);
        if (it != var_final_shapes.end()) {
            ctor_target_shapes[node.get()] = it->second;
        }
    }
    switch (node->Type()) {
        case SyntaxTreeType::Block: {
            auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            for (auto &s : blk->Stmts()) {
                if (s->Type() == SyntaxTreeType::LocalVar) {
                    auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(s);
                    auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
                    auto el = lv->Explist() ? std::dynamic_pointer_cast<SyntaxTreeExplist>(lv->Explist()) : nullptr;
                    const auto &names = nl->Names();
                    const auto &exps = el ? el->Exps() : std::vector<SyntaxTreeInterfacePtr>{};
                    for (size_t i = 0; i < names.size() && i < exps.size(); ++i) {
                        LinkExprToTargetShape(exps[i], names[i], var_final_shapes, ctor_target_shapes);
                    }
                }
            }
            break;
        }
        case SyntaxTreeType::TableConstructor:
            break;
        default:
            break;
    }
}

void UnifiedTypeAnalyzer::BuildSummary(const std::string & /*func_name*/,
                                        const SyntaxTreeInterfacePtr & /*func_block*/,
                                        const SSAFunction & /*ssa*/,
                                        const TypeEnv & /*version_types*/,
                                        InferResult & /*ir*/) {
    // stub
}

SSATypeInfo UnifiedTypeAnalyzer::ApplyCallSummary(const std::string & /*callee_name*/,
                                                   const InferResult & /*ir*/) const {
    return {T_DYNAMIC, -1};
}

// ── 发现可特化参数 ──────────────────────────────────────────────────────
std::vector<UnifiedTypeAnalyzer::SpecParam>
UnifiedTypeAnalyzer::FindSpecializableParams(const SyntaxTreeInterfacePtr &func_block,
                                              const CFGFunction &cfg,
                                              const SSAFunction &ssa,
                                              const InferResult &ir) {
    std::vector<SpecParam> result;
    if (!func_block) return result;

    std::unordered_set<std::string> param_names;
    // 收集所有参数
    for (auto &[pname, pidx] : cfg.param_indices) {
        param_names.insert(pname);
    }

    std::unordered_set<std::string> expr_vars;

    bool include_cmp = false;
    std::function<void(const SyntaxTreeInterfacePtr&, int)> collect_vars;
    collect_vars = [&](const SyntaxTreeInterfacePtr &node, int depth) {
        if (!node) return;
        switch (node->Type()) {
            case SyntaxTreeType::Var: {
                auto *v = static_cast<SyntaxTreeVar *>(node.get());
                if (v->GetVarKind() == VarKind::kSimple) {
                    const std::string &name = v->GetName();
                    if (param_names.count(name) && depth > 0) {
                        expr_vars.insert(name);
                    }
                }
                break;
            }
            case SyntaxTreeType::PrefixExp: {
                auto *pe = static_cast<SyntaxTreePrefixexp *>(node.get());
                if (pe->GetPrefixKind() == PrefixExpKind::kVar && pe->GetValue()) {
                    collect_vars(pe->GetValue(), depth);
                } else if (pe->GetPrefixKind() == PrefixExpKind::kExp && pe->GetValue()) {
                    collect_vars(pe->GetValue(), depth);
                }
                break;
            }
            case SyntaxTreeType::Exp: {
                auto *e = static_cast<SyntaxTreeExp *>(node.get());
                if (e->GetExpKind() == ExpKind::kBinop) {
                    auto *bin = e->Op() ? dynamic_cast<SyntaxTreeBinop*>(e->Op().get()) : nullptr;
                    auto kind = bin ? bin->GetOpKind() : BinOpKind::kPlus;
                    bool is_arith = (kind == BinOpKind::kPlus || kind == BinOpKind::kMinus ||
                                     kind == BinOpKind::kStar || kind == BinOpKind::kSlash ||
                                     kind == BinOpKind::kDoubleSlash || kind == BinOpKind::kMod ||
                                     kind == BinOpKind::kPow || kind == BinOpKind::kBitAnd ||
                                     kind == BinOpKind::kBitOr || kind == BinOpKind::kXor ||
                                     kind == BinOpKind::kLeftShift || kind == BinOpKind::kRightShift);
                    bool is_compare = (kind == BinOpKind::kEqual || kind == BinOpKind::kNotEqual ||
                                       kind == BinOpKind::kLess || kind == BinOpKind::kMore ||
                                       kind == BinOpKind::kLessEqual || kind == BinOpKind::kMoreEqual);
                    bool is_logic = (kind == BinOpKind::kAnd || kind == BinOpKind::kOr);
                    if (is_arith || (include_cmp && (is_compare || is_logic))) {
                        if (e->Left()) collect_vars(e->Left(), depth + 1);
                        if (e->Right()) collect_vars(e->Right(), depth + 1);
                    }
                } else if (e->GetExpKind() == ExpKind::kUnop) {
                    if (e->Right()) collect_vars(e->Right(), depth + 1);
                } else if (e->GetExpKind() == ExpKind::kPrefixExp) {
                    // PrefixExp 只是"包装"节点：depth 应保持不变。只有当来自 Binop
                    // 内部时 depth>0，才能把 Var 标记为可特化参数；顶层 PrefixExp
                    // (如 return a) 不应特化。
                    if (e->Right()) collect_vars(e->Right(), depth);
                }
                break;
            }
            case SyntaxTreeType::FunctionCall: {
                auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
                if (fc && fc->Args()) {
                    auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
                    if (!args) break;
                    if (args->GetArgsKind() == ArgsKind::kExpList && args->Explist()) {
                        auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(args->Explist());
                        if (el) for (auto &e : el->Exps()) collect_vars(e, depth);
                    } else if (args->GetArgsKind() == ArgsKind::kTableConstructor && args->Tableconstructor()) {
                        collect_vars(args->Tableconstructor(), depth);
                    }
                }
                break;
            }
            default:
                break;
        }
    };

    std::function<void(const SyntaxTreeInterfacePtr&)> visit_all;
    visit_all = [&](const SyntaxTreeInterfacePtr &node) {
        if (!node) return;
        if (node->Type() == SyntaxTreeType::Exp) {
            auto *e = static_cast<SyntaxTreeExp *>(node.get());
            if (e->GetExpKind() == ExpKind::kBinop || e->GetExpKind() == ExpKind::kUnop || e->GetExpKind() == ExpKind::kPrefixExp) {
                collect_vars(node, 0);
            }
            return;
        }
        switch (node->Type()) {
            case SyntaxTreeType::Block: {
                auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
                for (auto &s : blk->Stmts()) visit_all(s);
                break;
            }
            case SyntaxTreeType::Return: {
                auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(node);
                if (ret->Explist()) visit_all(ret->Explist());
                break;
            }
            case SyntaxTreeType::Assign: {
                auto as = std::dynamic_pointer_cast<SyntaxTreeAssign>(node);
                if (as->Varlist()) visit_all(as->Varlist());
                if (as->Explist()) visit_all(as->Explist());
                break;
            }
            case SyntaxTreeType::VarList: {
                auto vl = std::dynamic_pointer_cast<SyntaxTreeVarlist>(node);
                for (auto &v : vl->Vars()) visit_all(v);
                break;
            }
            case SyntaxTreeType::ExpList: {
                auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
                for (auto &e : el->Exps()) visit_all(e);
                break;
            }
            case SyntaxTreeType::Var: {
                auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(node);
                if (v->GetExp()) visit_all(v->GetExp());
                if (v->GetPrefixexp()) visit_all(v->GetPrefixexp());
                break;
            }
            case SyntaxTreeType::FunctionCall: {
                auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
                if (fc->prefixexp()) visit_all(fc->prefixexp());
                if (fc->Args()) visit_all(fc->Args());
                break;
            }
            case SyntaxTreeType::TableConstructor: {
                auto tc = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node);
                if (tc->Fieldlist()) visit_all(tc->Fieldlist());
                break;
            }
            case SyntaxTreeType::FieldList: {
                auto fl = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
                for (auto &f : fl->Fields()) visit_all(f);
                break;
            }
            case SyntaxTreeType::Field: {
                auto f = std::dynamic_pointer_cast<SyntaxTreeField>(node);
                if (f->Key()) visit_all(f->Key());
                if (f->Value()) visit_all(f->Value());
                break;
            }
            case SyntaxTreeType::While: {
                auto w = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
                if (w->Exp()) visit_all(w->Exp());
                if (w->Block()) visit_all(w->Block());
                break;
            }
            case SyntaxTreeType::Repeat: {
                auto r = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
                if (r->Block()) visit_all(r->Block());
                if (r->Exp()) visit_all(r->Exp());
                break;
            }
            case SyntaxTreeType::If: {
                auto ifn = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
                if (ifn->Exp()) visit_all(ifn->Exp());
                if (ifn->Block()) visit_all(ifn->Block());
                if (ifn->ElseIfs()) visit_all(ifn->ElseIfs());
                if (ifn->ElseBlock()) visit_all(ifn->ElseBlock());
                break;
            }
            case SyntaxTreeType::ElseIfList: {
                auto el = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(node);
                for (auto &e : el->ElseifExps()) visit_all(e);
                for (auto &b : el->ElseifBlocks()) visit_all(b);
                break;
            }
            case SyntaxTreeType::ForLoop: {
                auto fl = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
                if (fl->ExpBegin()) visit_all(fl->ExpBegin());
                if (fl->ExpEnd()) visit_all(fl->ExpEnd());
                if (fl->ExpStep()) visit_all(fl->ExpStep());
                if (fl->Block()) visit_all(fl->Block());
                break;
            }
            case SyntaxTreeType::ForIn: {
                auto fi = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
                if (fi->Namelist()) visit_all(fi->Namelist());
                if (fi->Explist()) visit_all(fi->Explist());
                if (fi->Block()) visit_all(fi->Block());
                break;
            }
            case SyntaxTreeType::FuncBody: {
                auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(node);
                if (fb->Block()) visit_all(fb->Block());
                break;
            }
            case SyntaxTreeType::LocalVar: {
                auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
                if (lv->Namelist()) visit_all(lv->Namelist());
                if (lv->Explist()) visit_all(lv->Explist());
                break;
            }
            default:
                break;
        }
    };
    include_cmp = false;
    visit_all(func_block);

    if (!expr_vars.empty()) {
        include_cmp = true;
        visit_all(func_block);
    }

    for (const auto &name : expr_vars) {
        auto it = cfg.param_indices.find(name);
        if (it != cfg.param_indices.end()) {
            result.push_back({it->second, true, false});
        }
    }
    std::sort(result.begin(), result.end(),
              [](const SpecParam &a, const SpecParam &b) { return a.param_index < b.param_index; });
    result.erase(std::unique(result.begin(), result.end(),
                              [](const SpecParam &a, const SpecParam &b) {
                                  return a.param_index == b.param_index && a.is_math == b.is_math;
                              }),
                 result.end());

    return result;
}

}// namespace fakelua
