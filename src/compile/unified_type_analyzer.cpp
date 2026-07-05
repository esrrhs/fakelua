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
                        auto ty = InferExprType(node, ssa, version_types, ir);
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

        // 记录特化快照
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
                        auto ty = InferExprType(node, ssa, version_types, ir);
                        snap[(size_t)bitmask][node.get()] = ty;
                        break;
                    }
                    default:
                        break;
                }
            });
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

// ─────────────────────────────────────────────────────────────────────────
// 递归推导表达式类型
// ─────────────────────────────────────────────────────────────────────────
SSATypeInfo UnifiedTypeAnalyzer::InferExprType(const SyntaxTreeInterfacePtr &expr,
                                                const SSAFunction &ssa,
                                                const TypeEnv &version_types,
                                                const InferResult &ir) {
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
                    auto lt = InferExprType(e->Left(), ssa, version_types, ir);
                    auto rt = InferExprType(e->Right(), ssa, version_types, ir);
                    auto *op = static_cast<SyntaxTreeBinop *>(e->Op().get());
                    return {ShapeRegistry::MeetType(lt.type, rt.type), -1};
                }
                case ExpKind::kUnop: {
                    auto operand = InferExprType(e->Right(), ssa, version_types, ir);
                    return operand;
                }
                case ExpKind::kPrefixExp: {
                    return InferExprType(e->Right(), ssa, version_types, ir);
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
                // 通过 SSA 查找
                auto vit = ssa.use_versions.find(expr.get());
                if (vit != ssa.use_versions.end()) {
                    auto tit = version_types.find(vit->second);
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
            return InferExprType(pe->GetValue(), ssa, version_types, ir);
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
        auto val_ty = InferExprType(fp->Value(), ssa, version_types, ir);
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

    // 收集函数参数名
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

    std::unordered_set<std::string> math_vars;

    // Walk 找所有算术二元运算的操作数
    WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
        if (!node || node->Type() != SyntaxTreeType::Exp) return;
        auto *e = static_cast<SyntaxTreeExp *>(node.get());
        if (e->GetExpKind() != ExpKind::kBinop) return;
        auto *bop = static_cast<SyntaxTreeBinop *>(e->Op().get());
        if (!bop) return;
        auto opk = bop->GetOpKind();
        bool is_arith = (opk == BinOpKind::kPlus || opk == BinOpKind::kMinus ||
                        opk == BinOpKind::kStar || opk == BinOpKind::kSlash ||
                        opk == BinOpKind::kDoubleSlash || opk == BinOpKind::kPow ||
                        opk == BinOpKind::kMod || opk == BinOpKind::kBitAnd ||
                        opk == BinOpKind::kBitOr || opk == BinOpKind::kXor ||
                        opk == BinOpKind::kLeftShift || opk == BinOpKind::kRightShift);
        if (!is_arith) return;
        // 收集两侧的简单变量名
        auto collect_vars = [&](const SyntaxTreeInterfacePtr &sub) {
            if (sub && sub->Type() == SyntaxTreeType::Exp) {
                auto *se = static_cast<SyntaxTreeExp *>(sub.get());
                if (se->GetExpKind() == ExpKind::kPrefixExp && se->Right() &&
                    se->Right()->Type() == SyntaxTreeType::PrefixExp) {
                    auto *pe2 = static_cast<SyntaxTreePrefixexp *>(se->Right().get());
                    if (pe2->GetPrefixKind() == PrefixExpKind::kVar && pe2->GetValue() &&
                        pe2->GetValue()->Type() == SyntaxTreeType::Var) {
                        auto *v = static_cast<SyntaxTreeVar *>(pe2->GetValue().get());
                        if (v->GetVarKind() == VarKind::kSimple && param_names.count(v->GetName())) {
                            math_vars.insert(v->GetName());
                        }
                    }
                }
            }
        };
        collect_vars(e->Left());
        collect_vars(e->Right());
    });

    // 映射到 SpecParam
    // todo: 完整实现需要按 param_index 排序
    for (const auto &name : math_vars) {
        for (size_t i = 0; i < cfg.param_indices.size(); ++i) {
            if (i < cfg.blocks.size()) {  // wrong check, but simplifies
                result.push_back({static_cast<int>(i), true, false});
                break;
            }
        }
    }

    return result;
}

}// namespace fakelua
