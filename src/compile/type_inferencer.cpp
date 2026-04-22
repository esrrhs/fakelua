#include <ranges>

#include "compile/type_inferencer.h"

#include "compile/syntax_tree.h"
#include "util/common.h"

namespace fakelua {

TypeEnvironment::TypeEnvironment() {
    EnterScope();
}

void TypeEnvironment::EnterScope() {
    scopes_.emplace_back();
}

void TypeEnvironment::ExitScope() {
    if (scopes_.size() > 1) {
        scopes_.pop_back();
    }
}

void TypeEnvironment::Define(const std::string &name, const InferredType type) {
    scopes_.back()[name] = type;
}

bool TypeEnvironment::Update(const std::string &name, const InferredType type) {
    for (auto &scope: std::views::reverse(scopes_)) {
        if (const auto found = scope.find(name); found != scope.end()) {
            found->second = MergeType(found->second, type);
            return true;
        }
    }
    return false;
}

InferredType TypeEnvironment::Lookup(const std::string &name) const {
    for (const auto &scope: std::views::reverse(scopes_)) {
        if (const auto found = scope.find(name); found != scope.end()) {
            return found->second;
        }
    }
    return T_DYNAMIC;
}

InferredType TypeEnvironment::MergeType(const InferredType old_type, const InferredType new_type) {
    if (old_type == T_DYNAMIC || new_type == T_DYNAMIC) {
        return T_DYNAMIC;
    }
    if (old_type == T_UNKNOWN) {
        return new_type;
    }
    if (new_type == T_UNKNOWN) {
        return old_type;
    }
    if (old_type == new_type) {
        return old_type;
    }
    return T_DYNAMIC;
}

void TypeInferencer::Process(CompileResult &cr) {
    WalkSyntaxTree(cr.chunk, [](const SyntaxTreeInterfacePtr &ptr) { ptr->SetEvalType(T_UNKNOWN); });
    InferAndSetEvalType(cr.chunk);

    // 在正常推断之后，通过迭代不动点试推断发现数学参数，写入 cr.math_param_positions。
    DiscoverMathParams(cr);
}

InferredType TypeInferencer::InferAndSetEvalType(const SyntaxTreeInterfacePtr &node) {
    if (!node) {
        return T_UNKNOWN;
    }

    switch (node->Type()) {
        case SyntaxTreeType::Block: {
            const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            // 独立的 do...end 块必须引入自己的作用域，使内部的
            // 局部声明不会污染（或覆盖）外围作用域。
            InferBlock(block, true);
            block->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::LocalVar: {
            const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
            const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(local_var->Namelist());
            std::vector<SyntaxTreeInterfacePtr> exps;
            if (const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(local_var->Explist())) {
                exps = explist->Exps();
            }

            if (!namelist) {
                node->SetEvalType(T_UNKNOWN);
                return T_UNKNOWN;
            }

            const auto &names = namelist->Names();
            for (size_t i = 0; i < names.size(); ++i) {
                InferredType type = T_DYNAMIC;
                if (i < exps.size()) {
                    const auto inferred = InferAndSetEvalType(exps[i]);
                    // 文件级局部变量存储为 static const CVar，而非 int64_t/double，
                    // 因此在环境中必须始终为 T_DYNAMIC，以防止函数将它们
                    // 视为原生类型变量。
                    if (funcbody_depth_ > 0) {
                        type = inferred;
                    }
                }
                env_.Define(names[i], type);
            }

            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::Assign: {
            const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(node);
            const auto varlist = std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist());
            const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(assign->Explist());
            if (!varlist || !explist || explist->Exps().empty() || varlist->Vars().empty()) {
                node->SetEvalType(T_DYNAMIC);
                return T_DYNAMIC;
            }

            DEBUG_ASSERT(varlist->Vars().size() == 1 && explist->Exps().size() == 1);// 预处理阶段已将多赋值拆分成单赋值

            const InferredType rhs_type = InferAndSetEvalType(explist->Exps()[0]);
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(varlist->Vars()[0]);
            if (!var || var->GetType() != "simple") {
                if (var) {
                    var->SetEvalType(T_DYNAMIC);
                }
                node->SetEvalType(T_DYNAMIC);
                return T_DYNAMIC;
            }

            const std::string name = var->GetName();
            if (!env_.Update(name, rhs_type)) {
                var->SetEvalType(T_DYNAMIC);
                node->SetEvalType(T_DYNAMIC);
                return T_DYNAMIC;
            }

            const auto current = env_.Lookup(name);
            var->SetEvalType(current);
            node->SetEvalType(current);
            return current;
        }
        case SyntaxTreeType::ForLoop: {
            const auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
            InferAndSetEvalType(for_loop->ExpBegin());
            InferAndSetEvalType(for_loop->ExpEnd());
            InferAndSetEvalType(for_loop->ExpStep());

            // 仅当所有边界都是 T_INT 时才将循环变量标记为 T_INT，
            // 这与 CGen 使用的整型特化路径相匹配。当所有边界均为数值（T_INT 或 T_FLOAT）
            // 但并非全为 T_INT 时，标记为 T_FLOAT 以启用 double 快路径。
            // 当任何边界为 T_DYNAMIC 时，CGen 会生成 CVar 循环控制变量，
            // 因此循环变量也必须是 T_DYNAMIC 以保持类型一致。
            const bool begin_valid = for_loop->ExpBegin() != nullptr;
            const bool end_valid = for_loop->ExpEnd() != nullptr;
            const bool step_numeric = !for_loop->ExpStep() || for_loop->ExpStep()->EvalType() == T_INT ||
                                      for_loop->ExpStep()->EvalType() == T_FLOAT;
            const bool all_int = begin_valid && end_valid && for_loop->ExpBegin()->EvalType() == T_INT &&
                                 for_loop->ExpEnd()->EvalType() == T_INT &&
                                 (!for_loop->ExpStep() || for_loop->ExpStep()->EvalType() == T_INT);
            const bool all_numeric =
                    !all_int && begin_valid && end_valid &&
                    (for_loop->ExpBegin()->EvalType() == T_INT || for_loop->ExpBegin()->EvalType() == T_FLOAT) &&
                    (for_loop->ExpEnd()->EvalType() == T_INT || for_loop->ExpEnd()->EvalType() == T_FLOAT) && step_numeric;
            const InferredType loop_var_type = all_int ? T_INT : (all_numeric ? T_FLOAT : T_DYNAMIC);

            env_.EnterScope();
            env_.Define(for_loop->Name(), loop_var_type);
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(for_loop->Block()), false);
            const InferredType final_loop_var_type = env_.Lookup(for_loop->Name());
            env_.ExitScope();

            node->SetEvalType(final_loop_var_type);
            return final_loop_var_type;
        }
        case SyntaxTreeType::Function: {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(node);
            InferAndSetEvalType(func->Funcbody());
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::LocalFunction: {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node);
            InferAndSetEvalType(func->Funcbody());
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::FunctionDef: {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(node);
            InferAndSetEvalType(func->Funcbody());
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::FuncBody: {
            const auto funcbody = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(node);
            funcbody_depth_++;
            env_.EnterScope();
            if (const auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(funcbody->Parlist())) {
                if (const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(parlist->Namelist())) {
                    for (const auto &name: namelist->Names()) {
                        env_.Define(name, T_DYNAMIC);
                    }
                }
            }
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(funcbody->Block()), false);
            env_.ExitScope();
            funcbody_depth_--;
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::Return: {
            const auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(node);
            InferAndSetEvalType(ret->Explist());
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::ExpList: {
            const auto exp_list = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
            InferredType last = T_UNKNOWN;
            for (const auto &exp: exp_list->Exps()) {
                last = InferAndSetEvalType(exp);
            }
            node->SetEvalType(last);
            return last;
        }
        case SyntaxTreeType::Exp: {
            const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
            return InferExp(exp);
        }
        case SyntaxTreeType::PrefixExp: {
            const auto prefix_exp = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node);
            return InferPrefixExp(prefix_exp);
        }
        case SyntaxTreeType::Var: {
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(node);
            return InferVar(var);
        }
        case SyntaxTreeType::FunctionCall: {
            const auto functioncall = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
            InferAndSetEvalType(functioncall->prefixexp());
            InferAndSetEvalType(functioncall->Args());
            node->SetEvalType(T_DYNAMIC);
            return T_DYNAMIC;
        }
        case SyntaxTreeType::TableConstructor: {
            const auto tableconstructor = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node);
            InferAndSetEvalType(tableconstructor->Fieldlist());
            node->SetEvalType(T_DYNAMIC);
            return T_DYNAMIC;
        }
        case SyntaxTreeType::Args: {
            const auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node);
            InferAndSetEvalType(args->Explist());
            InferAndSetEvalType(args->Tableconstructor());
            InferAndSetEvalType(args->String());
            node->SetEvalType(T_DYNAMIC);
            return T_DYNAMIC;
        }
        case SyntaxTreeType::FieldList: {
            const auto fieldlist = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
            for (const auto &field: fieldlist->Fields()) {
                InferAndSetEvalType(field);
            }
            node->SetEvalType(T_DYNAMIC);
            return T_DYNAMIC;
        }
        case SyntaxTreeType::Field: {
            const auto field = std::dynamic_pointer_cast<SyntaxTreeField>(node);
            InferAndSetEvalType(field->Key());
            InferAndSetEvalType(field->Value());
            node->SetEvalType(T_DYNAMIC);
            return T_DYNAMIC;
        }
        case SyntaxTreeType::If: {
            const auto if_stmt = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
            InferAndSetEvalType(if_stmt->Exp());
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(if_stmt->Block()), true);
            if (const auto elseifs = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(if_stmt->ElseIfs())) {
                for (size_t i = 0; i < elseifs->ElseifSize(); ++i) {
                    InferAndSetEvalType(elseifs->ElseifExp(i));
                    InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(elseifs->ElseifBlock(i)), true);
                }
            }
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(if_stmt->ElseBlock()), true);
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::While: {
            const auto while_stmt = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
            InferAndSetEvalType(while_stmt->Exp());
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(while_stmt->Block()), true);
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::Repeat: {
            const auto repeat_stmt = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(repeat_stmt->Block()), true);
            InferAndSetEvalType(repeat_stmt->Exp());
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::ForIn: {
            const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
            InferAndSetEvalType(for_in->Namelist());
            InferAndSetEvalType(for_in->Explist());
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(for_in->Block()), true);
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        default: {
            node->SetEvalType(T_DYNAMIC);
            return T_DYNAMIC;
        }
    }
}

