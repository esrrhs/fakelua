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

/**
 * @brief 检测特定语法树节点在两组推导映射（快照）中的类型变化是否属于“数值优化”或“非等价改变”
 * 
 * @param node 被检测的 AST 语法树节点
 * @param typed_map 新试探推导产生的类型映射快照 (Snapshot)
 * @param compare_map 基准类型映射快照 (通常为形参设为 T_DYNAMIC 时的推导快照)
 * @param improvement_mode 是否处于“类型提升检测模式”
 *        - 若为 true：检测节点是否从 compare_map 中的 T_DYNAMIC 提升为 typed_map 中的数值类型（T_INT / T_FLOAT）
 *        - 若为 false：检测节点在两个快照中的类型是否产生了非等价变动（例如从 T_INT 变成了 T_FLOAT，或反之）
 * @return 如果符合检测条件，返回 true；否则返回 false
 */
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

// ── SpecializationAnalyzer 实现 ───────────────────────────────────────────

void SpecializationAnalyzer::Analyze(const ParseResult &pr, InferResult &ir) {
    // 第一步：分析并识别所有的数学特化参数
    MathFuncInfoMap math_func_info = IdentifyMathParams(pr, ir);

    // 缓存所有目标特化函数的 return 语句返回值信息
    std::unordered_map<std::string, FuncRetInfo> func_ret_cache;
    for (const auto &[func_name, info] : math_func_info) {
        FuncRetInfo ret_info;
        ret_info.ends_with_return = CollectReturnExps(info.block, ret_info.ret_exps);
        func_ret_cache[func_name] = std::move(ret_info);
    }

    // 第二步：执行 2^k 次方穷举的特化推导，推导各特化分支下的返回值类型并填充 JIT 语法树节点快照
    InferSpecializationReturnTypes(ir, math_func_info, func_ret_cache);
}

