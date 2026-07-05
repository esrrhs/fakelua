#include "compile/unified_type_analyzer.h"
#include "compile/syntax_tree.h"
#include <queue>

namespace fakelua {

// ─────────────────────────────────────────────────────────────────────────
// 主分析入口
// ─────────────────────────────────────────────────────────────────────────
void UnifiedTypeAnalyzer::Analyze(const std::string &func_name,
                                   const SyntaxTreeInterfacePtr &func_block,
                                   const CFGFunction &cfg,
                                   SSAFunction &ssa,
                                   InferResult &ir,
                                   int bitmask,
                                   const ParamAssumption &param_assumptions) {
    cur_func_name_ = func_name;

    // 初始化 shape_registry
    if (!ir.shape_registry) {
        ir.shape_registry = std::make_shared<ShapeRegistry>();
    }
    registry_ = ir.shape_registry.get();

    // 构建变量名 → SSA 版本映射（SSABuilder 骨架不实现完整 use_versions 映射）
    auto name_ver = BuildVarNameVersionMap(ssa);

    // 运行 worklist
    TypeEnv version_types;
    RunWorklist(ssa, param_assumptions, version_types, ir);

    // 合并结果到 InferResult
    if (bitmask == -1) {
        // 主分析
        for (auto &[ver, ty] : version_types) {
            ir.ssa_version_types[ver] = ty;
        }

        // 推导所有 AST 节点的类型
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
                        auto ty = InferExprType(node, ssa, version_types, ir, name_ver);
                        ir.main_ssa_types[node.get()] = ty;
                        ir.node_ssa_version[node.get()] = -1;  // 不精确——留作后续
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
    } else {
        // 特化分析
        auto &snap = ir.spec_ssa_snapshots[func_name];
        if ((size_t)bitmask >= snap.size()) snap.resize((size_t)bitmask + 1);

        // 记录特化快照（第一轮：从 AST 节点直接推导）
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
                        auto ty = InferExprType(node, ssa, version_types, ir, name_ver);
                        snap[(size_t)bitmask][node.get()] = ty;
                        break;
                    }
                    default:
                        break;
                }
            });
        }

        // 第二轮：对 local 声明和赋值，用快照已知类型回写 RHS 表达式的类型。
        // 这样 CGen 的特化变形体看到 local x = <expr> 时能根据快照决定 x 的原生类型。
        if (func_block) {
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
                                // 已推导为 T_INT/T_FLOAT：记录到 snap 供后续 CGen 查询
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
// Worklist 主循环（简化实现）
// ─────────────────────────────────────────────────────────────────────────
void UnifiedTypeAnalyzer::RunWorklist(const SSAFunction &ssa,
                                       const ParamAssumption &param_assumptions,
                                       TypeEnv &version_types,
                                       const InferResult &ir) {
    // 初始化参数版本类型
    for (size_t i = 0; i < ssa.param_versions.size(); ++i) {
        int ver = ssa.param_versions[i];
        auto pit = param_assumptions.find(ver);
        version_types[ver] = (pit != param_assumptions.end())
            ? pit->second : SSATypeInfo{T_DYNAMIC, -1};
    }
}

// 为当前 SSA 函数构建变量名 → 最新 SSA 版本号的映射。
// 由于 SSABuilder 仍是骨架（不实现 Cytron 算法），退而求其次用 var_all_versions
// 中每个变量的最后一个版本号（对过程内单赋值形式的简单函数足够）。
UnifiedTypeAnalyzer::VarNameToVersion UnifiedTypeAnalyzer::BuildVarNameVersionMap(const SSAFunction &ssa) {
    VarNameToVersion m;
    for (auto &[name, vers] : ssa.var_all_versions) {
        if (!vers.empty()) m[name] = vers.back();
    }
    return m;
}

// ─────────────────────────────────────────────────────────────────────────
// 递归推导表达式类型
// name_ver 闭包提供 var_name → SSA version 的查找；
// 当 SSABuilder 仍是骨架无法提供 use_versions 时，通过此 map 解析变量类型。
// ─────────────────────────────────────────────────────────────────────────
SSATypeInfo UnifiedTypeAnalyzer::InferExprType(
    const SyntaxTreeInterfacePtr &expr,
    const SSAFunction &ssa,
    const TypeEnv &version_types,
    const InferResult &ir,
    const std::unordered_map<std::string, int> &name_ver) {

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
                    auto lt = InferExprType(e->Left(), ssa, version_types, ir, name_ver);
                    auto rt = InferExprType(e->Right(), ssa, version_types, ir, name_ver);
                    return {ShapeRegistry::MeetType(lt.type, rt.type), -1};
                }
                case ExpKind::kUnop: {
                    return InferExprType(e->Right(), ssa, version_types, ir, name_ver);
                }
                case ExpKind::kPrefixExp: {
                    return InferExprType(e->Right(), ssa, version_types, ir, name_ver);
                }
                case ExpKind::kTableConstructor: {
                    int shape_id = BuildShapeFromCtor(expr, ssa, version_types, ir);
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
                // 1. 优先通过 SSA use_versions 精确查找
                auto vit = ssa.use_versions.find(expr.get());
                if (vit != ssa.use_versions.end()) {
                    auto tit = version_types.find(vit->second);
                    if (tit != version_types.end()) return tit->second;
                }
                // 2. 回退：通过变量名查找其 SSABuilder 分配的最新版本
                auto nit = name_ver.find(vname);
                if (nit != name_ver.end()) {
                    auto tit = version_types.find(nit->second);
                    if (tit != version_types.end()) return tit->second;
                }
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
            return InferExprType(pe->GetValue(), ssa, version_types, ir, name_ver);
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
            // kArray field
            auto *ve = (fp->Value() && fp->Value()->Type() == SyntaxTreeType::Exp)
                ? static_cast<SyntaxTreeExp *>(fp->Value().get()) : nullptr;
            if (fp->GetFieldKind() == FieldKind::kArray && fp->Key() == nullptr) {
                fd.name = std::to_string(shape.fields.size() + 1);
                fd.is_int_key = true;
                fd.c_field_name = "_int_" + fd.name;
            } else {
                // explicit key
                auto *ke = fp->Key() && fp->Key()->Type() == SyntaxTreeType::Exp
                    ? static_cast<SyntaxTreeExp *>(fp->Key().get()) : nullptr;
                if (ke && ke->GetExpKind() == ExpKind::kNumber) {
                    fd.name = ke->ExpValue();
                    fd.is_int_key = (fd.name.find('.') == std::string::npos);
                    fd.c_field_name = fd.is_int_key ? "_int_" + fd.name : "_float_" + fd.name;
                } else {
                    continue; // skip non-static key
                }
            }
        }

        // 推导字段值类型
        auto val_ty = InferExprType(fp->Value(), ssa, version_types, ir, {});
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
    // 递归
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
            // already handled above
            break;
        default:
            break;
    }
}