InferredType TypeInferencer::InferExp(const std::shared_ptr<SyntaxTreeExp> &exp) {
    const auto &exp_type = exp->ExpType();

    if (exp_type == "number") {
        const auto &value = exp->ExpValue();
        const auto ret = IsInteger(value) ? T_INT : T_FLOAT;
        exp->SetEvalType(ret);
        return ret;
    }
    if (exp_type == "prefixexp") {
        const auto ret = InferAndSetEvalType(exp->Right());
        exp->SetEvalType(ret);
        return ret;
    }
    if (exp_type == "binop") {
        const auto left_type = InferAndSetEvalType(exp->Left());
        const auto right_type = InferAndSetEvalType(exp->Right());

        if (left_type == T_DYNAMIC || right_type == T_DYNAMIC) {
            exp->SetEvalType(T_DYNAMIC);
            return T_DYNAMIC;
        }

        const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(exp->Op());
        if (!op) {
            exp->SetEvalType(T_DYNAMIC);
            return T_DYNAMIC;
        }

        const auto &op_name = op->GetOp();

        // Arithmetic ops that preserve INT+INT=INT, mixed→FLOAT
        if (op_name == "PLUS" || op_name == "MINUS" || op_name == "STAR" || op_name == "DOUBLE_SLASH" || op_name == "MOD") {
            if (left_type == T_INT && right_type == T_INT) {
                exp->SetEvalType(T_INT);
                return T_INT;
            }
            if ((left_type == T_INT || left_type == T_FLOAT) && (right_type == T_INT || right_type == T_FLOAT)) {
                exp->SetEvalType(T_FLOAT);
                return T_FLOAT;
            }
        }

        // Ops that always produce FLOAT
        if (op_name == "SLASH" || op_name == "POW") {
            if ((left_type == T_INT || left_type == T_FLOAT) && (right_type == T_INT || right_type == T_FLOAT)) {
                exp->SetEvalType(T_FLOAT);
                return T_FLOAT;
            }
        }

        // Bitwise ops: always T_INT when both operands are T_INT
        if (op_name == "BITAND" || op_name == "BITOR" || op_name == "XOR" || op_name == "LEFT_SHIFT" || op_name == "RIGHT_SHIFT") {
            if (left_type == T_INT && right_type == T_INT) {
                exp->SetEvalType(T_INT);
                return T_INT;
            }
        }

        exp->SetEvalType(T_DYNAMIC);
        return T_DYNAMIC;
    }

    if (exp_type == "unop") {
        const auto operand_type = InferAndSetEvalType(exp->Right());
        const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(exp->Op());
        if (!op) {
            exp->SetEvalType(T_DYNAMIC);
            return T_DYNAMIC;
        }
        const auto &op_name = op->GetOp();
        if (op_name == "MINUS") {
            if (operand_type == T_INT) {
                exp->SetEvalType(T_INT);
                return T_INT;
            }
            if (operand_type == T_FLOAT) {
                exp->SetEvalType(T_FLOAT);
                return T_FLOAT;
            }
        }
        if (op_name == "BITNOT") {
            if (operand_type == T_INT) {
                exp->SetEvalType(T_INT);
                return T_INT;
            }
        }
        exp->SetEvalType(T_DYNAMIC);
        return T_DYNAMIC;
    }

    exp->SetEvalType(T_DYNAMIC);
    return T_DYNAMIC;
}