SpecializationAnalyzer::MathFuncInfoMap SpecializationAnalyzer::IdentifyMathParams(const ParseResult &pr, InferResult &ir) {
    MathFuncInfoMap math_func_info;
    if (!pr.chunk || pr.chunk->Type() != SyntaxTreeType::Block) {
        return math_func_info;
    }

    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(pr.chunk);

    // 1. 遍历顶层语法树，筛选出所有普通函数及局部函数声明
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

        // 保存分析此函数特化所需要的所有必要 AST 和 SSA 数据
        MathFuncInfo info;
        info.block = funcbody_ptr->Block();
        info.params = params;
        info.param_versions = ssa_it->second->param_versions;

        math_func_info[name] = info;
    }

    // 2. 迭代寻找数学参数。使用迭代轮次（pass）是为了解决函数间互相嵌套依赖特化的场景下，
    //    形参特化优化在调用链条中级联传播的收敛问题（即 fixed point 收敛）。
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

            // 运行两组极限推导作为基准比对：
            // Baseline: 所有的形参全部假设为 T_DYNAMIC
            const auto baseline = RunTrialInference(name, info.block, *cfg_it->second, *ssa_it->second,
                                                   MakeAssumedParamTypes(info.param_versions, -1, T_DYNAMIC, T_DYNAMIC), ir);
            // All_Int: 所有的形参全部积极地假设为 T_INT
            const auto all_int = RunTrialInference(name, info.block, *cfg_it->second, *ssa_it->second,
                                                  MakeAssumedParamTypes(info.param_versions, -1, T_INT, T_INT), ir);

            // 通过交叉比较 baseline 与 all_int 确定哪些参数的特化会触发函数体内节点类型的有效提升
            const auto math_indices = FindMathParamIndices(name, info, baseline, all_int, ir.math_param_positions, ir);

            if (math_indices.empty()) {
                continue;
            }
            if (math_indices.size() > kMaxMathSpecializedParams) {
                // 如果发现的特化参数过多，为了避免 2^k 次方的分支数量膨胀，在此处实施安全阈值剪枝
                continue;
            }

            ir.math_param_positions[name] = math_indices;
            new_discovery = true;
        }
        if (!new_discovery) {
            break; // 推导收敛，无新参数发现，退出迭代
        }
    }

    // 过滤出真正具有数学特化参数的函数并返回
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
    // 如果假设全部参数为 T_INT 后，整个函数体连一个运算节点都没被优化为数值类型，则直接说明该函数无特化价值
    if (!CheckArithmeticTypeChanges(all_int, baseline, info.block, true, known_math_positions)) {
        return math_indices;
    }

    auto cfg_it = ir.cfg_functions.find(name);
    auto ssa_it = ir.ssa_functions.find(name);
    if (cfg_it == ir.cfg_functions.end() || ssa_it == ir.ssa_functions.end()) {
        return math_indices;
    }

    // 逐个参数排除，确定导致优化的真正因果位置：
    // 若把第 i 个参数从 T_INT 退化为 T_DYNAMIC 之后，整个函数体丢失了原有的特化提升类型，
    // 则说明第 i 个参数对于特化是必须的，应将其判定为数学参数。
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
    // 1. 初始化每个函数下所有 2^k 次方的特化槽位
    for (const auto &[func_name, info] : math_func_info) {
        const auto &math_params = ir.math_param_positions.at(func_name);
        ir.specialization_return_types[func_name].assign(static_cast<size_t>(1 << static_cast<int>(math_params.size())), T_INT);
        ir.specialization_snapshots[func_name].resize(static_cast<size_t>(1 << static_cast<int>(math_params.size())));
    }

    // 2. 迭代求取各特化组合的推导类型收敛（解决互相递归调用的特化类型传播收敛问题）
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

            // 穷举特化组合：第 i 个比特表示第 i 个数学特化形参假设为 T_FLOAT (1) 还是 T_INT (0)
            for (int bitmask = 0; bitmask < num_specs; ++bitmask) {
                // 构建初始假设并运行一次单参数组合的试探推导
                auto assumed = MakeSpecializedParamTypes(info.param_versions, math_indices, bitmask);
                auto new_snapshot = RunTrialInference(func_name, info.block, *cfg_it->second, *ssa_it->second, assumed, ir);

                // 基于得到的 AST 类型快照计算当前特化版本对应的合并返回值类型
                const auto new_ret = ComputeReturnTypeFromSnapshot(new_snapshot, ret_info);

                const auto bitmask_sz = static_cast<size_t>(bitmask);
                auto &cur_ret = ir.specialization_return_types[func_name][bitmask_sz];
                auto &cur_snap = ir.specialization_snapshots[func_name][bitmask_sz];

                // 检测是否与旧的特化类型或快照类型映射存在变动。若有，触发更优类型合并，设置 changed 以启动新一轮迭代传播。
                if (new_ret != cur_ret || new_snapshot != cur_snap) {
                    cur_ret = new_ret;
                    cur_snap = std::move(new_snapshot);
                    changed = true;
                }
            }
        }
        if (!changed) {
            break; // 相互依赖调用的特化类型已完全收敛，退出迭代
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
        // 如果满足以下任何一个节点类型的优化提升或特征变动：
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

    // 比较表达式本身的推导类型永远是 T_BOOL。因此我们检测其两个操作数（Left 和 Right）的推导类型是否发生提升/变动。
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

    // 检查递归特化调用是否发生了变化：
    // 被调用者也是数学特化函数时，若我们传入给被调用者“数学特化参数槽”处的参数类型在两张表中不一样，
    // 则说明第 i 个参数的假设直接决定了下游调用特化链的成功与否，因此当前形参应该判定为数学参数。
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

    // 创建局部临时的推导环境，隔离本次试探推断的中间类型变动，防止污染全局主要推断上下文
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
        return T_DYNAMIC; // 存在任何不确定 return 的流分支，退化为非类型特化
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
                actual_ret = T_FLOAT; // 如果存在任何一个返回表达式是浮点数，返回值类型提升/升级为 T_FLOAT
            }
        } else if (inferred != T_INT) {
            return T_DYNAMIC; // 存在非数值类型的返回，退化为非数值特化 (T_DYNAMIC)
        }
    }
    return actual_ret;
}

} // namespace fakelua
