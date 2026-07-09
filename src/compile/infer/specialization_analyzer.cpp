#include "compile/infer/specialization_analyzer.h"
#include "compile/infer/cfg.h"
#include "compile/infer/ssa.h"
#include "compile/infer/unified_type_analyzer.h"
#include "compile/syntax_tree.h"
#include "util/common.h"
#include "util/debug.h"
#include "util/exception.h"
#include <algorithm>

namespace fakelua {

namespace {

bool CheckNodeChangeCommon(const SyntaxTreeInterfacePtr &node,
                           const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                           const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                           bool improvement_mode) {
    const auto it_typed = typed_map.find(node.get());
    const auto it_compare = compare_map.find(node.get());
    if (it_typed == typed_map.end() || it_compare == compare_map.end()) {
        return false;
    }
    return IsNumericInferredType(it_typed->second.type) &&
           (improvement_mode ? (it_compare->second.type == T_DYNAMIC) : (it_compare->second.type != it_typed->second.type));
}
} // namespace

void SpecializationAnalyzer::Analyze(const ParseResult &pr, InferResult &ir) {
    MathFuncInfoMap math_func_info = IdentifyMathParams(pr, ir);

    std::unordered_map<std::string, FuncRetInfo> func_ret_cache;
    for (const auto &[func_name, info] : math_func_info) {
        FuncRetInfo ret_info;
        ret_info.ends_with_return = CollectReturnExps(info.block, ret_info.ret_exps);
        func_ret_cache[func_name] = std::move(ret_info);
    }

    InferSpecializationReturnTypes(ir, math_func_info, func_ret_cache);
}

SpecializationAnalyzer::MathFuncInfoMap SpecializationAnalyzer::IdentifyMathParams(const ParseResult &pr, InferResult &ir) {
    MathFuncInfoMap math_func_info;
    if (!pr.chunk || pr.chunk->Type() != SyntaxTreeType::Block) {
        return math_func_info;
    }

    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(pr.chunk);

    for (const auto &stmt: top_block->Stmts()) {
        std::string name;
        SyntaxTreeInterfacePtr funcbody;
        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            const auto funcname = std::dynamic_pointer_cast<SyntaxTreeFuncname>(func->Funcname());
            const auto fnlist = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(funcname->FuncNameList());
            name = fnlist->Funcnames()[0];
            funcbody = func->Funcbody();
        } else if (stmt->Type() == SyntaxTreeType::LocalFunction) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt);
            name = func->Name();
            funcbody = func->Funcbody();
        }
        if (name.empty() || !funcbody) {
            continue;
        }

        const auto funcbody_ptr = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
        std::vector<std::string> params;
        if (funcbody_ptr->Parlist()) {
            const auto pl = std::dynamic_pointer_cast<SyntaxTreeParlist>(funcbody_ptr->Parlist());
            if (pl && pl->Namelist()) {
                const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(pl->Namelist());
                if (nl) {
                    params = nl->Names();
                }
            }
        }

        auto ssa_it = ir.ssa_functions.find(name);
        if (ssa_it == ir.ssa_functions.end()) {
            continue;
        }

        MathFuncInfo info;
        info.block = funcbody_ptr->Block();
        info.params = params;
        info.param_versions = ssa_it->second->param_versions;

        math_func_info[name] = info;
    }

    for (int pass = 0; pass < kMaxSpecIterations; ++pass) {
        bool new_discovery = false;
        for (const auto &[name, info] : math_func_info) {
            if (ir.math_param_positions.contains(name)) {
                continue;
            }

            auto cfg_it = ir.cfg_functions.find(name);
            auto ssa_it = ir.ssa_functions.find(name);
            if (cfg_it == ir.cfg_functions.end() || ssa_it == ir.ssa_functions.end()) {
                continue;
            }

            const auto baseline = RunTrialInference(name, info.block, *cfg_it->second, *ssa_it->second,
                                                   MakeAssumedParamTypes(info.param_versions, -1, T_DYNAMIC, T_DYNAMIC), ir);
            const auto all_int = RunTrialInference(name, info.block, *cfg_it->second, *ssa_it->second,
                                                  MakeAssumedParamTypes(info.param_versions, -1, T_INT, T_INT), ir);

            const auto math_indices = FindMathParamIndices(name, info, baseline, all_int, ir.math_param_positions, ir);

            if (math_indices.empty()) {
                continue;
            }
            if (math_indices.size() > kMaxMathSpecializedParams) {
                continue;
            }

            ir.math_param_positions[name] = math_indices;
            new_discovery = true;
        }
        if (!new_discovery) {
            break;
        }
    }

    MathFuncInfoMap filtered_info;
    for (const auto &[name, info] : math_func_info) {
        if (ir.math_param_positions.contains(name)) {
            filtered_info[name] = info;
        }
    }
    return filtered_info;
}