void UnifiedTypeAnalyzer::BuildSummary(const std::string & /*func_name*/,
                                        const SyntaxTreeInterfacePtr & /*func_block*/,
                                        const SSAFunction &ssa,
                                        const TypeEnv &version_types,
                                        InferResult &ir) {
    // 收集返回类型简化实现
    // 完整实现需要遍历所有 return 节点
}

// ── 发现可特化参数 ──────────────────────────────────────────────────────
std::vector<UnifiedTypeAnalyzer::SpecParam>
UnifiedTypeAnalyzer::FindSpecializableParams(const SyntaxTreeInterfacePtr &func_block,
                                              const CFGFunction &cfg,
                                              const SSAFunction &ssa,
                                              const InferResult &ir) {
    std::vector<SpecParam> result;
    if (!func_block) return result;

    // 收集函数参数名（优先从 FuncBody 结构恢复；fallback 到 cfg.param_indices）
    std::unordered_set<std::string> param_names;
    if (func_block->Type() == SyntaxTreeType::FuncBody) {
        auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(func_block);
        if (fb && fb->Parlist() && fb->Parlist()->Type() == SyntaxTreeType::ParList) {
            auto pl = std::dynamic_pointer_cast<SyntaxTreeParlist>(fb->Parlist());
            if (pl && pl->Namelist() && pl->Namelist()->Type() == SyntaxTreeType::NameList) {
                auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(pl->Namelist());
                for (const auto &n : nl->Names()) param_names.insert(n);
            }
        }
    }
    // Fallback：当 func_block 是函数体 Block 而非 FuncBody 时，cfg.param_indices 持有参数名
    if (param_names.empty()) {
        for (const auto &[pname, _] : cfg.param_indices) param_names.insert(pname);
    }

    // 收集表达式中直接出现的简单变量名（不进入 Call/Index 内部）
    std::unordered_set<std::string> expr_vars;
    std::function<void(const SyntaxTreeInterfacePtr&, int)> collect_vars;
    collect_vars = [&](const SyntaxTreeInterfacePtr &node, int depth) {
        if (!node) return;
        if (node->Type() == SyntaxTreeType::Var) {
            auto *v = static_cast<SyntaxTreeVar *>(node.get());
            if (v->GetVarKind() == VarKind::kSimple && param_names.count(v->GetName())) {
                expr_vars.insert(v->GetName());
            }
            return;
        }
        if (node->Type() == SyntaxTreeType::PrefixExp) {
            auto *pe = static_cast<SyntaxTreePrefixexp *>(node.get());
            if (pe->GetPrefixKind() == PrefixExpKind::kVar) {
                collect_vars(pe->GetValue(), depth);
            } else if (pe->GetPrefixKind() == PrefixExpKind::kExp && pe->GetValue()) {
                // 括号表达式 (exp) — 继续深入
                collect_vars(pe->GetValue(), depth);
            }
            return;
        }
        if (node->Type() != SyntaxTreeType::Exp) return;
        auto *e = static_cast<SyntaxTreeExp *>(node.get());
        auto k = e->GetExpKind();
        if (k == ExpKind::kNumber || k == ExpKind::kString || k == ExpKind::kNil ||
            k == ExpKind::kTrue || k == ExpKind::kFalse) return;
        if (k == ExpKind::kPrefixExp) {
            if (e->Right()) collect_vars(e->Right(), depth);
            return;
        }
        if (k == ExpKind::kBinop) {
            auto *bop = e->Op() ? static_cast<SyntaxTreeBinop *>(e->Op().get()) : nullptr;
            if (bop) {
                auto opk = bop->GetOpKind();
                bool is_arith = (opk == BinOpKind::kPlus || opk == BinOpKind::kMinus ||
                                opk == BinOpKind::kStar || opk == BinOpKind::kSlash ||
                                opk == BinOpKind::kDoubleSlash || opk == BinOpKind::kPow ||
                                opk == BinOpKind::kMod || opk == BinOpKind::kBitAnd ||
                                opk == BinOpKind::kBitOr || opk == BinOpKind::kXor ||
                                opk == BinOpKind::kLeftShift || opk == BinOpKind::kRightShift);
                if (is_arith && depth < 4) {
                    if (e->Left()) collect_vars(e->Left(), depth + 1);
                    if (e->Right()) collect_vars(e->Right(), depth + 1);
                    return;
                }
                // 三元模式 (cond) and val1 or val2：
                // AST: and(Left=cond, Right=val1), or(Left=and(...,val1), Right=val2)
                if ((opk == BinOpKind::kAnd || opk == BinOpKind::kOr) && depth < 4) {
                    const SyntaxTreeInterfacePtr &val_node = e->Right();
                    const SyntaxTreeInterfacePtr &cond_node = e->Left();
                    if (val_node && val_node->Type() == SyntaxTreeType::Exp) {
                        auto ve = std::dynamic_pointer_cast<SyntaxTreeExp>(val_node);
                        if (ve && ve->GetExpKind() == ExpKind::kNumber) {
                            if (cond_node) collect_vars(cond_node, depth + 1);
                        }
                    }
                }
                // depth>0 意味着我们从三元模式递归进来，比较/逻辑操作符的操作数也要收集。
                if (depth > 0 && depth < 4) {
                    if (e->Left()) collect_vars(e->Left(), depth + 1);
                    if (e->Right()) collect_vars(e->Right(), depth + 1);
                    return;
                }
            }
        }
        if (k == ExpKind::kUnop) {
            if (e->Right()) collect_vars(e->Right(), depth + 1);
            return;
        }
    };

    // 递归遍历函数体，找到所有 Exp 节点。
    // WalkSyntaxTree 不会深入 Exp 子表达式，需要 custom visitor。
    std::function<void(const SyntaxTreeInterfacePtr&)> visit_all;
    visit_all = [&](const SyntaxTreeInterfacePtr &node) {
        if (!node) return;
        if (node->Type() == SyntaxTreeType::Exp) {
            auto *e = static_cast<SyntaxTreeExp *>(node.get());
            // 对所有 Binop/Unop 调用 collect_vars 进行遍历：
            // collect_vars 内部会根据 depth 和 opkind 判断是否收集参数。
            if (e->GetExpKind() == ExpKind::kBinop || e->GetExpKind() == ExpKind::kUnop || e->GetExpKind() == ExpKind::kPrefixExp) {
                // 顶层 Binop: 以 depth=0 调用；其内部会处理算术/三元/比较的回退收集
                collect_vars(node, 0);
            }
            return;
        }
        // Recurse via WalkSyntaxTree semantics for non-Exp nodes
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
    visit_all(func_block);

    // 映射 expr_vars → SpecParam（含 param_index，按 param_index 排序并去重）
    for (const auto &name : expr_vars) {
        auto it = cfg.param_indices.find(name);
        if (it != cfg.param_indices.end()) {
            result.push_back({it->second, true, false});
        }
    }
    // 去重并用 param_index 排序
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