InferredType TypeInferencer::InferPrefixExp(const std::shared_ptr<SyntaxTreePrefixexp> &prefix_exp) {
    const auto &prefix_type = prefix_exp->GetType();
    InferredType ret = T_DYNAMIC;

    if (prefix_type == "var" || prefix_type == "exp") {
        ret = InferAndSetEvalType(prefix_exp->GetValue());
    } else if (prefix_type == "functioncall") {
        ret = T_DYNAMIC;
        InferAndSetEvalType(prefix_exp->GetValue());
    }

    prefix_exp->SetEvalType(ret);
    return ret;
}

InferredType TypeInferencer::InferVar(const std::shared_ptr<SyntaxTreeVar> &var) {
    if (var->GetType() == "simple") {
        const auto ret = env_.Lookup(var->GetName());
        var->SetEvalType(ret);
        return ret;
    }

    // 对于"方括号"和"点号"变量，处理子表达式以便内部变量
    //（例如用作表索引的整型循环变量）设置其 EvalType。
    // 如果不这样做，CGen 会在需要 CVar 的地方发出原始 int64_t 变量名。
    if (const auto pe = var->GetPrefixexp()) {
        InferAndSetEvalType(pe);
    }
    if (var->GetType() == "square") {
        if (const auto exp = var->GetExp()) {
            InferAndSetEvalType(exp);
        }
    }

    var->SetEvalType(T_DYNAMIC);
    return T_DYNAMIC;
}