std::vector<int> SpecializationAnalyzer::FindMathParamIndices(const std::string &name, const MathFuncInfo &info,
                                                              const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &baseline,
                                                              const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &all_int,
                                                              const std::unordered_map<std::string, std::vector<int>> &known_math_positions,
                                                              InferResult &ir) {
    std::vector<int> math_indices;
    if (!CheckArithmeticTypeChanges(all_int, baseline, info.block, true, known_math_positions)) {
        return math_indices;
    }

    auto cfg_it = ir.cfg_functions.find(name);
    auto ssa_it = ir.ssa_functions.find(name);
    if (cfg_it == ir.cfg_functions.end() || ssa_it == ir.ssa_functions.end()) {
        return math_indices;
    }

    for (int i = 0; i < static_cast<int>(info.param_versions.size()); ++i) {
        const auto without_p_assumed = MakeAssumedParamTypes(info.param_versions, i, T_DYNAMIC, T_INT);
        const auto without_p_map = RunTrialInference(name, info.block, *cfg_it->second, *ssa_it->second, without_p_assumed, ir);
        if (CheckArithmeticTypeChanges(all_int, without_p_map, info.block, false, known_math_positions)) {
            math_indices.push_back(i);
        }
    }
    return math_indices;
}

void SpecializationAnalyzer::InferSpecializationReturnTypes(InferResult &ir, const MathFuncInfoMap &math_func_info,
                                                            const std::unordered_map<std::string, FuncRetInfo> &func_ret_cache) {
    for (const auto &[func_name, info] : math_func_info) {
        const auto &math_params = ir.math_param_positions.at(func_name);
        ir.specialization_return_types[func_name].assign(static_cast<size_t>(1 << static_cast<int>(math_params.size())), T_INT);
        ir.specialization_snapshots[func_name].resize(static_cast<size_t>(1 << static_cast<int>(math_params.size())));
    }

    for (int round = 0; round < kMaxSpecIterations; ++round) {
        bool changed = false;
        for (const auto &[func_name, info] : math_func_info) {
            const auto &math_indices = ir.math_param_positions.at(func_name);
            const auto &ret_info = func_ret_cache.at(func_name);
            const int num_specs = 1 << static_cast<int>(math_indices.size());

            auto cfg_it = ir.cfg_functions.find(func_name);
            auto ssa_it = ir.ssa_functions.find(func_name);
            if (cfg_it == ir.cfg_functions.end() || ssa_it == ir.ssa_functions.end()) {
                continue;
            }

            for (int bitmask = 0; bitmask < num_specs; ++bitmask) {
                auto assumed = MakeSpecializedParamTypes(info.param_versions, math_indices, bitmask);
                auto new_snapshot = RunTrialInference(func_name, info.block, *cfg_it->second, *ssa_it->second, assumed, ir);

                const auto new_ret = ComputeReturnTypeFromSnapshot(new_snapshot, ret_info);

                const auto bitmask_sz = static_cast<size_t>(bitmask);
                auto &cur_ret = ir.specialization_return_types[func_name][bitmask_sz];
                auto &cur_snap = ir.specialization_snapshots[func_name][bitmask_sz];

                if (new_ret != cur_ret || new_snapshot != cur_snap) {
                    cur_ret = new_ret;
                    cur_snap = std::move(new_snapshot);
                    changed = true;
                }
            }
        }
        if (!changed) {
            break;
        }
    }
}

