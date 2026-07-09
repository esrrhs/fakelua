#include "compile/infer/unified_type_analyzer.h"
#include "compile/syntax_tree.h"
#include <algorithm>
#include <cstddef>
#include <queue>
#include <unordered_set>

namespace fakelua {

// ─────────────────────────────────────────────────────────────────────────
// HM 类型 ↔ SSATypeInfo 转换
// ─────────────────────────────────────────────────────────────────────────

namespace {
bool IsPrimHmKind(TypeKind k) {
    switch (k) {
        case TypeKind::TY_INT:
        case TypeKind::TY_FLOAT:
        case TypeKind::TY_STRING:
        case TypeKind::TY_BOOL:
        case TypeKind::TY_NIL:
        case TypeKind::TY_DYNAMIC:
            return true;
        default:
            return false;
    }
}

InferredType HmPrimToInferred(TypeKind k) {
    switch (k) {
        case TypeKind::TY_INT:
            return T_INT;
        case TypeKind::TY_FLOAT:
            return T_FLOAT;
        case TypeKind::TY_STRING:
            return T_STRING;
        case TypeKind::TY_BOOL:
            return T_BOOL;
        case TypeKind::TY_NIL:
            return T_NIL;
        case TypeKind::TY_DYNAMIC:
            return T_DYNAMIC;
        default:
            return T_DYNAMIC;
    }
}

TypeKind InferredToHmPrim(InferredType t) {
    switch (t) {
        case T_INT:
            return TypeKind::TY_INT;
        case T_FLOAT:
            return TypeKind::TY_FLOAT;
        case T_STRING:
            return TypeKind::TY_STRING;
        case T_BOOL:
            return TypeKind::TY_BOOL;
        case T_NIL:
            return TypeKind::TY_NIL;
        case T_DYNAMIC:
            return TypeKind::TY_DYNAMIC;
        default:
            return TypeKind::TY_DYNAMIC;
    }
}
}// namespace

Type *UnifiedTypeAnalyzer::SsaInfoToHm(const SSATypeInfo &info) {
    if (IsRecordInferredType(info.type) && info.shape_id >= 0 && registry_) {
        // 构造一个 HM record shape（深度拷贝 registry 中的 shape）。
        const ShapeType &s = registry_->Get(info.shape_id);
        Type *t = HmType::MakeRecord(hm_arena_, s.is_open);
        for (const auto &f: s.fields) {
            RecordField hf;
            hf.name = hm_arena_.AllocString(f.name);
            hf.c_field_name = hm_arena_.AllocString(f.c_field_name);
            hf.type = SsaInfoToHm({f.type, -1});
            hf.optional = f.optional;
            hf.is_int_key = f.is_int_key;
            t->fields.push_back(hf);
        }
        return t;
    }
    return HmType::MakePrim(hm_arena_, InferredToHmPrim(info.type));
}

SSATypeInfo UnifiedTypeAnalyzer::HmToSsaInfo(Type *t) {
    if (!t) return {T_DYNAMIC, -1};
    t = TypeVarTable::Prune(t);
    if (!t) return {T_DYNAMIC, -1};

    if (t->kind == TypeKind::TY_VAR) {
        // 自由变量：退化为 T_DYNAMIC。
        return {T_DYNAMIC, -1};
    }
    if (IsPrimHmKind(t->kind)) {
        return {HmPrimToInferred(t->kind), -1};
    }
    if (t->kind == TypeKind::TY_RECORD || t->kind == TypeKind::TY_RECORD_OPEN) {
        // 构建 ShapeType。
        ShapeType shape;
        shape.is_open = t->is_open;
        for (const auto &f: t->fields) {
            FieldDef fd;
            fd.name = f.name ? f.name : "";
            fd.c_field_name = f.c_field_name ? f.c_field_name : ToSafeCFieldName(fd.name);
            fd.optional = f.optional;
            fd.is_int_key = f.is_int_key;
            Type *ft = TypeVarTable::Prune(f.type);
            if (ft == nullptr) {
                fd.type = T_DYNAMIC;
            } else if (IsPrimHmKind(ft->kind)) {
                fd.type = HmPrimToInferred(ft->kind);
            } else if (ft->kind == TypeKind::TY_VAR) {
                fd.type = T_DYNAMIC;
            } else if (ft->kind == TypeKind::TY_RECORD || ft->kind == TypeKind::TY_RECORD_OPEN) {
                // 嵌套 record：递归注入。
                fd.type = T_RECORD;
                // shape_id 占位由 InjectHmRecordIntoRegistry 处理（回填）。
                fd.type = T_DYNAMIC;// 保守退化，避免嵌套 record 的复杂性
            } else {
                fd.type = T_DYNAMIC;
            }
            shape.fields.push_back(std::move(fd));
        }
        if (shape.fields.empty()) return {T_DYNAMIC, -1};
        int sid = registry_->Intern(std::move(shape));
        return {T_RECORD, sid};
    }
    return {T_DYNAMIC, -1};
}

int UnifiedTypeAnalyzer::InjectHmRecordIntoRegistry(Type *record_ty) {
    if (!record_ty || !registry_) return -1;
    record_ty = TypeVarTable::Prune(record_ty);
    if (!record_ty) return -1;
    SSATypeInfo info = HmToSsaInfo(record_ty);
    return info.shape_id;
}

static void RecordSsaVersionType(const SyntaxTreeInterfacePtr &node, const SSAFunction &ssa, const SSATypeInfo &ty, InferResult &ir);

// ─────────────────────────────────────────────────────────────────────────
// 主分析入口 — 流敏感工作表
// ─────────────────────────────────────────────────────────────────────────
void UnifiedTypeAnalyzer::Analyze(const std::string &func_name, const SyntaxTreeInterfacePtr &func_block, const CFGFunction &cfg, SSAFunction &ssa, InferResult &ir, const std::unordered_map<int, SSATypeInfo> *assumed_param_types) {
    cur_func_name_ = func_name;

    if (!ir.shape_registry) {
        ir.shape_registry = std::make_shared<ShapeRegistry>();
    }
    registry_ = ir.shape_registry.get();

    // 重置每函数的状态（HM arena、变量表、参数映射）。
    // hm_arena_ 在每次 Analyze 开始时整体 reset，所有 Type* 指针失效。
    // 同时必须清除 ir.func_summaries 中的悬空 HM 指针（它们在上一轮分析
    // 中被写入，但底层 arena 已被 Reset）。
    for (auto &[fn, fs]: ir.func_summaries) {
        fs.param_hm_types.clear();
        fs.ret_hm_type = nullptr;
        fs.must_use_hm = false;
    }
    hm_arena_.Reset();
    hm_table_.Reset();
    cur_param_hm_vars_.clear();

    // 为每个参数创建 HM 参数类型变量（多态：每个调用点独立实例化）。
    // param_indices 中 pname → pidx。
    for (const auto &[pname, pidx]: cfg.param_indices) {
        Type *v = hm_table_.NewVar();
        cur_param_hm_vars_[pname] = v;
    }

    // 构建变量名 → SSA 版本映射
    auto name_ver = BuildVarNameVersionMap(ssa);

    // ── 主分析：跑流敏感工作表（规范 §6）────────────────────────────
    TypeEnv version_types;
    // 初始化参数版本类型
    for (size_t i = 0; i < ssa.param_versions.size(); ++i) {
        int ver = ssa.param_versions[i];
        if (assumed_param_types && assumed_param_types->contains(ver)) {
            version_types[ver] = assumed_param_types->at(ver);
        } else {
            version_types[ver] = {T_DYNAMIC, -1};
        }
    }

    // 跑流敏感工作表，返回每个块的 out_env
    std::unordered_map<int, VarEnv> block_outs = RunWorklist(cfg, ssa, version_types, ir);

    // ── φ 节点类型推导 ──────────────────────────────────────────────────
    // 对每个有 φ 块的变量，从各前驱块的 out_env 取类型，Meet 得到 φ 结果类型
    for (auto &kv: ssa.block_phis) {
        int bid = kv.first;
        auto &phis = kv.second;
        auto *blk = cfg.FindBlock(bid);
        if (!blk) continue;
        for (auto &phi: phis) {
            SSATypeInfo merged{T_UNKNOWN, -1};
            bool first = true;
            for (int pred_id: blk->pred_ids) {
                auto pred_env_it = block_outs.find(pred_id);
                if (pred_env_it == block_outs.end()) continue;
                auto var_it = pred_env_it->second.find(phi.var_name);
                if (var_it == pred_env_it->second.end()) continue;
                if (first) {
                    merged = var_it->second;
                    first = false;
                } else
                    merged = Meet(merged, var_it->second, registry_);
            }
            if (first) merged = {T_DYNAMIC, -1};// 无前驱有该变量
            version_types[phi.result_version] = merged;
            ssa.version_types[phi.result_version] = merged;
        }
    }

    // 合并 version_types → ir.ssa_version_types
    for (const auto &[ver, ty]: version_types) {
        ir.ssa_version_types[ver] = ty;
    }

    // 用工作表得到的 env 直接对每个 block 的顶层 stmt 做精确的类型传播
    for (const auto &b: cfg.blocks) {
        if (b.stmts.empty()) continue;
        // 块的 in_env = meet of preds' out_env
        VarEnv env;
        bool first_pred = true;
        for (int pid: b.pred_ids) {
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
            for (const auto &[pname, pidx]: cfg.param_indices) {
                int ver = (pidx < (int) ssa.param_versions.size()) ? ssa.param_versions[pidx] : -1;
                if (ver >= 0) {
                    auto vit = version_types.find(ver);
                    if (vit != version_types.end()) env[pname] = vit->second;
                }
            }
        }
        // 对块内每个 stmt 做 Transfer（更新 env），然后填充 main_ssa_types
        ConstEnv block_const_env;// §12.6 常量传播
        for (const auto &s: b.stmts) {
            env = TransferStmtConst(s, env, block_const_env, ssa, version_types);
        }
        PopulateNodeTypesFromStmts(b.stmts, env, block_const_env, ssa, version_types, ir);
        // 保存块的 out_env
        block_outs[b.id] = env;
    }

    // 推导所有 AST 节点的类型（使用合并的全局 env）
    if (func_block) {
        VarEnv merged;
        for (const auto &[bid, env]: block_outs) {
            for (const auto &[v, t]: env) {
                auto it = merged.find(v);
                if (it == merged.end()) merged[v] = t;
                else
                    it->second = Meet(it->second, t, registry_);
            }
        }
        for (const auto &[v, t]: merged) {
            if (IsRecordInferredType(t.type) && t.shape_id >= 0) {
                ir.var_final_shapes[v] = t.shape_id;
            }
        }
        if (func_name == kInitFunctionName) {
            for (const auto &[v, t]: merged) {
                ir.global_const_vars[v] = t.type;
            }
        }
        WalkSyntaxTreePruned(func_block, [&](const SyntaxTreeInterfacePtr &node) -> bool {
            if (!node) return false;
            if (node->Type() == SyntaxTreeType::Function || node->Type() == SyntaxTreeType::LocalFunction) {
                return false;
            }
            switch (node->Type()) {
                case SyntaxTreeType::Exp:
                case SyntaxTreeType::Var:
                case SyntaxTreeType::PrefixExp:
                case SyntaxTreeType::TableConstructor:
                case SyntaxTreeType::FunctionCall:
                case SyntaxTreeType::ExpList: {
                    auto ty = InferExprType(node, ssa, version_types, ir, name_ver, &merged);
                    ir.main_ssa_types[node.get()] = ty;
                    RecordSsaVersionType(node, ssa, ty, ir);
                    break;
                }
                default:
                    break;
            }
            return true;
        });
    }

    // 计算 widest shapes
    ComputeVarFinalShapes(ssa, ir);
    ComputeCtorTargetShapes(func_block, ssa, ir);

    // §8 逃逸分析
    ComputeEscape(func_name, func_block, cfg, ir.escape_vars[func_name]);
}

// ─────────────────────────────────────────────────────────────────────────
// 流敏感工作表（§6 简化版：per-block VarEnv，按 var_name merge）
// ─────────────────────────────────────────────────────────────────────────
std::unordered_map<int, UnifiedTypeAnalyzer::VarEnv> UnifiedTypeAnalyzer::RunWorklist(const CFGFunction &cfg, const SSAFunction &ssa, const TypeEnv &version_types, const InferResult &ir) {
    // 退化为单块情况（无分支）
    if (cfg.blocks.size() <= 1) {
        VarEnv env;
        ConstEnv const_env;// §12.6 常量传播
        // seed param types
        for (const auto &[pname, pidx]: cfg.param_indices) {
            int ver = (pidx < (int) ssa.param_versions.size()) ? ssa.param_versions[pidx] : -1;
            SSATypeInfo pty = {T_DYNAMIC, -1};
            if (ver >= 0) {
                auto vit = version_types.find(ver);
                if (vit != version_types.end()) {
                    pty = vit->second;
                }
            }
            env[pname] = pty;
        }
        for (const auto &b: cfg.blocks) {
            for (const auto &s: b.stmts) {
                env = TransferStmtConst(s, env, const_env, ssa, TypeEnv{});
            }
        }
        std::unordered_map<int, VarEnv> result;
        for (const auto &b: cfg.blocks) result[b.id] = env;
        return result;
    }

    // 多块情况：需要拓扑序（reverse-postorder from entry）
    // 1) BFS/DFS 遍历可达块，按完成时间逆序排列
    std::vector<int> rpo_order;
    std::unordered_set<int> visited;
    auto block_succs = [&](int bid) -> const std::vector<int> & {
        static const std::vector<int> empty;
        auto *bp = cfg.FindBlock(bid);
        return bp ? bp->succ_ids : empty;
    };
    std::function<void(int)> dfs = [&](int bid) {
        if (visited.count(bid)) return;
        visited.insert(bid);
        for (int sid: block_succs(bid)) dfs(sid);
        rpo_order.push_back(bid);
    };
    dfs(cfg.entry_id);
    std::reverse(rpo_order.begin(), rpo_order.end());

    // 2) in/out envs
    std::unordered_map<int, VarEnv> in_envs, out_envs;

    // 3) 初始 env：seed params in entry block
    in_envs[cfg.entry_id] = {};
    for (const auto &[pname, pidx]: cfg.param_indices) {
        int ver = (pidx < (int) ssa.param_versions.size()) ? ssa.param_versions[pidx] : -1;
        SSATypeInfo pty = {T_DYNAMIC, -1};
        if (ver >= 0) {
            auto vit = version_types.find(ver);
            if (vit != version_types.end()) {
                pty = vit->second;
            }
        }
        in_envs[cfg.entry_id][pname] = pty;
    }

    // 4) 迭代直到不动点
    std::deque<int> worklist(rpo_order.begin(), rpo_order.end());
    int iter_count = 0;
    constexpr int kMaxIters = 64;
    constexpr int kWidenIter = 3;
    bool widened = false;

    std::unordered_map<int, int> block_exec_count;

    while (!worklist.empty() && iter_count < kMaxIters) {
        int bid = worklist.front();
        worklist.pop_front();
        iter_count++;
        block_exec_count[bid]++;

        VarEnv new_in;
        bool first_pred = true;
        for (int pid: cfg.FindBlock(bid)->pred_ids) {
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
            new_in = {};// unreachable block (shouldn't happen if dfs found all)
        } else if (bid == cfg.entry_id) {
            new_in = in_envs[cfg.entry_id];
        }

        VarEnv old_out = out_envs[bid];
        VarEnv new_out = new_in;
        const TypeEnv empty_vt;
        for (const auto &s: cfg.FindBlock(bid)->stmts) {
            new_out = TransferStmt(s, new_out, ssa, empty_vt);
        }

        // widening on loops: 当某个基本块被重复执行超过阈值时，对该块的 env 进行 Widen
        if (block_exec_count[bid] > kWidenIter && registry_) {
            for (auto &[vname, vtype]: new_out) {
                if (IsRecordInferredType(vtype.type) && vtype.shape_id >= 0) {
                    int new_shape_id = registry_->Widen(vtype.shape_id, block_exec_count[bid]);
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
            for (int sid: block_succs(bid)) {
                if (std::find(worklist.begin(), worklist.end(), sid) == worklist.end()) worklist.push_back(sid);
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
    for (const auto &[v, t]: b) {
        auto it = r.find(v);
        if (it == r.end()) r[v] = t;
        else
            it->second = UnifiedTypeAnalyzer::Meet(it->second, t);
    }
    return r;
}

// ─────────────────────────────────────────────────────────────────────────
// Transfer function — 单语句 env 转移
// -------------------------------------------------------------------
// 目前只处理能简单推导出变量类型的赋值和 local 声明；
// 控制流语句（If/While/For）在外部 CFG 已经分裂，不在此处递归。
// ─────────────────────────────────────────────────────────────────────────
UnifiedTypeAnalyzer::VarEnv UnifiedTypeAnalyzer::TransferStmt(const SyntaxTreeInterfacePtr &stmt, const VarEnv &env, const SSAFunction &ssa, const TypeEnv &version_types) {

    if (!stmt) return env;
    VarEnv out = env;
    const VarEnv *envp = &out;
    // InferExprType 此时不需要 ir，但我们保留调用兼容（传 dummy）
    InferResult dummy_ir;

    switch (stmt->Type()) {
        case SyntaxTreeType::LocalVar: {
            auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
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
            auto as = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);
            auto vl = as->Varlist() ? std::dynamic_pointer_cast<SyntaxTreeVarlist>(as->Varlist()) : nullptr;
            auto el = as->Explist() ? std::dynamic_pointer_cast<SyntaxTreeExplist>(as->Explist()) : nullptr;
            if (!vl || !el) return out;
            auto &vars = vl->Vars();
            auto &exps = el->Exps();
            for (size_t i = 0; i < vars.size() && i < exps.size(); ++i) {
                if (vars[i]->Type() != SyntaxTreeType::Var) continue;
                auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(vars[i]);
                if (!v) continue;

                // 递归自赋值检测：node.next = node 应让 node.next 退化为 T_DYNAMIC。
                // 条件：LHS 是 a.field，RHS 是引用同一个简单变量 a 的表达式。
                bool recursive_self_assign = false;
                if (v->GetVarKind() == VarKind::kDot || v->GetVarKind() == VarKind::kSquare) {
                    auto rhs_pe = exps[i]->Type() == SyntaxTreeType::PrefixExp ? std::dynamic_pointer_cast<SyntaxTreePrefixexp>(exps[i]) : nullptr;
                    if (!rhs_pe && exps[i]->Type() == SyntaxTreeType::Exp) {
                        auto rhs_ep = std::dynamic_pointer_cast<SyntaxTreeExp>(exps[i]);
                        if (rhs_ep->GetExpKind() == ExpKind::kPrefixExp) rhs_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(rhs_ep->Right());
                    }
                    if (rhs_pe && rhs_pe->GetPrefixKind() == PrefixExpKind::kVar) {
                        auto rhs_var = std::dynamic_pointer_cast<SyntaxTreeVar>(rhs_pe->GetValue());
                        if (rhs_var && rhs_var->GetVarKind() == VarKind::kSimple) {
                            const std::string &rhs_name = rhs_var->GetName();
                            auto base_pe2 = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(v->GetPrefixexp());
                            if (base_pe2 && base_pe2->GetPrefixKind() == PrefixExpKind::kVar) {
                                auto base_var2 = std::dynamic_pointer_cast<SyntaxTreeVar>(base_pe2->GetValue());
                                if (base_var2 && base_var2->GetVarKind() == VarKind::kSimple && base_var2->GetName() == rhs_name) {
                                    recursive_self_assign = true;
                                }
                            }
                        }
                    }
                }

                auto ty = InferExprType(exps[i], ssa, version_types, dummy_ir, {}, envp);
                if (v->GetVarKind() == VarKind::kSimple) {
                    // 简单变量赋值：local x = ... 或 x = ...
                    out[v->GetName()] = ty;
                } else if (v->GetVarKind() == VarKind::kDot || v->GetVarKind() == VarKind::kSquare) {
                    // 字段写入：a.b = v 或 a[b] = v
                    // 规范 §5.2.4：更新 base 变量的 shape
                    const std::string &field_name = v->GetName();
                    auto base_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(v->GetPrefixexp());
                    if (base_pe && base_pe->GetPrefixKind() == PrefixExpKind::kVar) {
                        auto base_var = std::dynamic_pointer_cast<SyntaxTreeVar>(base_pe->GetValue());
                        if (base_var && base_var->GetVarKind() == VarKind::kSimple) {
                            const std::string &base_name = base_var->GetName();
                            auto base_it = out.find(base_name);
                            if (base_it != out.end() && base_it->second.shape_id >= 0 && registry_) {
                                // 规范 §5.2.4：字段写入 a.b = v
                                ShapeType shape = registry_->Get(base_it->second.shape_id);
                                FieldDef *fd = shape.FindFieldMut(field_name);
                                if (recursive_self_assign) {
                                    // 自赋值引起的递归类型检测：node.next = node
                                    // 对该字段退化为 T_DYNAMIC（走 hash，避免结构递归）。
                                    if (fd) {
                                        fd->type = T_DYNAMIC;
                                    } else {
                                        if (!shape.is_open) shape.is_open = true;
                                        FieldDef new_fd;
                                        new_fd.name = field_name;
                                        new_fd.c_field_name = ToSafeCFieldName(field_name);
                                        new_fd.type = T_DYNAMIC;
                                        new_fd.optional = false;
                                        shape.fields.push_back(new_fd);
                                    }
                                } else if (fd) {
                                    // 已有字段：合一类型
                                    fd->type = ShapeRegistry::MeetType(fd->type, ty.type);
                                } else {
                                    // 新字段
                                    if (!shape.is_open) {
                                        // 封闭 record 加新字段 → 退化为开放
                                        shape.is_open = true;
                                    }
                                    FieldDef reg_fd;
                                    reg_fd.name = field_name;
                                    reg_fd.c_field_name = ToSafeCFieldName(field_name);
                                    reg_fd.type = ty.type;
                                    reg_fd.optional = false;
                                    shape.fields.push_back(reg_fd);
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
            auto fl = std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt);
            // 数值 for 游标类型推断：如果 env 中 ExpEnd/ExpStep 对应变量为 T_FLOAT，
            // 或字面量含小数点，则游标 T_FLOAT；否则 T_INT。
            InferredType cursor_type = T_INT;
            std::function<void(const SyntaxTreeInterfacePtr &)> check_exp;
            check_exp = [&](const SyntaxTreeInterfacePtr &e) {
                if (!e) return;
                if (e->Type() == SyntaxTreeType::Exp) {
                    auto ep = std::dynamic_pointer_cast<SyntaxTreeExp>(e);
                    if (ep->GetExpKind() == ExpKind::kNumber) {
                        const auto &v = ep->ExpValue();
                        if (v.find('.') != std::string::npos || v.find('e') != std::string::npos || v.find('E') != std::string::npos) cursor_type = T_FLOAT;
                    } else if (ep->GetExpKind() == ExpKind::kPrefixExp && ep->Right()) {
                        check_exp(ep->Right());
                    }
                } else if (e->Type() == SyntaxTreeType::Var) {
                    auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(e);
                    if (v->GetVarKind() == VarKind::kSimple) {
                        auto it = out.find(v->GetName());
                        if (it != out.end() && it->second.type == T_FLOAT) cursor_type = T_FLOAT;
                    }
                } else if (e->Type() == SyntaxTreeType::PrefixExp) {
                    auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e);
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
            auto fi = std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt);
            if (fi->Namelist() && fi->Namelist()->Type() == SyntaxTreeType::NameList) {
                auto nlist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(fi->Namelist());
                if (nlist) {
                    for (const auto &n: nlist->Names()) out[n] = {T_DYNAMIC, -1};
                }
            }
            break;
        }
        default:
            break;
    }
    return out;
}

// ── 常量传播辅助 ────────────────────────────────────────────────────────
// 从表达式提取字面量值（string / number）。
bool UnifiedTypeAnalyzer::ExtractLiteral(const SyntaxTreeInterfacePtr &exp, std::string &value_out) {
    if (!exp) return false;
    // 直接 String 节点
    if (exp->Type() == SyntaxTreeType::Exp) {
        auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
        auto k = e->GetExpKind();
        if (k == ExpKind::kString) {
            value_out = e->ExpValue();
            return true;
        }
        if (k == ExpKind::kNumber) {
            value_out = e->ExpValue();
            return true;
        }
        // 解包 PrefixExp(Var) 等包装
        if (k == ExpKind::kPrefixExp && e->Right()) return ExtractLiteral(e->Right(), value_out);
        return false;
    }
    if (exp->Type() == SyntaxTreeType::PrefixExp) {
        auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(exp);
        if (pe->GetValue()) return ExtractLiteral(pe->GetValue(), value_out);
        return false;
    }
    if (exp->Type() == SyntaxTreeType::Var) {
        auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(exp);
        if (v->GetVarKind() == VarKind::kSimple && v->GetName().rfind("__fakelua_vararg_", 0) == 0) return false;
        return false;// 变量引用不在此处理（由 ResolveKeyConstant 查 ConstEnv）
    }
    return false;
}

// 用 ConstEnv 将 key 表达式解析为常量字符串。
bool UnifiedTypeAnalyzer::ResolveKeyConstant(const SyntaxTreeInterfacePtr &key_expr, const ConstEnv &const_env, std::string &key_out) {
    if (!key_expr) return false;
    // 直接字面量
    if (ExtractLiteral(key_expr, key_out)) return true;
    // 变量名：查 ConstEnv
    if (key_expr->Type() == SyntaxTreeType::Exp) {
        auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(key_expr);
        if (e->GetExpKind() == ExpKind::kPrefixExp && e->Right()) return ResolveKeyConstant(e->Right(), const_env, key_out);
        return false;
    }
    if (key_expr->Type() == SyntaxTreeType::PrefixExp) {
        auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(key_expr);
        if (pe->GetValue()) return ResolveKeyConstant(pe->GetValue(), const_env, key_out);
        return false;
    }
    if (key_expr->Type() == SyntaxTreeType::Var) {
        auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(key_expr);
        if (v->GetVarKind() == VarKind::kSimple) {
            auto it = const_env.find(v->GetName());
            if (it != const_env.end()) {
                key_out = it->second;
                return true;
            }
        }
        return false;
    }
    return false;
}

// 带常量传播的 env 转移
UnifiedTypeAnalyzer::VarEnv UnifiedTypeAnalyzer::TransferStmtConst(const SyntaxTreeInterfacePtr &stmt, const VarEnv &env, ConstEnv &const_env, const SSAFunction &ssa, const TypeEnv &version_types) {

    if (!stmt) return env;
    VarEnv out = env;
    ConstEnv &const_out = const_env;// 同引用，直接修改

    switch (stmt->Type()) {
        case SyntaxTreeType::LocalVar: {
            auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
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
                    const_out.erase(names[i]);
                    continue;
                }
                auto &exp = exps->Exps()[i];
                auto ty = InferExprType(exp, ssa, version_types, InferResult{}, {}, &env, &const_env);
                out[names[i]] = ty;
                // 常量传播：记录字面量常量
                std::string lit_val;
                if (ExtractLiteral(exp, lit_val)) {
                    const_out[names[i]] = lit_val;
                } else {
                    const_out.erase(names[i]);// 不再确定是常量
                }
            }
            break;
        }
        case SyntaxTreeType::Assign: {
            auto as = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);
            auto vl = as->Varlist() ? std::dynamic_pointer_cast<SyntaxTreeVarlist>(as->Varlist()) : nullptr;
            auto el = as->Explist() ? std::dynamic_pointer_cast<SyntaxTreeExplist>(as->Explist()) : nullptr;
            if (!vl || !el) return out;
            auto &vars = vl->Vars();
            auto &exps = el->Exps();
            for (size_t i = 0; i < vars.size() && i < exps.size(); ++i) {
                if (vars[i]->Type() != SyntaxTreeType::Var) continue;
                auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(vars[i]);
                if (!v) continue;
                auto ty = InferExprType(exps[i], ssa, version_types, InferResult{}, {}, &env);
                if (v->GetVarKind() == VarKind::kSimple) {
                    const std::string &vn = v->GetName();
                    out[vn] = ty;
                    std::string lit_val;
                    if (ExtractLiteral(exps[i], lit_val)) {
                        const_out[vn] = lit_val;
                    } else {
                        const_out.erase(vn);// 不再确定是常量（变量来源）
                    }
                } else {
                    // kDot/kSquare 字段写入（已有处理）
                    break;
                }
            }
            break;
        }
        default:
            break;
    }
    return out;
}

static void RecordSsaVersionType(const SyntaxTreeInterfacePtr &node, const SSAFunction &ssa, const SSATypeInfo &ty, InferResult &ir) {
    if (node && node->Type() == SyntaxTreeType::Var) {
        auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(node);
        if (v->GetVarKind() == VarKind::kSimple) {
            auto dit = ssa.def_versions.find(node.get());
            if (dit != ssa.def_versions.end()) {
                ir.ssa_version_types[dit->second] = ty;
            }
            auto uit = ssa.use_versions.find(node.get());
            if (uit != ssa.use_versions.end()) {
                for (int ver: uit->second) {
                    ir.ssa_version_types[ver] = ty;
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────
// PopulateNodeTypesFromBlock：用块的 env 上下文，填充该块内所有 AST 子节点的 main_ssa_types
// 与 Analyze 中的 WalkSyntaxTree 同理，但用 per-block env 而非全局 env。
// ─────────────────────────────────────────────────────────────────────────
void UnifiedTypeAnalyzer::PopulateNodeTypesFromStmts(const std::vector<SyntaxTreeInterfacePtr> &stmts, const VarEnv &env, const ConstEnv &const_env, const SSAFunction &ssa,
                                                     const TypeEnv &version_types, InferResult &ir, std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> *out_map) {

    if (!out_map) out_map = &ir.main_ssa_types;
    for (const auto &stmt: stmts) {
        if (!stmt) continue;
        WalkSyntaxTreePruned(stmt, [&](const SyntaxTreeInterfacePtr &node) -> bool {
            if (!node) return false;
            if (node->Type() == SyntaxTreeType::Function || node->Type() == SyntaxTreeType::LocalFunction) {
                return false;
            }
            switch (node->Type()) {
                case SyntaxTreeType::Exp:
                case SyntaxTreeType::Var:
                case SyntaxTreeType::PrefixExp:
                case SyntaxTreeType::TableConstructor:
                case SyntaxTreeType::FunctionCall:
                case SyntaxTreeType::ExpList: {
                    auto ty = InferExprType(node, ssa, version_types, ir, {}, &env, &const_env);
                    (*out_map)[node.get()] = ty;
                    if (out_map == &ir.main_ssa_types) {
                        RecordSsaVersionType(node, ssa, ty, ir);
                    }
                    break;
                }
                case SyntaxTreeType::ForLoop: {
                    // 推导 for-loop 游标节点本身类型（CGen 用来决定 i 的声明类型）
                    auto fl = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
                    InferredType cursor_type = T_INT;
                    std::function<void(const SyntaxTreeInterfacePtr &)> check_exp;
                    check_exp = [&](const SyntaxTreeInterfacePtr &e) {
                        if (!e) return;
                        if (e->Type() == SyntaxTreeType::Exp) {
                            auto ep = std::dynamic_pointer_cast<SyntaxTreeExp>(e);
                            if (ep->GetExpKind() == ExpKind::kNumber) {
                                const auto &v = ep->ExpValue();
                                if (v.find('.') != std::string::npos || v.find('e') != std::string::npos || v.find('E') != std::string::npos) cursor_type = T_FLOAT;
                            } else if (ep->GetExpKind() == ExpKind::kPrefixExp && ep->Right()) {
                                check_exp(ep->Right());
                            }
                        } else if (e->Type() == SyntaxTreeType::Var) {
                            auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(e);
                            if (v->GetVarKind() == VarKind::kSimple) {
                                auto it = env.find(v->GetName());
                                if (it != env.end() && it->second.type == T_FLOAT) cursor_type = T_FLOAT;
                            }
                        } else if (e->Type() == SyntaxTreeType::PrefixExp) {
                            auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e);
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
            return true;
        });
    }
}

// ─────────────────────────────────────────────────────────────────────────
// 变量名 → SSA 版本映射
// ─────────────────────────────────────────────────────────────────────────
UnifiedTypeAnalyzer::VarNameToVersion UnifiedTypeAnalyzer::BuildVarNameVersionMap(const SSAFunction &ssa) {
    VarNameToVersion m;
    for (auto &[name, vers]: ssa.var_all_versions) {
        if (!vers.empty()) m[name] = vers.back();
    }
    return m;
}

// ─────────────────────────────────────────────────────────────────────────
// 递归推导表达式类型
// ─────────────────────────────────────────────────────────────────────────
SSATypeInfo UnifiedTypeAnalyzer::InferExprType(const SyntaxTreeInterfacePtr &expr, const SSAFunction &ssa, const TypeEnv &version_types, const InferResult &ir,
                                               const std::unordered_map<std::string, int> &name_ver, const VarEnv *local_env, const ConstEnv *const_env) {

    if (!expr) return {T_UNKNOWN, -1};

    switch (expr->Type()) {
        case SyntaxTreeType::Exp: {
            auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(expr);
            switch (e->GetExpKind()) {
                case ExpKind::kNumber: {
                    const auto &v = e->ExpValue();
                    if (v.find('.') == std::string::npos && v.find('e') == std::string::npos && v.find('E') == std::string::npos) return {T_INT, -1};
                    return {T_FLOAT, -1};
                }
                case ExpKind::kString:
                    return {T_STRING, -1};
                case ExpKind::kNil:
                    return {T_NIL, -1};
                case ExpKind::kTrue:
                case ExpKind::kFalse:
                    return {T_BOOL, -1};
                case ExpKind::kBinop: {
                    const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(e->Op());
                    DEBUG_ASSERT(op);
                    const auto k = op->GetOpKind();
                    if (k == BinOpKind::kLess || k == BinOpKind::kLessEqual ||
                        k == BinOpKind::kMore || k == BinOpKind::kMoreEqual ||
                        k == BinOpKind::kEqual || k == BinOpKind::kNotEqual) {
                        return {T_BOOL, -1};
                    }
                    if (k == BinOpKind::kBitAnd || k == BinOpKind::kBitOr ||
                        k == BinOpKind::kXor || k == BinOpKind::kLeftShift ||
                        k == BinOpKind::kRightShift) {
                        return {T_INT, -1};
                    }
                    if (k == BinOpKind::kSlash || k == BinOpKind::kPow) {
                        return {T_FLOAT, -1};
                    }
                    auto lt = InferExprType(e->Left(), ssa, version_types, ir, name_ver, local_env, const_env);
                    auto rt = InferExprType(e->Right(), ssa, version_types, ir, name_ver, local_env, const_env);
                    return {ShapeRegistry::MeetType(lt.type, rt.type), -1};
                }
                case ExpKind::kUnop: {
                    const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(e->Op());
                    DEBUG_ASSERT(op);
                    const auto op_kind = op->GetOpKind();
                    if (op_kind == UnOpKind::kNot) {
                        return {T_BOOL, -1};
                    }
                    if (op_kind == UnOpKind::kBitNot || op_kind == UnOpKind::kNumberSign) {
                        return {T_INT, -1};
                    }
                    return InferExprType(e->Right(), ssa, version_types, ir, name_ver, local_env, const_env);
                }
                case ExpKind::kPrefixExp: {
                    return InferExprType(e->Right(), ssa, version_types, ir, name_ver, local_env, const_env);
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
            auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(expr);
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
                // kDot: field_name 直接就是字段名（字符串）
                // kSquare: field_name 初始为空，需要从 key 表达式解析
                const std::string &dot_field_name = v->GetName();
                auto base_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(v->GetPrefixexp());
                if (base_pe && base_pe->GetPrefixKind() == PrefixExpKind::kVar) {
                    auto base_var = std::dynamic_pointer_cast<SyntaxTreeVar>(base_pe->GetValue());
                    if (base_var && base_var->GetVarKind() == VarKind::kSimple) {
                        // 查找 base 变量的类型
                        SSATypeInfo base_type{T_DYNAMIC, -1};
                        auto env_it = local_env->find(base_var->GetName());
                        if (env_it != local_env->end()) {
                            base_type = env_it->second;
                        }
                        if (base_type.shape_id >= 0 && registry_) {
                            const ShapeType &shape = registry_->Get(base_type.shape_id);
                            // kDot：直接字段名查找
                            if (v->GetVarKind() == VarKind::kDot) {
                                const FieldDef *fd = shape.FindField(dot_field_name);
                                if (fd) return {fd->type, -1};            // ★ 命中 → 走偏移
                                if (shape.is_open) return {T_DYNAMIC, -1};// 开放 → hash
                                return {T_NIL, -1};                       // 封闭无此字段 → nil
                            }
                            // kSquare：先尝试直接字段名，再尝试常量传播
                            const FieldDef *fd = shape.FindField(dot_field_name);
                            if (fd) return {fd->type, -1};
                            // §12.6 常量传播：a[key] 中 key 是变量但编译期可知其值
                            if (const_env) {
                                std::string key_val;
                                if (ResolveKeyConstant(v->GetExp(), *const_env, key_val)) {
                                    const FieldDef *cfd = shape.FindField(key_val);
                                    if (cfd) return {cfd->type, -1};// 常量 key 命中 → 走偏移
                                    if (shape.is_open) return {T_DYNAMIC, -1};
                                    return {T_NIL, -1};
                                }
                            }
                            // 无法确定 key → 退化
                            if (shape.is_open) {
                                return {T_DYNAMIC, -1};
                            }
                            InferredType merged_field_type = T_UNKNOWN;
                            if (!shape.fields.empty()) {
                                merged_field_type = shape.fields[0].type;
                                for (size_t idx = 1; idx < shape.fields.size(); ++idx) {
                                    merged_field_type = ShapeRegistry::MeetType(merged_field_type, shape.fields[idx].type);
                                }
                            }
                            if (merged_field_type == T_UNKNOWN || merged_field_type == T_NIL) {
                                return {T_DYNAMIC, -1};
                            }
                            return {merged_field_type, -1};
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
            auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(expr);
            return InferExprType(pe->GetValue(), ssa, version_types, ir, name_ver, local_env, const_env);
        }
        case SyntaxTreeType::FunctionCall: {
            // 过程间分析（规范 §7）：从函数摘要获取返回类型
            auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(expr);
            std::string callee;
            if (fc->prefixexp() && fc->prefixexp()->Type() == SyntaxTreeType::Var) {
                callee = std::dynamic_pointer_cast<SyntaxTreeVar>(fc->prefixexp())->GetName();
            } else if (fc->prefixexp() && fc->prefixexp()->Type() == SyntaxTreeType::PrefixExp) {
                auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
                if (pe->GetValue() && pe->GetValue()->Type() == SyntaxTreeType::Var) callee = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue())->GetName();
            }
            if (!callee.empty()) {
                auto math_it = ir.math_param_positions.find(callee);
                auto spec_it = ir.specialization_return_types.find(callee);
                if (math_it != ir.math_param_positions.end() && spec_it != ir.specialization_return_types.end()) {
                    const auto &math_params = math_it->second;
                    const auto &spec_returns = spec_it->second;
                    const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
                    if (args_ptr) {
                        const auto raw_args = ExtractCallRawArgs(args_ptr);
                        int bitmask = 0;
                        bool args_valid = true;
                        for (size_t k = 0; k < math_params.size(); ++k) {
                            int param_pos = math_params[k];
                            if (param_pos < (int)raw_args.size()) {
                                SSATypeInfo arg_info = InferExprType(raw_args[param_pos], ssa, version_types, ir, name_ver, local_env, const_env);
                                if (arg_info.type == T_FLOAT) {
                                    bitmask |= (1 << k);
                                } else if (arg_info.type != T_INT) {
                                    args_valid = false;
                                    break;
                                }
                            } else {
                                args_valid = false;
                                break;
                            }
                        }
                        if (args_valid && bitmask < (int)spec_returns.size()) {
                            return {spec_returns[bitmask], -1};
                        }
                    }
                }

                auto it = ir.func_summaries.find(callee);
                if (it != ir.func_summaries.end()) {
                    const FuncSummary &summary = it->second;
                    if (summary.being_built) {
                        // 递归调用：返回类型未知
                        return {T_DYNAMIC, -1};
                    }

                    // ── HM 多态实例化（规范 §12 多态）────────────────────────
                    // 若调用点已知的 HM 签名可用，使用 HM 统一推导更宽的返回类型。
                    // 条件：summary 带 must_use_hm 且数量与实参数量一致。该路径
                    // 也处理 make(x) 类型的跨函数类型推导，例如 `local p = make(1)`
                    // 期望得到 p : Record{val:int}。
                    if (summary.must_use_hm && !summary.param_hm_types.empty()) {
                        // 推导各实参类型。
                        auto raw_args = ExtractCallRawArgs(std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args()));
                        if (!raw_args.empty() && (int) raw_args.size() == (int) summary.param_hm_types.size()) {
                            // 为每个实参生成 HM 类型（来自当前推导结果）。
                            bool ok = true;
                            std::vector<Type *> arg_hm;
                            arg_hm.reserve(raw_args.size());
                            for (size_t k = 0; k < raw_args.size(); ++k) {
                                SSATypeInfo arg_info = InferExprType(raw_args[k], ssa, version_types, ir, name_ver, local_env);
                                arg_hm.push_back(SsaInfoToHm(arg_info));
                            }
                            // 创建一个独立的子作用域实例化：克隆 ret_hm_type 并替换形参。
                            Type *inst_ret = InstantiateHmSignature(summary.param_hm_types, summary.ret_hm_type, arg_hm, ok);
                            if (ok && inst_ret) {
                                SSATypeInfo specialized = HmToSsaInfo(inst_ret);
                                // 只有在被推导出更具体类型时采用（避免覆盖 static 已知返回）。
                                if (specialized.type != T_DYNAMIC && specialized.type != T_UNKNOWN) {
                                    return specialized;
                                }
                            }
                            // 回退：原始多态返回类型。
                            SSATypeInfo poly_ret = HmToSsaInfo(summary.ret_hm_type);
                            if (poly_ret.type != T_UNKNOWN) return poly_ret;
                        }
                    }

                    if (summary.ret_type.type != T_UNKNOWN && summary.ret_type.type != T_DYNAMIC) {
                        return summary.ret_type;
                    }
                    // 返回类型是 DYNAMIC 但摘要已知：尝试从表达式推导
                    // （对于 return param 类型的函数，ret_type 保持 DYNAMIC）
                }
            }
            return {T_DYNAMIC, -1};
        }
        default:
            return {T_DYNAMIC, -1};
    }
}

int UnifiedTypeAnalyzer::BuildShapeFromCtor(const SyntaxTreeInterfacePtr &tc, const SSAFunction &ssa, const TypeEnv &version_types, const InferResult &ir) {
    if (!tc || tc->Type() != SyntaxTreeType::TableConstructor) return -1;
    auto tc_ptr = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(tc);
    if (!tc_ptr->Fieldlist()) return -1;
    auto fl = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(tc_ptr->Fieldlist());

    ShapeType shape;
    shape.is_open = false;

    for (auto &field_node: fl->Fields()) {
        if (!field_node || field_node->Type() != SyntaxTreeType::Field) continue;
        auto fp = std::dynamic_pointer_cast<SyntaxTreeField>(field_node);

        FieldDef fd;
        if (fp->GetFieldKind() == FieldKind::kObject) {
            fd.name = fp->Name();
            fd.c_field_name = ToSafeCFieldName(fp->Name());
        } else {
            if (fp->GetFieldKind() == FieldKind::kArray && fp->Key() == nullptr) {
                fd.name = std::to_string(shape.fields.size() + 1);
                fd.is_int_key = true;
                fd.c_field_name = ToSafeCFieldName("_int_" + fd.name);
            } else {
                auto ke = fp->Key() && fp->Key()->Type() == SyntaxTreeType::Exp ? std::dynamic_pointer_cast<SyntaxTreeExp>(fp->Key()) : nullptr;
                if (ke && ke->GetExpKind() == ExpKind::kNumber) {
                    fd.name = ke->ExpValue();
                    fd.is_int_key = (fd.name.find('.') == std::string::npos);
                    fd.c_field_name = ToSafeCFieldName(fd.is_int_key ? "_int_" + fd.name : "_float_" + fd.name);
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

    for (auto &[var_name, versions]: ssa.var_all_versions) {
        if (versions.empty()) continue;
        SSATypeInfo merged{T_UNKNOWN, -1};
        for (int ver: versions) {
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

void UnifiedTypeAnalyzer::ComputeCtorTargetShapes(const SyntaxTreeInterfacePtr &func_block, const SSAFunction &ssa, InferResult &ir) {
    if (!func_block) return;
    auto &ctor_map = ir.ctor_target_shapes;
    ctor_map.clear();

    LinkExprToTargetShape(func_block, "", ir.var_final_shapes, ctor_map);
}

void UnifiedTypeAnalyzer::LinkExprToTargetShape(const SyntaxTreeInterfacePtr &node, const std::string &target_name, const std::unordered_map<std::string, int> &var_final_shapes,
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
            for (auto &s: blk->Stmts()) {
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
                    auto fb =
                            (s->Type() == SyntaxTreeType::Function) ? std::dynamic_pointer_cast<SyntaxTreeFunction>(s)->Funcbody() : std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(s)->Funcbody();
                    auto fb_ptr = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(fb);
                    if (fb_ptr && fb_ptr->Block()) LinkExprToTargetShape(fb_ptr->Block(), target_name, var_final_shapes, ctor_target_shapes);
                }
            }
            break;
        }
        case SyntaxTreeType::Exp: {
            auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
            LinkExprToTargetShape(e->Right(), target_name, var_final_shapes, ctor_target_shapes);
            break;
        }
        case SyntaxTreeType::ExpList: {
            auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
            for (auto &exp: el->Exps()) LinkExprToTargetShape(exp, target_name, var_final_shapes, ctor_target_shapes);
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
static InferredType DeriveExprTypeForRet(const SyntaxTreeInterfacePtr &expr, const std::vector<SSATypeInfo> &param_types,
                                         const std::unordered_map<std::string, int> &param_indices, const InferResult &ir) {
    if (!expr) return T_UNKNOWN;
    auto inner = expr;
    // 只解包 Exp(kPrefixExp) 和 PrefixExp 包装，不碰 Binop/Number
    while (inner && inner->Type() == SyntaxTreeType::PrefixExp) {
        auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(inner);
        inner = pe->GetValue();
    }
    while (inner && inner->Type() == SyntaxTreeType::Exp && std::dynamic_pointer_cast<SyntaxTreeExp>(inner)->GetExpKind() == ExpKind::kPrefixExp) {
        auto ep = std::dynamic_pointer_cast<SyntaxTreeExp>(inner);
        inner = ep->Right();
    }
    // 再次解包 PrefixExp（Exp(kPrefixExp) 的 Right 可能是 PrefixExp）
    while (inner && inner->Type() == SyntaxTreeType::PrefixExp) {
        auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(inner);
        inner = pe->GetValue();
    }
    if (!inner) return T_UNKNOWN;
    if (inner->Type() == SyntaxTreeType::FunctionCall) {
        // 函数调用：查找 func_summaries 获取返回类型
        auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(inner);
        std::string callee;
        if (fc->prefixexp() && fc->prefixexp()->Type() == SyntaxTreeType::Var) {
            callee = std::dynamic_pointer_cast<SyntaxTreeVar>(fc->prefixexp())->GetName();
        } else if (fc->prefixexp() && fc->prefixexp()->Type() == SyntaxTreeType::PrefixExp) {
            auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
            if (pe->GetValue() && pe->GetValue()->Type() == SyntaxTreeType::Var) callee = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue())->GetName();
        }
        if (!callee.empty()) {
            auto it = ir.func_summaries.find(callee);
            if (it != ir.func_summaries.end() && it->second.ret_type.type != T_UNKNOWN && it->second.ret_type.type != T_DYNAMIC) return it->second.ret_type.type;
        }
        return T_UNKNOWN;
    }
    if (inner->Type() == SyntaxTreeType::Var) {
        auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(inner);
        if (v->GetVarKind() == VarKind::kSimple) {
            const auto &vname = v->GetName();
            if (const auto pit = param_indices.find(vname); pit != param_indices.end()) {
                int pidx = pit->second;
                if (pidx >= 0 && pidx < static_cast<int>(param_types.size())) {
                    return param_types[pidx].type;
                }
            }
            return T_UNKNOWN;
        }
        return T_UNKNOWN;
    }
    if (inner->Type() == SyntaxTreeType::Exp) {
        auto ep = std::dynamic_pointer_cast<SyntaxTreeExp>(inner);
        if (ep->GetExpKind() == ExpKind::kNumber) {
            const auto &val = ep->ExpValue();
            return (val.find('.') != std::string::npos || val.find('e') != std::string::npos || val.find('E') != std::string::npos) ? T_FLOAT : T_INT;
        }
        if (ep->GetExpKind() == ExpKind::kBinop) {
            auto bin = ep->Op() ? std::dynamic_pointer_cast<SyntaxTreeBinop>(ep->Op()) : nullptr;
            if (!bin) return T_UNKNOWN;
            auto kind = bin->GetOpKind();
            bool is_arith =
                    (kind == BinOpKind::kPlus || kind == BinOpKind::kMinus || kind == BinOpKind::kStar || kind == BinOpKind::kSlash || kind == BinOpKind::kDoubleSlash || kind == BinOpKind::kMod ||
                     kind == BinOpKind::kPow || kind == BinOpKind::kBitAnd || kind == BinOpKind::kBitOr || kind == BinOpKind::kXor || kind == BinOpKind::kLeftShift || kind == BinOpKind::kRightShift);
            InferredType lt = DeriveExprTypeForRet(ep->Left(), param_types, param_indices, ir);
            InferredType rt = DeriveExprTypeForRet(ep->Right(), param_types, param_indices, ir);
            if (is_arith && IsNumericInferredType(lt) && IsNumericInferredType(rt)) return (lt == T_FLOAT || rt == T_FLOAT) ? T_FLOAT : T_INT;
            if ((kind == BinOpKind::kAnd || kind == BinOpKind::kOr) && IsNumericInferredType(lt) && IsNumericInferredType(rt)) return (lt == T_FLOAT || rt == T_FLOAT) ? T_FLOAT : T_INT;
            return T_UNKNOWN;
        }
        if (ep->GetExpKind() == ExpKind::kUnop) {
            auto unop = ep->Op() ? std::dynamic_pointer_cast<SyntaxTreeUnop>(ep->Op()) : nullptr;
            if (unop && (unop->GetOpKind() == UnOpKind::kMinus || unop->GetOpKind() == UnOpKind::kBitNot)) return DeriveExprTypeForRet(ep->Right(), param_types, param_indices, ir);
            return T_UNKNOWN;
        }
    }
    return T_UNKNOWN;
}

void UnifiedTypeAnalyzer::BuildSummary(const std::string &func_name, const SyntaxTreeInterfacePtr &func_block, const SSAFunction &ssa, const CFGFunction &cfg, const TypeEnv &version_types,
                                       InferResult &ir) {
    FuncSummary &s = ir.func_summaries[func_name];
    s.func_name = func_name;
    s.param_types.resize(ssa.param_versions.size(), {T_DYNAMIC, -1});
    s.param_escape.resize(ssa.param_versions.size(), false);

    // §8.2 逃逸信息收集
    auto esc_it = ir.escape_vars.find(func_name);
    if (esc_it != ir.escape_vars.end()) {
        const EscapeEnv &esc = esc_it->second;
        // 检查每个参数是否逃逸
        for (size_t i = 0; i < ssa.param_versions.size(); ++i) {
            // 通过参数名查找逃逸状态
            // 注意：param_indices 给出了参数名到索引的映射
            for (auto &[pname, pidx]: cfg.param_indices) {
                if (pidx == (int) i) {
                    auto pit = esc.find(pname);
                    if (pit != esc.end()) s.param_escape[i] = pit->second;
                    break;
                }
            }
        }
    }

    // 从 version_types 推导参数类型（规范 §7.2）
    for (size_t i = 0; i < ssa.param_versions.size(); ++i) {
        int ver = ssa.param_versions[i];
        // 从 SSA 定义块中查找该参数被使用时的类型
        auto vt_it = version_types.find(ver);
        if (vt_it != version_types.end()) {
            s.param_types[i] = vt_it->second;
        } else {
            // 默认 T_DYNAMIC（参数类型未知）
            s.param_types[i] = {T_DYNAMIC, -1};
        }
    }

    // 从 main_ssa_types 收集返回类型
    SSATypeInfo merged_ret{T_UNKNOWN, -1};
    if (func_block) {
        WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
            if (!node || node->Type() != SyntaxTreeType::Return) return;
            auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(node);
            if (!ret->Explist()) return;
            auto el_ptr = ret->Explist()->Type() == SyntaxTreeType::ExpList ? std::dynamic_pointer_cast<SyntaxTreeExplist>(ret->Explist()) : std::shared_ptr<SyntaxTreeExplist>();
            if (!el_ptr || el_ptr->Exps().empty()) return;
            auto it = ir.main_ssa_types.find(el_ptr->Exps()[0].get());
            if (it != ir.main_ssa_types.end()) {
                if (merged_ret.type == T_UNKNOWN) merged_ret = it->second;
                else
                    merged_ret = Meet(merged_ret, it->second, registry_);
            }
        });
    }
    if (merged_ret.type == T_UNKNOWN) merged_ret = {T_NIL, -1};
    // 如果返回类型是 T_DYNAMIC（参数类型未知），尝试从表达式结构推导
    // 对于 `return n+1` 这类算术表达式，返回类型与参数一致（视为 T_INT 默认）
    if (merged_ret.type == T_DYNAMIC && func_block) {
        WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
            if (!node || node->Type() != SyntaxTreeType::Return) return;
            auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(node);
            if (!ret->Explist()) return;
            auto el_ptr = ret->Explist()->Type() == SyntaxTreeType::ExpList ? std::dynamic_pointer_cast<SyntaxTreeExplist>(ret->Explist()) : std::shared_ptr<SyntaxTreeExplist>();
            if (!el_ptr || el_ptr->Exps().empty()) return;
            InferredType derived = DeriveExprTypeForRet(el_ptr->Exps()[0], s.param_types, cfg.param_indices, ir);
            if (derived != T_UNKNOWN && derived != T_DYNAMIC) {
                merged_ret = {derived, -1};
            }
        });
    }
    s.ret_type = merged_ret;

    // ── 构建 HM 多态签名 ─────────────────────────────────────────────────
    // 对没有具体参数的函数，我们基于 return 推导一个 (params → ret) 的 HM
    // 多态签名，让调用点可以通过实际参数类型实例化。
    BuildHmSignature(s, cfg, merged_ret);
}

// ── HM 签名构建 ──────────────────────────────────────────────────────────
// 为 summary 生成 (param_hm_types) → ret_hm_type，用于调用点多态实例化。
void UnifiedTypeAnalyzer::BuildHmSignature(FuncSummary &s, const CFGFunction &cfg, const SSATypeInfo &ret_type) {
    // 只有当我们有参数记录时生成签名。如果没有参数，不标记 must_use_hm。
    if (cfg.param_indices.empty()) return;

    // 为每个参数创建一个 "通用" 类型变量。
    // 多态签名：签名中的 params 全是变量，ret 引用这些变量。
    std::vector<Type *> param_hm;
    param_hm.reserve(cfg.param_indices.size());
    for (size_t i = 0; i < cfg.param_indices.size(); ++i) {
        param_hm.push_back(hm_table_.NewVar());
    }

    // ret_hm_type：尝试基于 ret_type 推断一个有用的表达式。
    Type *ret_hm = nullptr;
    if (IsRecordInferredType(ret_type.type) && ret_type.shape_id >= 0 && registry_) {
        // 返回 record —— 把各字段值替换为 "多态变量" 如果字段类型是 T_DYNAMIC；
        // 否则保留具体类型。
        const ShapeType &sh = registry_->Get(ret_type.shape_id);
        Type *rec = HmType::MakeRecord(hm_arena_, sh.is_open);
        for (const auto &f: sh.fields) {
            RecordField hf;
            hf.name = hm_arena_.AllocString(f.name);
            hf.c_field_name = hm_arena_.AllocString(f.c_field_name);
            if (f.type == T_DYNAMIC && !param_hm.empty()) {
                // 推断是某个参数的值（保守选择第一个参数变量）。
                hf.type = param_hm[0];
            } else {
                hf.type = SsaInfoToHm({f.type, -1});
            }
            hf.optional = f.optional;
            hf.is_int_key = f.is_int_key;
            rec->fields.push_back(hf);
        }
        ret_hm = rec;
    } else if (IsNumericInferredType(ret_type.type)) {
        ret_hm = SsaInfoToHm(ret_type);
    } else if (ret_type.type == T_STRING || ret_type.type == T_BOOL || ret_type.type == T_NIL) {
        ret_hm = SsaInfoToHm(ret_type);
    } else {
        ret_hm = HmType::MakePrim(hm_arena_, TypeKind::TY_DYNAMIC);
    }

    s.param_hm_types = std::move(param_hm);
    s.ret_hm_type = ret_hm;
    s.must_use_hm = true;
}

// ── HM 多态签名实例化 ──────────────────────────────────────────────────
// 对每个调用点把形参替换为实参类型，返回 (可能更具体的) 返回类型表达式。
// 如果 signature 不可用，返回 nullptr 并设 ok=false。
Type *UnifiedTypeAnalyzer::InstantiateHmSignature(const std::vector<Type *> &param_hm_types, Type *ret_hm_type, const std::vector<Type *> &arg_hm_types, bool &ok) {
    ok = true;
    if (!ret_hm_type) {
        ok = false;
        return nullptr;
    }
    // 浅替换 ret 类型中出现的形参变量为对应实参类型。
    return SubstituteHmVars(ret_hm_type, param_hm_types, arg_hm_types, ok);
}

namespace {
// 替换 T* 中出现的形参变量（按指针相等）为对应实参类型。
// 纯函数：始终返回一个新 T*（从 arena 分配），原形参 T 不被修改。
Type *SubstituteWalk(Type *t, const std::vector<Type *> &from, const std::vector<Type *> &to, fakelua::TypeArena &arena, bool &ok) {
    if (!t) return nullptr;
    while (t && t->kind == fakelua::TypeKind::TY_VAR && t->bound) t = t->bound;
    if (!t) return nullptr;

    // 如果 t 是一个形参变量（按指针），直接替换为对应的实参 T。
    for (size_t i = 0; i < from.size(); ++i) {
        if (t == from[i] && i < to.size()) {
            return to[i];
        }
    }

    // 如果 t 是自由变量且不在形参列表中，保持原样返回。
    if (t->kind == fakelua::TypeKind::TY_VAR) return t;

    switch (t->kind) {
        case fakelua::TypeKind::TY_INT:
        case fakelua::TypeKind::TY_FLOAT:
        case fakelua::TypeKind::TY_STRING:
        case fakelua::TypeKind::TY_BOOL:
        case fakelua::TypeKind::TY_NIL:
        case fakelua::TypeKind::TY_DYNAMIC:
            return t;
        case fakelua::TypeKind::TY_FUN:
            return t;
        case fakelua::TypeKind::TY_RECORD:
        case fakelua::TypeKind::TY_RECORD_OPEN: {
            fakelua::Type *nr = fakelua::HmType::MakeRecord(arena, t->is_open);
            for (const auto &f: t->fields) {
                fakelua::RecordField nf;
                nf.name = f.name;
                nf.c_field_name = f.c_field_name;
                nf.type = SubstituteWalk(f.type, from, to, arena, ok);
                nf.optional = f.optional;
                nf.is_int_key = f.is_int_key;
                nr->fields.push_back(nf);
            }
            return nr;
        }
        case fakelua::TypeKind::TY_ARRAY:
            return fakelua::HmType::MakeArray(arena, SubstituteWalk(t->elem, from, to, arena, ok));
        case fakelua::TypeKind::TY_UNION: {
            fakelua::Type *nu = fakelua::HmType::MakeUnion(arena, {});
            for (auto *m: t->members) {
                nu->members.push_back(SubstituteWalk(m, from, to, arena, ok));
            }
            return nu;
        }
        case fakelua::TypeKind::TY_VAR:
            return t;
    }
    return t;
}
}// namespace

Type *UnifiedTypeAnalyzer::SubstituteHmVars(Type *t, const std::vector<Type *> &from, const std::vector<Type *> &to, bool &ok) {
    ok = true;
    return SubstituteWalk(t, from, to, hm_arena_, ok);
}

// ── §8 逃逸分析 ──────────────────────────────────────────────────────────

void UnifiedTypeAnalyzer::ComputeEscape(const std::string &func_name, const SyntaxTreeInterfacePtr &func_block, const CFGFunction &cfg, EscapeEnv &escape_env) {
    if (!func_block) return;

    std::function<void(const SyntaxTreeInterfacePtr &, bool)> walk;
    walk = [&](const SyntaxTreeInterfacePtr &node, bool is_in_indexing_prefix) {
        if (!node) return;
        switch (node->Type()) {
            case SyntaxTreeType::Var: {
                auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(node);
                if (v->GetVarKind() == VarKind::kSimple) {
                    if (!is_in_indexing_prefix) {
                        escape_env[v->GetName()] = true;
                    }
                } else if (v->GetVarKind() == VarKind::kDot || v->GetVarKind() == VarKind::kSquare) {
                    walk(v->GetPrefixexp(), true);
                    if (v->GetVarKind() == VarKind::kSquare) {
                        walk(v->GetExp(), false);
                    }
                }
                break;
            }
            case SyntaxTreeType::PrefixExp: {
                auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node);
                walk(pe->GetValue(), is_in_indexing_prefix);
                break;
            }
            case SyntaxTreeType::Assign: {
                auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(node);
                auto vl = assign->Varlist() ? std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist()) : nullptr;
                auto el = assign->Explist() ? std::dynamic_pointer_cast<SyntaxTreeExplist>(assign->Explist()) : nullptr;
                if (vl) {
                    for (auto &var: vl->Vars()) {
                        if (var) {
                            auto var_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(var);
                            if (var_ptr) {
                                if (var_ptr->GetVarKind() == VarKind::kSimple) {
                                    // Write to simple var: does not cause it to escape
                                } else {
                                    // Write to field: walk prefix with indexing = true
                                    walk(var_ptr, false);
                                }
                            }
                        }
                    }
                }
                if (el) {
                    for (auto &exp: el->Exps()) {
                        walk(exp, false);
                    }
                }
                break;
            }
            case SyntaxTreeType::LocalVar: {
                auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
                if (local_var->Explist()) {
                    walk(local_var->Explist(), false);
                }
                break;
            }
            case SyntaxTreeType::Block: {
                auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
                for (auto &stmt: block->Stmts()) walk(stmt, false);
                break;
            }
            case SyntaxTreeType::Return: {
                auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(node);
                walk(ret->Explist(), false);
                break;
            }
            case SyntaxTreeType::VarList: {
                auto varlist = std::dynamic_pointer_cast<SyntaxTreeVarlist>(node);
                for (auto &var: varlist->Vars()) walk(var, false);
                break;
            }
            case SyntaxTreeType::ExpList: {
                auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
                for (auto &exp: explist->Exps()) walk(exp, false);
                break;
            }
            case SyntaxTreeType::FunctionCall: {
                auto functioncall = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
                walk(functioncall->prefixexp(), false);
                walk(functioncall->Args(), false);
                break;
            }
            case SyntaxTreeType::TableConstructor: {
                auto tc = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node);
                walk(tc->Fieldlist(), false);
                break;
            }
            case SyntaxTreeType::FieldList: {
                auto fieldlist = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
                for (auto &field: fieldlist->Fields()) walk(field, false);
                break;
            }
            case SyntaxTreeType::Field: {
                auto field = std::dynamic_pointer_cast<SyntaxTreeField>(node);
                walk(field->Key(), false);
                walk(field->Value(), false);
                break;
            }
            case SyntaxTreeType::While: {
                auto while_node = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
                walk(while_node->Exp(), false);
                walk(while_node->Block(), false);
                break;
            }
            case SyntaxTreeType::Repeat: {
                auto rep = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
                walk(rep->Block(), false);
                walk(rep->Exp(), false);
                break;
            }
            case SyntaxTreeType::If: {
                auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
                walk(if_node->Exp(), false);
                walk(if_node->Block(), false);
                walk(if_node->ElseIfs(), false);
                walk(if_node->ElseBlock(), false);
                break;
            }
            case SyntaxTreeType::ElseIfList: {
                auto elseifs = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(node);
                for (auto &exp: elseifs->ElseifExps()) walk(exp, false);
                for (auto &block: elseifs->ElseifBlocks()) walk(block, false);
                break;
            }
            case SyntaxTreeType::ForLoop: {
                auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
                walk(for_loop->ExpBegin(), false);
                walk(for_loop->ExpEnd(), false);
                walk(for_loop->ExpStep(), false);
                walk(for_loop->Block(), false);
                break;
            }
            case SyntaxTreeType::ForIn: {
                auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
                walk(for_in->Explist(), false);
                walk(for_in->Block(), false);
                break;
            }
            case SyntaxTreeType::Function: {
                auto func_node = std::dynamic_pointer_cast<SyntaxTreeFunction>(node);
                walk(func_node->Funcbody(), false);
                break;
            }
            case SyntaxTreeType::LocalFunction: {
                auto func_node = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node);
                walk(func_node->Funcbody(), false);
                break;
            }
            case SyntaxTreeType::FuncBody: {
                auto func_body = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(node);
                walk(func_body->Block(), false);
                break;
            }
            case SyntaxTreeType::Exp: {
                auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
                walk(exp->Left(), false);
                walk(exp->Right(), false);
                break;
            }
            case SyntaxTreeType::Args: {
                auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node);
                if (args->GetArgsKind() == ArgsKind::kExpList) {
                    walk(args->Explist(), false);
                } else if (args->GetArgsKind() == ArgsKind::kTableConstructor) {
                    walk(args->Tableconstructor(), false);
                } else if (args->GetArgsKind() == ArgsKind::kString) {
                    walk(args->String(), false);
                }
                break;
            }
            default:
                break;
        }
    };
    walk(func_block, false);
}

}// namespace fakelua