void TypeInferencer::InferBlock(const std::shared_ptr<SyntaxTreeBlock> &block, const bool new_scope) {
    if (!block) {
        return;
    }

    if (new_scope) {
        env_.EnterScope();
    }

    for (const auto &stmt: block->Stmts()) {
        InferAndSetEvalType(stmt);
    }

    // 后处理：变量可能被初始化为类型化表达式（例如整数字面量给出 T_INT），
    // 但随后被涉及函数调用或参数的赋值修改为 T_DYNAMIC。在这种情况下，
    // LocalVar 声明必须使用变量的*最终*类型，以便 C 代码生成器
    // 为所有后续赋值生成兼容的存储声明。
    for (const auto &stmt: block->Stmts()) {
        if (stmt->Type() != SyntaxTreeType::LocalVar) {
            continue;
        }
        const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
        const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(local_var->Namelist());
        if (!namelist) {
            continue;
        }
        std::vector<SyntaxTreeInterfacePtr> exps;
        if (const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(local_var->Explist())) {
            exps = explist->Exps();
        }
        const auto &names = namelist->Names();
        for (size_t i = 0; i < names.size(); ++i) {
            if (i < exps.size()) {
                const auto final_type = env_.Lookup(names[i]);
                exps[i]->SetEvalType(final_type);
            }
        }
    }

    block->SetEvalType(T_UNKNOWN);
    if (new_scope) {
        env_.ExitScope();
    }
}