bool SpecializationAnalyzer::CheckArithmeticTypeChanges(const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                                                        const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                                                        const SyntaxTreeInterfacePtr &func_block, bool improvement_mode,
                                                        const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const {
    bool found = false;
    WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
        if (found) {
            return;
        }
        if (CheckArithmeticNodeChange(node, typed_map, compare_map, improvement_mode) ||
            CheckComparisonNodeChange(node, typed_map, compare_map, improvement_mode) ||
            CheckForLoopNodeChange(node, typed_map, compare_map, improvement_mode) ||
            CheckCallNodeChange(node, typed_map, compare_map, math_param_positions)) {
            found = true;
        }
    });
    return found;
}

bool SpecializationAnalyzer::CheckArithmeticNodeChange(const SyntaxTreeInterfacePtr &node,
                                                       const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                                                       const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                                                       bool improvement_mode) const {
    return IsArithmeticExpr(node) && CheckNodeChangeCommon(node, typed_map, compare_map, improvement_mode);
}

bool SpecializationAnalyzer::CheckComparisonNodeChange(const SyntaxTreeInterfacePtr &node,
                                                       const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                                                       const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                                                       bool improvement_mode) const {
    if (!IsNativeComparisonExpr(node)) {
        return false;
    }
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
    const auto left = exp->Left();
    const auto right = exp->Right();
    DEBUG_ASSERT(left && right);

    const auto lt_typed = typed_map.find(left.get());
    const auto rt_typed = typed_map.find(right.get());
    const auto lt_compare = compare_map.find(left.get());
    const auto rt_compare = compare_map.find(right.get());

    if (lt_typed == typed_map.end() || rt_typed == typed_map.end() ||
        lt_compare == compare_map.end() || rt_compare == compare_map.end()) {
        return false;
    }

    if (IsNumericInferredType(lt_typed->second.type) && IsNumericInferredType(rt_typed->second.type)) {
        if (improvement_mode) {
            return (lt_compare->second.type == T_DYNAMIC || rt_compare->second.type == T_DYNAMIC);
        } else {
            return (lt_compare->second.type != lt_typed->second.type || rt_compare->second.type != rt_typed->second.type);
        }
    }
    return false;
}

bool SpecializationAnalyzer::CheckForLoopNodeChange(const SyntaxTreeInterfacePtr &node,
                                                    const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                                                    const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                                                    bool improvement_mode) const {
    return (node->Type() == SyntaxTreeType::ForLoop) && CheckNodeChangeCommon(node, typed_map, compare_map, improvement_mode);
}

bool SpecializationAnalyzer::CheckCallNodeChange(const SyntaxTreeInterfacePtr &node,
                                                 const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &typed_map,
                                                 const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &compare_map,
                                                 const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const {
    if (node->Type() != SyntaxTreeType::FunctionCall) {
        return false;
    }
    const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
    DEBUG_ASSERT(fc);

    std::string callee_name;
    if (fc->prefixexp() && fc->prefixexp()->Type() == SyntaxTreeType::Var) {
        callee_name = std::dynamic_pointer_cast<SyntaxTreeVar>(fc->prefixexp())->GetName();
    } else if (fc->prefixexp() && fc->prefixexp()->Type() == SyntaxTreeType::PrefixExp) {
        auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
        if (pe->GetValue() && pe->GetValue()->Type() == SyntaxTreeType::Var) {
            callee_name = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue())->GetName();
        }
    }

    if (callee_name.empty()) {
        return false;
    }

    if (const auto math_it = math_param_positions.find(callee_name); math_it != math_param_positions.end()) {
        const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
        if (!args_ptr) {
            return false;
        }
        const auto raw_args = ExtractCallRawArgs(args_ptr);
        for (const int param_pos : math_it->second) {
            if (param_pos >= static_cast<int>(raw_args.size())) {
                return false;
            }
            const auto &arg = raw_args[static_cast<size_t>(param_pos)];
            const auto it_typed = typed_map.find(arg.get());
            const auto it_comp = compare_map.find(arg.get());
            if (it_typed == typed_map.end() || it_comp == compare_map.end()) {
                return false;
            }
            if ((it_typed->second.type == T_INT || it_typed->second.type == T_FLOAT) && it_comp->second.type != it_typed->second.type) {
                return true;
            }
        }
    }
    return false;
}

