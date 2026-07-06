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
                                   InferResult &ir) {
    cur_func_name_ = func_name;

    if (!ir.shape_registry) {
        ir.shape_registry = std::make_shared<ShapeRegistry>();
    }
    registry_ = ir.shape_registry.get();

    // 构建变量名 → SSA 版本映射
    auto name_ver = BuildVarNameVersionMap(ssa);

    // ── 主分析：跑流敏感工作表（规范 §6）────────────────────────────
    TypeEnv version_types;
    // 初始化参数版本类型（默认 T_DYNAMIC）
    for (size_t i = 0; i < ssa.param_versions.size(); ++i) {
        version_types[ssa.param_versions[i]] = {T_DYNAMIC, -1};
    }

    // 跑流敏感工作表，返回每个块的 out_env
    std::unordered_map<int, VarEnv> block_outs = RunWorklist(cfg, ssa, ir);

    // ── φ 节点类型推导 ──────────────────────────────────────────────────
    // 对每个有 φ 块的变量，从各前驱块的 out_env 取类型，Meet 得到 φ 结果类型
    for (auto &kv : ssa.block_phis) {
        int bid = kv.first;
        auto &phis = kv.second;
        auto *blk = cfg.FindBlock(bid);
        if (!blk) continue;
        for (auto &phi : phis) {
            SSATypeInfo merged{T_UNKNOWN, -1};
            bool first = true;
            for (int pred_id : blk->pred_ids) {
                auto pred_env_it = block_outs.find(pred_id);
                if (pred_env_it == block_outs.end()) continue;
                auto var_it = pred_env_it->second.find(phi.var_name);
                if (var_it == pred_env_it->second.end()) continue;
                if (first) { merged = var_it->second; first = false; }
                else merged = Meet(merged, var_it->second, registry_);
            }
            if (first) merged = {T_DYNAMIC, -1};  // 无前驱有该变量
            version_types[phi.result_version] = merged;
            ssa.version_types[phi.result_version] = merged;
        }
    }

    // 合并 version_types → ir.ssa_version_types
    for (const auto &[ver, ty] : version_types) {
        ir.ssa_version_types[ver] = ty;
    }

    // 用工作表得到的 env 直接对每个 block 的顶层 stmt 做精确的类型传播
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

    // 推导所有 AST 节点的类型（使用合并的全局 env）
    if (func_block) {
        VarEnv merged;
        for (const auto &[bid, env] : block_outs) {
            for (const auto &[v, t] : env) {
                auto it = merged.find(v);
                if (it == merged.end()) merged[v] = t;
                else it->second = Meet(it->second, t, registry_);
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

    // 计算 widest shapes
    ComputeVarFinalShapes(ssa, ir);
    ComputeCtorTargetShapes(func_block, ssa, ir);
}

// ─────────────────────────────────────────────────────────────────────────
// 流敏感工作表（§6 简化版：per-block VarEnv，按 var_name merge）
// ─────────────────────────────────────────────────────────────────────────
std::unordered_map<int, UnifiedTypeAnalyzer::VarEnv>
UnifiedTypeAnalyzer::RunWorklist(const CFGFunction &cfg,
                                  const SSAFunction &ssa,
                                  const InferResult &ir) {
    // 退化为单块情况（无分支）
    if (cfg.blocks.size() <= 1) {
        VarEnv env;
        // seed param types (default T_DYNAMIC)
        for (const auto &[pname, pidx] : cfg.param_indices) {
            env[pname] = {T_DYNAMIC, -1};
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

    // 3) 初始 env：seed params in entry block (default T_DYNAMIC)
    in_envs[cfg.entry_id] = {};
    for (const auto &[pname, pidx] : cfg.param_indices) {
        in_envs[cfg.entry_id][pname] = {T_DYNAMIC, -1};
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

        // widening on loops: 迭代超限时，对 env 中的 record shape 调用 Widen
        if (iter_count > kWidenIter && registry_) {
            for (auto &[vname, vtype] : new_out) {
                if (IsRecordInferredType(vtype.type) && vtype.shape_id >= 0) {
                    int new_shape_id = registry_->Widen(vtype.shape_id, iter_count);
                    if (new_shape_id < 0) {
                        // 退化为 T_DYNAMIC
                        vtype.type = T_DYNAMIC;
                        vtype.shape_id = -1;
                    } else {
                        vtype.shape_id = new_shape_id;
                    }
                }
            }
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
                if (!v) continue;
                auto ty = InferExprType(exps[i], ssa, version_types, dummy_ir, {}, envp);
                if (v->GetVarKind() == VarKind::kSimple) {
                    // 简单变量赋值：local x = ... 或 x = ...
                    out[v->GetName()] = ty;
                } else if (v->GetVarKind() == VarKind::kDot || v->GetVarKind() == VarKind::kSquare) {
                    // 字段写入：a.b = v 或 a[b] = v
                    // 规范 §5.2.4：更新 base 变量的 shape
                    const std::string &field_name = v->GetName();
                    auto *base_pe = static_cast<SyntaxTreePrefixexp *>(v->GetPrefixexp().get());
                    if (base_pe && base_pe->GetPrefixKind() == PrefixExpKind::kVar) {
                        auto *base_var = static_cast<SyntaxTreeVar *>(base_pe->GetValue().get());
                        if (base_var && base_var->GetVarKind() == VarKind::kSimple) {
                            const std::string &base_name = base_var->GetName();
                            auto base_it = out.find(base_name);
                            if (base_it != out.end() && base_it->second.shape_id >= 0 && registry_) {
                                // 规范 §5.2.4：字段写入 a.b = v
                                ShapeType shape = registry_->Get(base_it->second.shape_id);
                                FieldDef *fd = shape.FindFieldMut(field_name);
                                if (fd) {
                                    // 已有字段：合一类型
                                    fd->type = ShapeRegistry::MeetType(fd->type, ty.type);
                                } else {
                                    // 新字段
                                    if (!shape.is_open) {
                                        // 封闭 record 加新字段 → 退化为开放
                                        shape.is_open = true;
                                    }
                                    FieldDef new_fd;
                                    new_fd.name = field_name;
                                    new_fd.c_field_name = ToSafeCFieldName(field_name);
                                    new_fd.type = ty.type;
                                    new_fd.optional = false;
                                    shape.fields.push_back(new_fd);
                                }
                                base_it->second.shape_id = registry_->Intern(std::move(shape));
                            }
                        }
                    }
                }
            }
            break;
        }
        case SyntaxTreeType::ForLoop: {
            auto *fl = static_cast<SyntaxTreeForLoop *>(stmt.get());
            // 数值 for 游标类型推断：如果 env 中 ExpEnd/ExpStep 对应变量为 T_FLOAT，
            // 或字面量含小数点，则游标 T_FLOAT；否则 T_INT。
            InferredType cursor_type = T_INT;
            std::function<void(const SyntaxTreeInterfacePtr&)> check_exp;
            check_exp = [&](const SyntaxTreeInterfacePtr &e) {
                if (!e) return;
                if (e->Type() == SyntaxTreeType::Exp) {
                    auto *ep = static_cast<SyntaxTreeExp *>(e.get());
                    if (ep->GetExpKind() == ExpKind::kNumber) {
                        const auto &v = ep->ExpValue();
                        if (v.find('.') != std::string::npos || v.find('e') != std::string::npos || v.find('E') != std::string::npos)
                            cursor_type = T_FLOAT;
                    } else if (ep->GetExpKind() == ExpKind::kPrefixExp && ep->Right()) {
                        check_exp(ep->Right());
                    }
                } else if (e->Type() == SyntaxTreeType::Var) {
                    auto *v = static_cast<SyntaxTreeVar *>(e.get());
                    if (v->GetVarKind() == VarKind::kSimple) {
                        auto it = out.find(v->GetName());
                        if (it != out.end() && it->second.type == T_FLOAT)
                            cursor_type = T_FLOAT;
                    }
                } else if (e->Type() == SyntaxTreeType::PrefixExp) {
                    auto *pe = static_cast<SyntaxTreePrefixexp *>(e.get());
                    if (pe->GetValue()) check_exp(pe->GetValue());
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
    InferResult &ir,
    std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> *out_map) {

    if (!out_map) out_map = &ir.main_ssa_types;
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
                    (*out_map)[node.get()] = ty;
                    if (out_map == &ir.main_ssa_types)
                        ir.node_ssa_version[node.get()] = -1;
                    break;
                }
                case SyntaxTreeType::ForLoop: {
                    // 推导 for-loop 游标节点本身类型（CGen 用来决定 i 的声明类型）
                    auto *fl = static_cast<SyntaxTreeForLoop *>(node.get());
                    InferredType cursor_type = T_INT;
                    std::function<void(const SyntaxTreeInterfacePtr&)> check_exp;
                    check_exp = [&](const SyntaxTreeInterfacePtr &e) {
                        if (!e) return;
                        if (e->Type() == SyntaxTreeType::Exp) {
                            auto *ep = static_cast<SyntaxTreeExp *>(e.get());
                            if (ep->GetExpKind() == ExpKind::kNumber) {
                                const auto &v = ep->ExpValue();
                                if (v.find('.') != std::string::npos || v.find('e') != std::string::npos || v.find('E') != std::string::npos)
                                    cursor_type = T_FLOAT;
                            } else if (ep->GetExpKind() == ExpKind::kPrefixExp && ep->Right()) {
                                check_exp(ep->Right());
                            }
                        } else if (e->Type() == SyntaxTreeType::Var) {
                            auto *v = static_cast<SyntaxTreeVar *>(e.get());
                            if (v->GetVarKind() == VarKind::kSimple) {
                                auto it = env.find(v->GetName());
                                if (it != env.end() && it->second.type == T_FLOAT)
                                    cursor_type = T_FLOAT;
                            }
                        } else if (e->Type() == SyntaxTreeType::PrefixExp) {
                            auto *pe = static_cast<SyntaxTreePrefixexp *>(e.get());
                            if (pe->GetValue()) check_exp(pe->GetValue());
                        }
                    };
                    check_exp(fl->ExpBegin());
                    check_exp(fl->ExpEnd());
                    check_exp(fl->ExpStep());
                    (*out_map)[node.get()] = {cursor_type, -1};
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
                    // expr 是 Exp 包装节点，实际 TableConstructor 在其 Right() 中
                    int shape_id = BuildShapeFromCtor(e->Right(), ssa, version_types, ir);
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
                if (vit != ssa.use_versions.end() && !vit->second.empty()) {
                    auto tit = version_types.find(vit->second[0]);
                    if (tit != version_types.end()) return tit->second;
                }
                // 3. name_ver fallback
                auto nit = name_ver.find(vname);
                if (nit != name_ver.end()) {
                    auto tit = version_types.find(nit->second);
                    if (tit != version_types.end()) return tit->second;
                }
            } else if (v->GetVarKind() == VarKind::kDot || v->GetVarKind() == VarKind::kSquare) {
                // 规范 §5.2.3：字段读取 a.b / a[b]
                const std::string &field_name = v->GetName();
                auto *base_pe = static_cast<SyntaxTreePrefixexp *>(v->GetPrefixexp().get());
                if (base_pe && base_pe->GetPrefixKind() == PrefixExpKind::kVar) {
                    auto *base_var = static_cast<SyntaxTreeVar *>(base_pe->GetValue().get());
                    if (base_var && base_var->GetVarKind() == VarKind::kSimple) {
                        // 查找 base 变量的类型
                        SSATypeInfo base_type{T_DYNAMIC, -1};
                        auto env_it = local_env->find(base_var->GetName());
                        if (env_it != local_env->end()) {
                            base_type = env_it->second;
                        }
                        if (base_type.shape_id >= 0 && registry_) {
                            const ShapeType &shape = registry_->Get(base_type.shape_id);
                            const FieldDef *fd = shape.FindField(field_name);
                            if (fd) {
                                // ★ 命中 → 走偏移
                                return {fd->type, -1};
                            } else if (shape.is_open) {
                                // 开放 record 无此字段 → 走 hash
                                return {T_DYNAMIC, -1};
                            } else {
                                // 封闭 record 无此字段 → Lua 语义返回 nil
                                return {T_NIL, -1};
                            }
                        }
                    }
                }
                return {T_DYNAMIC, -1};
            }
            return {T_DYNAMIC, -1};
        }
        case SyntaxTreeType::TableConstructor: {
            int shape_id = BuildShapeFromCtor(expr, ssa, version_types, ir);
            if (shape_id >= 0) return {T_RECORD, shape_id};
            return {T_DYNAMIC, -1};
        }
        case SyntaxTreeType::PrefixExp: {
            auto *pe = static_cast<SyntaxTreePrefixexp *>(expr.get());
            return InferExprType(pe->GetValue(), ssa, version_types, ir, name_ver, local_env);
        }
        case SyntaxTreeType::FunctionCall: {
            // 尝试从函数摘要获取返回类型（过程间分析，规范 §7）
            auto *fc = static_cast<SyntaxTreeFunctioncall *>(expr.get());
            const std::string &callee = fc->prefixexp() && fc->prefixexp()->Type() == SyntaxTreeType::Var
                ? static_cast<SyntaxTreeVar *>(fc->prefixexp().get())->GetName()
                : "";
            if (!callee.empty()) {
                auto it = ir.func_summaries.find(callee);
                if (it != ir.func_summaries.end() && !it->second.being_built
                    && it->second.ret_type.type != T_UNKNOWN && it->second.ret_type.type != T_DYNAMIC) {
                    return it->second.ret_type;
                }
            }
            return {T_DYNAMIC, -1};
        }
        default:
            return {T_DYNAMIC, -1};
    }
}

int UnifiedTypeAnalyzer::BuildShapeFromCtor(const SyntaxTreeInterfacePtr &tc,
                                           const SSAFunction &ssa,
                                           const TypeEnv &version_types,
                                           const InferResult &ir) {
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

        auto val_ty = InferExprType(fp->Value(), ssa, version_types, ir, {});
        fd.type = val_ty.type;
        shape.fields.push_back(fd);
    }

    if (shape.fields.empty()) return -1;
    return registry_->Intern(std::move(shape));
}

SSATypeInfo UnifiedTypeAnalyzer::Meet(const SSATypeInfo &a, const SSATypeInfo &b, ShapeRegistry *reg) {
    if (a == b) return a;
    SSATypeInfo r;
    r.type = ShapeRegistry::MeetType(a.type, b.type);
    if (a.shape_id >= 0 && b.shape_id >= 0 && reg) {
        // 两个都是 record shape → 调用 registry 的 Meet 合并字段
        r.shape_id = reg->Meet(a.shape_id, b.shape_id);
    } else if (a.shape_id >= 0) {
        r.shape_id = a.shape_id;
    } else if (b.shape_id >= 0) {
        r.shape_id = b.shape_id;
    } else {
        r.shape_id = -1;
    }
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
                merged = Meet(merged, it->second, registry_);
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
                    if (!nl) continue;
                    const auto &names = nl->Names();
                    const auto &exps = el ? el->Exps() : std::vector<SyntaxTreeInterfacePtr>{};
                    for (size_t i = 0; i < names.size() && i < exps.size(); ++i) {
                        LinkExprToTargetShape(exps[i], names[i], var_final_shapes, ctor_target_shapes);
                    }
                } else if (s->Type() == SyntaxTreeType::Function || s->Type() == SyntaxTreeType::LocalFunction) {
                    auto fb = (s->Type() == SyntaxTreeType::Function)
                        ? std::dynamic_pointer_cast<SyntaxTreeFunction>(s)->Funcbody()
                        : std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(s)->Funcbody();
                    auto fb_ptr = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(fb);
                    if (fb_ptr && fb_ptr->Block())
                        LinkExprToTargetShape(fb_ptr->Block(), target_name, var_final_shapes, ctor_target_shapes);
                }
            }
            break;
        }
        case SyntaxTreeType::Exp: {
            auto *e = static_cast<SyntaxTreeExp *>(node.get());
            LinkExprToTargetShape(e->Right(), target_name, var_final_shapes, ctor_target_shapes);
            break;
        }
        case SyntaxTreeType::ExpList: {
            auto *el = static_cast<SyntaxTreeExplist *>(node.get());
            for (auto &exp : el->Exps()) LinkExprToTargetShape(exp, target_name, var_final_shapes, ctor_target_shapes);
            break;
        }
        case SyntaxTreeType::TableConstructor:
            break;
        default:
            break;
    }
}