// ---------------------------------------------------------------------------
// 数学参数特化发现：迭代不动点推断
// ---------------------------------------------------------------------------

// 判断 exp 节点是否为算术二元运算（结果可为 T_INT/T_FLOAT 的运算符）。
bool TypeInferencer::IsArithmeticBinop(const SyntaxTreeInterfacePtr &node) const {
    if (node->Type() != SyntaxTreeType::Exp) {
        return false;
    }
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
    if (exp->ExpType() != "binop") {
        return false;
    }
    const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(exp->Op());
    if (!op) {
        return false;
    }
    const auto &op_name = op->GetOp();
    return op_name == "PLUS" || op_name == "MINUS" || op_name == "STAR" || op_name == "SLASH" ||
           op_name == "DOUBLE_SLASH" || op_name == "POW" || op_name == "MOD" || op_name == "BITAND" ||
           op_name == "XOR" || op_name == "BITOR" || op_name == "LEFT_SHIFT" || op_name == "RIGHT_SHIFT";
}

// 收集函数体中所有出现在赋值语句 LHS 的简单变量名（不含 local 声明）。
// 只遍历当前函数体层，不进入嵌套函数定义。
void TypeInferencer::CollectReassignedVars(const SyntaxTreeInterfacePtr &node,
                                           std::unordered_set<std::string> &reassigned) const {
    if (!node) {
        return;
    }
    switch (node->Type()) {
        case SyntaxTreeType::Block: {
            const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            for (const auto &stmt: block->Stmts()) {
                CollectReassignedVars(stmt, reassigned);
            }
            break;
        }
        // 不进入嵌套函数定义。
        case SyntaxTreeType::Function:
        case SyntaxTreeType::LocalFunction:
        case SyntaxTreeType::FuncBody:
        case SyntaxTreeType::FunctionDef:
            break;
        case SyntaxTreeType::Assign: {
            const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(node);
            if (const auto varlist = assign->Varlist()) {
                const auto vl = std::dynamic_pointer_cast<SyntaxTreeVarlist>(varlist);
                for (const auto &v: vl->Vars()) {
                    if (v->Type() == SyntaxTreeType::Var) {
                        const auto sv = std::dynamic_pointer_cast<SyntaxTreeVar>(v);
                        if (sv->GetType() == "simple") {
                            reassigned.insert(sv->GetName());
                        }
                    }
                }
            }
            if (assign->Explist()) {
                CollectReassignedVars(assign->Explist(), reassigned);
            }
            break;
        }
        case SyntaxTreeType::If: {
            const auto if_stmt = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
            CollectReassignedVars(if_stmt->Block(), reassigned);
            if (const auto elseifs = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(if_stmt->ElseIfs())) {
                for (size_t i = 0; i < elseifs->ElseifSize(); ++i) {
                    CollectReassignedVars(elseifs->ElseifBlock(i), reassigned);
                }
            }
            CollectReassignedVars(if_stmt->ElseBlock(), reassigned);
            break;
        }
        case SyntaxTreeType::While: {
            const auto w = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
            CollectReassignedVars(w->Block(), reassigned);
            break;
        }
        case SyntaxTreeType::Repeat: {
            const auto r = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
            CollectReassignedVars(r->Block(), reassigned);
            break;
        }
        case SyntaxTreeType::ForLoop: {
            const auto fl = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
            CollectReassignedVars(fl->Block(), reassigned);
            break;
        }
        case SyntaxTreeType::ForIn: {
            const auto fi = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
            CollectReassignedVars(fi->Block(), reassigned);
            break;
        }
        default:
            break;
    }
}