bool SpecializationAnalyzer::IsArithmeticExpr(const SyntaxTreeInterfacePtr &node) const {
    if (node->Type() != SyntaxTreeType::Exp) {
        return false;
    }
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
    if (exp->GetExpKind() == ExpKind::kBinop) {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(exp->Op());
        DEBUG_ASSERT(op);
        const auto k = op->GetOpKind();
        return k == BinOpKind::kPlus || k == BinOpKind::kMinus || k == BinOpKind::kStar || k == BinOpKind::kSlash || k == BinOpKind::kDoubleSlash || k == BinOpKind::kPow || k == BinOpKind::kMod ||
               k == BinOpKind::kBitAnd || k == BinOpKind::kXor || k == BinOpKind::kBitOr || k == BinOpKind::kLeftShift || k == BinOpKind::kRightShift;
    }
    if (exp->GetExpKind() == ExpKind::kUnop) {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(exp->Op());
        DEBUG_ASSERT(op);
        return op->GetOpKind() == UnOpKind::kMinus || op->GetOpKind() == UnOpKind::kBitNot;
    }
    return false;
}

bool SpecializationAnalyzer::IsNativeComparisonExpr(const SyntaxTreeInterfacePtr &node) const {
    if (node->Type() != SyntaxTreeType::Exp) {
        return false;
    }
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
    if (exp->GetExpKind() != ExpKind::kBinop) {
        return false;
    }
    const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(exp->Op());
    DEBUG_ASSERT(op);
    const auto k = op->GetOpKind();
    return k == BinOpKind::kLess || k == BinOpKind::kLessEqual || k == BinOpKind::kMore || k == BinOpKind::kMoreEqual;
}

std::unordered_map<int, SSATypeInfo> SpecializationAnalyzer::MakeAssumedParamTypes(const std::vector<int> &param_versions, int special_idx,
                                                                                   InferredType special_type, InferredType default_type) const {
    std::unordered_map<int, SSATypeInfo> assumed;
    for (size_t i = 0; i < param_versions.size(); ++i) {
        assumed[param_versions[i]] = { (special_idx >= 0 && static_cast<int>(i) == special_idx) ? special_type : default_type, -1 };
    }
    return assumed;
}

std::unordered_map<int, SSATypeInfo> SpecializationAnalyzer::MakeSpecializedParamTypes(const std::vector<int> &param_versions,
                                                                                       const std::vector<int> &math_indices, int bitmask) const {
    std::unordered_map<int, SSATypeInfo> assumed;
    for (int ver : param_versions) {
        assumed[ver] = { T_DYNAMIC, -1 };
    }
    for (int i = 0; i < static_cast<int>(math_indices.size()); ++i) {
        assumed[param_versions[math_indices[i]]] = { (MathParamKindOf(bitmask, i) == kMathParamFloat) ? T_FLOAT : T_INT, -1 };
    }
    return assumed;
}

std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> SpecializationAnalyzer::RunTrialInference(
    const std::string &func_name, const SyntaxTreeInterfacePtr &func_block, const CFGFunction &cfg, SSAFunction &ssa,
    const std::unordered_map<int, SSATypeInfo> &assumed_types, InferResult &ir) {

    UnifiedTypeAnalyzer trial_uta(ir.shape_registry.get());
    InferResult trial_ir;
    trial_ir.shape_registry = ir.shape_registry;
    trial_ir.func_summaries = ir.func_summaries;
    trial_ir.cfg_functions = ir.cfg_functions;
    trial_ir.ssa_functions = ir.ssa_functions;
    trial_ir.math_param_positions = ir.math_param_positions;
    trial_ir.specialization_return_types = ir.specialization_return_types;

    trial_uta.Analyze(func_name, func_block, cfg, ssa, trial_ir, &assumed_types);

    return trial_ir.main_ssa_types;
}