// 从表达式结构推导返回类型（在 main_ssa_types 无法确定时回退）
// 对于 `return n+1` 这类纯参数+算术表达式，返回 T_INT 作为"数值"标记
static InferredType DeriveExprTypeForRet(const SyntaxTreeInterfacePtr &expr,
                                          const std::vector<SSATypeInfo> &param_types,
                                          const InferResult &ir) {
    if (!expr) return T_UNKNOWN;
    auto *inner = expr.get();
    // 只解包 Exp(kPrefixExp) 和 PrefixExp 包装，不碰 Binop/Number
    while (inner && inner->Type() == SyntaxTreeType::PrefixExp) {
        auto *pe = static_cast<SyntaxTreePrefixexp *>(inner);
        inner = pe->GetValue() ? pe->GetValue().get() : nullptr;
    }
    while (inner && inner->Type() == SyntaxTreeType::Exp &&
           static_cast<SyntaxTreeExp *>(inner)->GetExpKind() == ExpKind::kPrefixExp) {
        auto *ep = static_cast<SyntaxTreeExp *>(inner);
        inner = ep->Right() ? ep->Right().get() : nullptr;
    }
    // 再次解包 PrefixExp（Exp(kPrefixExp) 的 Right 可能是 PrefixExp）
    while (inner && inner->Type() == SyntaxTreeType::PrefixExp) {
        auto *pe = static_cast<SyntaxTreePrefixexp *>(inner);
        inner = pe->GetValue() ? pe->GetValue().get() : nullptr;
    }
    if (!inner) return T_UNKNOWN;
    if (inner->Type() == SyntaxTreeType::FunctionCall) {
        // 函数调用：查找 func_summaries 获取返回类型
        auto *fc = static_cast<SyntaxTreeFunctioncall *>(inner);
        std::string callee;
        if (fc->prefixexp() && fc->prefixexp()->Type() == SyntaxTreeType::Var) {
            callee = static_cast<SyntaxTreeVar *>(fc->prefixexp().get())->GetName();
        } else if (fc->prefixexp() && fc->prefixexp()->Type() == SyntaxTreeType::PrefixExp) {
            auto *pe = static_cast<SyntaxTreePrefixexp *>(fc->prefixexp().get());
            if (pe->GetValue() && pe->GetValue()->Type() == SyntaxTreeType::Var)
                callee = static_cast<SyntaxTreeVar *>(pe->GetValue().get())->GetName();
        }
        if (!callee.empty()) {
            auto it = ir.func_summaries.find(callee);
            if (it != ir.func_summaries.end() && it->second.ret_type.type != T_UNKNOWN && it->second.ret_type.type != T_DYNAMIC)
                return it->second.ret_type.type;
        }
        return T_UNKNOWN;
    }
    if (inner->Type() == SyntaxTreeType::Var) {
        auto *v = static_cast<SyntaxTreeVar *>(inner);
        if (v->GetVarKind() == VarKind::kSimple) return T_INT;
        return T_UNKNOWN;
    }
    if (inner->Type() == SyntaxTreeType::Exp) {
        auto *ep = static_cast<SyntaxTreeExp *>(inner);
        if (ep->GetExpKind() == ExpKind::kNumber) {
            const auto &val = ep->ExpValue();
            return (val.find('.') != std::string::npos || val.find('e') != std::string::npos || val.find('E') != std::string::npos)
                ? T_FLOAT : T_INT;
        }
        if (ep->GetExpKind() == ExpKind::kBinop) {
            auto *bin = ep->Op() ? dynamic_cast<SyntaxTreeBinop*>(ep->Op().get()) : nullptr;
            if (!bin) return T_UNKNOWN;
            auto kind = bin->GetOpKind();
            bool is_arith = (kind == BinOpKind::kPlus || kind == BinOpKind::kMinus ||
                             kind == BinOpKind::kStar || kind == BinOpKind::kSlash ||
                             kind == BinOpKind::kDoubleSlash || kind == BinOpKind::kMod ||
                             kind == BinOpKind::kPow || kind == BinOpKind::kBitAnd ||
                             kind == BinOpKind::kBitOr || kind == BinOpKind::kXor ||
                             kind == BinOpKind::kLeftShift || kind == BinOpKind::kRightShift);
            InferredType lt = DeriveExprTypeForRet(ep->Left(), param_types, ir);
            InferredType rt = DeriveExprTypeForRet(ep->Right(), param_types, ir);
            if (is_arith && IsNumericInferredType(lt) && IsNumericInferredType(rt))
                return (lt == T_FLOAT || rt == T_FLOAT) ? T_FLOAT : T_INT;
            if ((kind == BinOpKind::kAnd || kind == BinOpKind::kOr) &&
                IsNumericInferredType(lt) && IsNumericInferredType(rt))
                return (lt == T_FLOAT || rt == T_FLOAT) ? T_FLOAT : T_INT;
            return T_UNKNOWN;
        }
        if (ep->GetExpKind() == ExpKind::kUnop) {
            auto *unop = ep->Op() ? dynamic_cast<SyntaxTreeUnop*>(ep->Op().get()) : nullptr;
            if (unop && (unop->GetOpKind() == UnOpKind::kMinus || unop->GetOpKind() == UnOpKind::kBitNot))
                return DeriveExprTypeForRet(ep->Right(), param_types, ir);
            return T_UNKNOWN;
        }
    }
    return T_UNKNOWN;
}