TypeInferencer::EvalTypeMap TypeInferencer::RunTrialInference(const SyntaxTreeInterfacePtr &func_block,
                                               const std::vector<std::string> &params,
                                               const std::unordered_map<std::string, InferredType> &assumed_types) {
    // 保存当前推断器状态，trial 结束后恢复。
    TypeEnvironment saved_env = env_;
    const int saved_depth = funcbody_depth_;

    EvalTypeMap prev_map;

    for (int round = 0; round < kMaxSpecIterations; ++round) {
        // 每轮重置函数体内所有节点的 EvalType，保证推断从干净状态开始。
        WalkSyntaxTree(func_block, [](const SyntaxTreeInterfacePtr &n) { n->SetEvalType(T_UNKNOWN); });

        // 以假定的参数类型初始化 trial 环境。
        env_ = TypeEnvironment{};
        funcbody_depth_ = 1;
        for (const auto &p: params) {
            const auto it = assumed_types.find(p);
            env_.Define(p, it != assumed_types.end() ? it->second : T_DYNAMIC);
        }

        // 运行函数体类型推断（不新开作用域，参数已在当前作用域中定义）。
        InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(func_block), false);

        // 快照本轮推断结果。
        EvalTypeMap curr_map;
        WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &n) { curr_map[n.get()] = n->EvalType(); });

        if (curr_map == prev_map) {
            // 已达到不动点，提前退出。
            break;
        }
        prev_map = std::move(curr_map);
    }

    // 恢复推断器状态（EvalType 保持本轮 trial 推断结果，由调用方负责恢复）。
    env_ = std::move(saved_env);
    funcbody_depth_ = saved_depth;

    return prev_map;
}

bool TypeInferencer::HasArithmeticImprovement(const EvalTypeMap &all_int, const EvalTypeMap &baseline,
                                               const SyntaxTreeInterfacePtr &func_block) const {
    bool found = false;
    WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
        if (found || !IsArithmeticBinop(node)) {
            return;
        }
        const auto it_all = all_int.find(node.get());
        const auto it_base = baseline.find(node.get());
        if (it_all == all_int.end() || it_base == baseline.end()) {
            return;
        }
        // 全参数为 T_INT 时，该算术节点相对 baseline（全 T_DYNAMIC）变为有类型。
        if ((it_all->second == T_INT || it_all->second == T_FLOAT) && it_base->second == T_DYNAMIC) {
            found = true;
        }
    });
    return found;
}

bool TypeInferencer::ParamAffectsArithmetic(const EvalTypeMap &all_int, const EvalTypeMap &without_p,
                                             const SyntaxTreeInterfacePtr &func_block) const {
    bool found = false;
    WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
        if (found || !IsArithmeticBinop(node)) {
            return;
        }
        const auto it_all = all_int.find(node.get());
        const auto it_wo = without_p.find(node.get());
        if (it_all == all_int.end() || it_wo == without_p.end()) {
            return;
        }
        // all_int 中有类型，但去掉该参数（without_p）后退化：说明此参数参与了算术推断。
        if ((it_all->second == T_INT || it_all->second == T_FLOAT) && it_wo->second != it_all->second) {
            found = true;
        }
    });
    return found;
}