bool SpecializationAnalyzer::AllPathsReturn(const SyntaxTreeInterfacePtr &block_node) const {
    DEBUG_ASSERT(block_node);
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(block_node);
    if (!block || block->Stmts().empty()) {
        return false;
    }
    switch (const auto &last = block->Stmts().back(); last->Type()) {
        case SyntaxTreeType::Return:
            return true;
        case SyntaxTreeType::Block:
            return AllPathsReturn(last);
        case SyntaxTreeType::If: {
            const auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(last);
            if (!AllPathsReturn(if_node->Block())) {
                return false;
            }
            if (const auto elseifs = if_node->ElseIfs()) {
                const auto el = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseifs);
                for (const auto &blk: el->ElseifBlocks()) {
                    if (!AllPathsReturn(blk)) {
                        return false;
                    }
                }
            }
            if (!if_node->ElseBlock()) {
                return false;
            }
            return AllPathsReturn(if_node->ElseBlock());
        }
        default:
            return false;
    }
}

bool SpecializationAnalyzer::CollectReturnExps(const SyntaxTreeInterfacePtr &block_node, std::vector<SyntaxTreeInterfacePtr> &ret_exps) const {
    if (!block_node) return false;
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(block_node);
    if (!block || block->Stmts().empty()) {
        return false;
    }
    bool has_return = false;
    for (const auto &stmt: block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::Return) {
            has_return = true;
            const auto ret_node = std::dynamic_pointer_cast<SyntaxTreeReturn>(stmt);
            if (ret_node && ret_node->Explist()) {
                const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(ret_node->Explist());
                if (explist) {
                    for (const auto &exp: explist->Exps()) {
                        ret_exps.push_back(exp);
                    }
                }
            } else {
                ret_exps.push_back(nullptr);
            }
        } else if (stmt->Type() == SyntaxTreeType::Block) {
            if (CollectReturnExps(stmt, ret_exps)) {
                has_return = true;
            }
        } else if (stmt->Type() == SyntaxTreeType::If) {
            const auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);
            if (CollectReturnExps(if_node->Block(), ret_exps)) {
                has_return = true;
            }
            if (if_node->ElseIfs()) {
                const auto el = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(if_node->ElseIfs());
                for (const auto &blk: el->ElseifBlocks()) {
                    if (CollectReturnExps(blk, ret_exps)) {
                        has_return = true;
                    }
                }
            }
            if (if_node->ElseBlock()) {
                if (CollectReturnExps(if_node->ElseBlock(), ret_exps)) {
                    has_return = true;
                }
            }
        }
    }
    return has_return;
}

InferredType SpecializationAnalyzer::ComputeReturnTypeFromSnapshot(const std::unordered_map<const SyntaxTreeInterface *, SSATypeInfo> &snapshot,
                                                                   const FuncRetInfo &ret_info) const {
    if (!ret_info.ends_with_return || ret_info.ret_exps.empty()) {
        return T_DYNAMIC;
    }
    InferredType actual_ret = T_INT;
    for (const auto &ret_exp : ret_info.ret_exps) {
        if (!ret_exp) {
            return T_DYNAMIC;
        }
        const auto inferred = [&]() {
            if (const auto it = snapshot.find(ret_exp.get()); it != snapshot.end()) {
                return it->second.type;
            }
            return T_DYNAMIC;
        }();
        if (inferred == T_FLOAT) {
            if (actual_ret == T_INT) {
                actual_ret = T_FLOAT;
            }
        } else if (inferred != T_INT) {
            return T_DYNAMIC;
        }
    }
    return actual_ret;
}

} // namespace fakelua