void UnifiedTypeAnalyzer::BuildSummary(const std::string &func_name,
                                        const SyntaxTreeInterfacePtr &func_block,
                                        const SSAFunction &ssa,
                                        const TypeEnv &version_types,
                                        InferResult &ir) {
    FuncSummary &s = ir.func_summaries[func_name];
    s.func_name = func_name;
    s.param_types.resize(ssa.param_versions.size(), {T_DYNAMIC, -1});
    s.param_escape.resize(ssa.param_versions.size(), false);

    // 从 main_ssa_types 收集返回类型
    SSATypeInfo merged_ret{T_UNKNOWN, -1};
    if (func_block) {
        WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
            if (!node || node->Type() != SyntaxTreeType::Return) return;
            auto *ret = static_cast<SyntaxTreeReturn *>(node.get());
            if (!ret->Explist()) return;
            auto el_ptr = ret->Explist()->Type() == SyntaxTreeType::ExpList
                ? std::dynamic_pointer_cast<SyntaxTreeExplist>(ret->Explist())
                : std::shared_ptr<SyntaxTreeExplist>();
            if (!el_ptr || el_ptr->Exps().empty()) return;
            auto it = ir.main_ssa_types.find(el_ptr->Exps()[0].get());
            if (it != ir.main_ssa_types.end()) {
                if (merged_ret.type == T_UNKNOWN) merged_ret = it->second;
                else merged_ret = Meet(merged_ret, it->second, registry_);
            }
        });
    }
    if (merged_ret.type == T_UNKNOWN) merged_ret = {T_NIL, -1};
    // 如果返回类型是 T_DYNAMIC（参数类型未知），尝试从表达式结构推导
    // 对于 `return n+1` 这类算术表达式，返回类型与参数一致（视为 T_INT 默认）
    if (merged_ret.type == T_DYNAMIC && func_block) {
        WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
            if (!node || node->Type() != SyntaxTreeType::Return) return;
            auto *ret = static_cast<SyntaxTreeReturn *>(node.get());
            if (!ret->Explist()) return;
            auto el_ptr = ret->Explist()->Type() == SyntaxTreeType::ExpList
                ? std::dynamic_pointer_cast<SyntaxTreeExplist>(ret->Explist())
                : std::shared_ptr<SyntaxTreeExplist>();
            if (!el_ptr || el_ptr->Exps().empty()) return;
            InferredType derived = DeriveExprTypeForRet(el_ptr->Exps()[0], s.param_types, ir);
            if (derived != T_UNKNOWN && derived != T_DYNAMIC) {
                merged_ret = {derived, -1};
            }
        });
    }
    s.ret_type = merged_ret;
}

}// namespace fakelua