void TypeInferencer::DiscoverMathParams(CompileResult &cr) {
    const auto chunk = cr.chunk;
    if (!chunk || chunk->Type() != SyntaxTreeType::Block) {
        return;
    }

    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
    for (const auto &stmt: top_block->Stmts()) {
        // 解析顶层函数名和函数体。
        std::string name;
        SyntaxTreeInterfacePtr funcbody;

        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            const auto funcname = std::dynamic_pointer_cast<SyntaxTreeFuncname>(func->Funcname());
            // PreProcessor 已保证 funcname 合法（简单单段名，无冒号）
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
        const auto parlist_node = funcbody_ptr->Parlist();
        if (!parlist_node) {
            continue;
        }
        const auto parlist_ptr = std::dynamic_pointer_cast<SyntaxTreeParlist>(parlist_node);
        // PreProcessor 已保证不存在变长参数函数
        const auto namelist_node = parlist_ptr->Namelist();
        if (!namelist_node) {
            continue;
        }
        const auto params = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist_node)->Names();
        if (params.empty()) {
            continue;
        }

        const auto func_block = funcbody_ptr->Block();

        // 快照当前正常推断产生的 EvalType，trial 结束后恢复。
        std::vector<std::pair<SyntaxTreeInterface *, InferredType>> original_snapshot;
        WalkSyntaxTree(func_block,
                       [&](const SyntaxTreeInterfacePtr &n) { original_snapshot.emplace_back(n.get(), n->EvalType()); });

        // 构建参数类型假设映射：将 all_params 中所有参数设为 default_type，
        // 然后将 special_param（若非空）覆盖为 special_type。
        const auto make_assumed = [&](const std::string &special_param, InferredType special_type,
                                      InferredType default_type) {
            std::unordered_map<std::string, InferredType> m;
            for (const auto &p: params) {
                m[p] = (!special_param.empty() && p == special_param) ? special_type : default_type;
            }
            return m;
        };

        // 基准推断：所有参数 = T_DYNAMIC（与正常推断一致）。
        const auto baseline = RunTrialInference(func_block, params, make_assumed("", T_DYNAMIC, T_DYNAMIC));

        // 全参数为 T_INT 的推断，用于快速检验是否存在可被特化的算术表达式。
        const auto all_int = RunTrialInference(func_block, params, make_assumed("", T_INT, T_INT));

        // 若全参数 T_INT 相对 baseline 没有算术改善，则跳过此函数。
        if (!HasArithmeticImprovement(all_int, baseline, func_block)) {
            for (const auto &[ptr, type]: original_snapshot) {
                ptr->SetEvalType(type);
            }
            continue;
        }

        // leave-one-out 检验：逐一将某参数置回 T_DYNAMIC（其余仍为 T_INT），
        // 若出现算术退化则该参数为数学参数。
        // 注意：被函数体重新赋值的参数（如 n = n + 1）不能特化，
        // 否则 CGen 在特化签名（int64_t n）中会产生 CVar=int64_t 的类型错误。
        std::unordered_set<std::string> reassigned_params;
        CollectReassignedVars(func_block, reassigned_params);

        std::vector<int> math_indices;
        for (int i = 0; i < static_cast<int>(params.size()); ++i) {
            // 被重新赋值的参数跳过：特化 CGen 无法安全处理对原生类型参数的 CVar 回写。
            if (reassigned_params.count(params[i])) {
                continue;
            }
            // without_p：params[i] = T_DYNAMIC，其余 = T_INT。
            const auto without_p_assumed = make_assumed(params[i], T_DYNAMIC, T_INT);
            const auto without_p_map = RunTrialInference(func_block, params, without_p_assumed);
            if (ParamAffectsArithmetic(all_int, without_p_map, func_block)) {
                math_indices.push_back(i);
            }
        }

        // 恢复正常推断的 EvalType。
        for (const auto &[ptr, type]: original_snapshot) {
            ptr->SetEvalType(type);
        }

        if (!math_indices.empty()) {
            cr.math_param_positions[name] = math_indices;
            LOG_INFO("TypeInferencer: {} math params for {}", math_indices.size(), name);

            // 为该函数的每个 bitmask 组合运行一次 trial 推断并存储快照，
            // 使 CGen 能够在生成特化体时直接查询每个 AST 节点的正确类型，
            // 而无需在代码生成阶段重新推断。
            const int num_specs = 1 << static_cast<int>(math_indices.size());
            auto &snapshots = cr.specialization_snapshots[name];
            snapshots.resize(num_specs);
            for (int bitmask = 0; bitmask < num_specs; ++bitmask) {
                std::unordered_map<std::string, InferredType> assumed;
                for (const auto &p: params) {
                    assumed[p] = T_DYNAMIC; // 非数学参数默认 T_DYNAMIC
                }
                for (int i = 0; i < static_cast<int>(math_indices.size()); ++i) {
                    assumed[params[static_cast<size_t>(math_indices[i])]] =
                            (MathParamKindOf(bitmask, i) == kMathParamFloat) ? T_FLOAT : T_INT;
                }
                snapshots[bitmask] = RunTrialInference(func_block, params, assumed);
            }
        }
    }
}

}// namespace fakelua
