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
            DEBUG_ASSERT(varlist && explist && !varlist->Vars().empty() && !explist->Exps().empty());

            DEBUG_ASSERT(varlist->Vars().size() == 1 && explist->Exps().size() == 1);// 预处理阶段已将多赋值拆分成单赋值

            const InferredType rhs_type = InferAndSetEvalType(explist->Exps()[0]);
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(varlist->Vars()[0]);
            DEBUG_ASSERT(var && var->GetType() == "simple");

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
        DEBUG_ASSERT(op);

        const auto &op_name = op->GetOp();

        // 保持 INT+INT=INT、混合→FLOAT 语义的算术运算
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

        // 结果始终为 FLOAT 的运算
        if (op_name == "SLASH" || op_name == "POW") {
            if ((left_type == T_INT || left_type == T_FLOAT) && (right_type == T_INT || right_type == T_FLOAT)) {
                exp->SetEvalType(T_FLOAT);
                return T_FLOAT;
            }
        }

        // 位运算：两个操作数均为 T_INT 时结果为 T_INT
        if (op_name == "BITAND" || op_name == "BITOR" || op_name == "XOR" || op_name == "LEFT_SHIFT" || op_name == "RIGHT_SHIFT") {
            if (left_type == T_INT && right_type == T_INT) {
                exp->SetEvalType(T_INT);
                return T_INT;
            }
        }

        // AND/OR：Lua 中整数和浮点数始终为真值（包括 0），因此：
        //   a and b（a 为 T_INT/T_FLOAT）：a 始终为真，结果为 b → 类型为 right_type
        //   a or  b（a 为 T_INT/T_FLOAT）：a 始终为真，结果为 a → 类型为 left_type
        if (op_name == "AND") {
            if ((left_type == T_INT || left_type == T_FLOAT) && (right_type == T_INT || right_type == T_FLOAT)) {
                exp->SetEvalType(right_type);
                return right_type;
            }
        }
        if (op_name == "OR") {
            if ((left_type == T_INT || left_type == T_FLOAT) && (right_type == T_INT || right_type == T_FLOAT)) {
                exp->SetEvalType(left_type);
                return left_type;
            }
        }

        exp->SetEvalType(T_DYNAMIC);
        return T_DYNAMIC;
    }

    if (exp_type == "unop") {
        const auto operand_type = InferAndSetEvalType(exp->Right());
        const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(exp->Op());
        DEBUG_ASSERT(op);
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
        if (op_name == "NUMBER_SIGN") {
            // # 运算符始终返回整数（字符串字节数或表元素数）。
            // 无论操作数是字符串还是表（均为 T_DYNAMIC），结果类型始终为 T_INT。
            exp->SetEvalType(T_INT);
            return T_INT;
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

// 判断 exp 节点是否为算术表达式（结果可为 T_INT/T_FLOAT 的运算符）。
// 包括算术/位运算二元运算符，以及一元负号（MINUS）和按位取反（BITNOT）。
bool TypeInferencer::IsArithmeticExpr(const SyntaxTreeInterfacePtr &node) const {
    if (node->Type() != SyntaxTreeType::Exp) {
        return false;
    }
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
    if (exp->ExpType() == "binop") {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(exp->Op());
        if (!op) {
            return false;
        }
        const auto &op_name = op->GetOp();
        return op_name == "PLUS" || op_name == "MINUS" || op_name == "STAR" || op_name == "SLASH" ||
               op_name == "DOUBLE_SLASH" || op_name == "POW" || op_name == "MOD" || op_name == "BITAND" ||
               op_name == "XOR" || op_name == "BITOR" || op_name == "LEFT_SHIFT" || op_name == "RIGHT_SHIFT";
    }
    if (exp->ExpType() == "unop") {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(exp->Op());
        if (!op) {
            return false;
        }
        const auto &op_name = op->GetOp();
        // 一元负号：-T_INT=T_INT，-T_FLOAT=T_FLOAT，随参数类型改变。
        // 按位取反：~T_INT=T_INT，仅对整数参数有意义。
        return op_name == "MINUS" || op_name == "BITNOT";
    }
    return false;
}

// 判断 exp 节点是否为有序比较运算符（<、<=、>、>=）。
// 这类运算符在 Lua 中用于数值排序，当两侧操作数均为数值类型时，
// CGen 可通过 TryCompileNativeBoolExpr 直接生成原生 C 比较，无需 OpXxx 宏 + IsTrue 调用。
// 注意：EQUAL（==）和 NOT_EQUAL（~=）故意排除在外——Lua 的等价性比较对任意类型均有意义
//（字符串、布尔值、表、nil 均可比较），将其用作数学参数发现条件会错误地要求参数为数值类型。
bool TypeInferencer::IsNativeComparisonExpr(const SyntaxTreeInterfacePtr &node) const {
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
    return op_name == "LESS" || op_name == "LESS_EQUAL" || op_name == "MORE" ||
           op_name == "MORE_EQUAL";
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
                                               const SyntaxTreeInterfacePtr &func_block,
                                               const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const {
    if (HasArithmeticNodeTypeChange(all_int, baseline, func_block, true)) {
        return true;
    }
    if (HasComparisonOperandTypeChange(all_int, baseline, func_block, true)) {
        return true;
    }
    if (HasForLoopTypeChange(all_int, baseline, func_block, true)) {
        return true;
    }
    return HasMathCallImprovement(func_block, all_int, baseline, math_param_positions);
}

bool TypeInferencer::ParamAffectsArithmetic(const EvalTypeMap &all_int, const EvalTypeMap &without_p,
                                              const SyntaxTreeInterfacePtr &func_block,
                                              const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const {
    if (HasArithmeticNodeTypeChange(all_int, without_p, func_block, false)) {
        return true;
    }
    if (HasComparisonOperandTypeChange(all_int, without_p, func_block, true)) {
        return true;
    }
    if (HasForLoopTypeChange(all_int, without_p, func_block, true)) {
        return true;
    }
    return HasMathCallImprovement(func_block, all_int, without_p, math_param_positions);
}

bool TypeInferencer::HasMathCallImprovement(
        const SyntaxTreeInterfacePtr &func_block,
        const EvalTypeMap &typed_map,
        const EvalTypeMap &compare_map,
        const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const {
    bool found = false;
    WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
        if (found || node->Type() != SyntaxTreeType::FunctionCall) {
            return;
        }
        const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
        if (!fc) {
            return;
        }
        const auto callee_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
        if (!callee_pe || callee_pe->GetType() != "var") {
            return;
        }
        const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(callee_pe->GetValue());
        if (!callee_var || callee_var->GetType() != "simple") {
            return;
        }
        const auto &callee_name = callee_var->GetName();
        const auto math_it = math_param_positions.find(callee_name);
        if (math_it == math_param_positions.end()) {
            return;
        }
        const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
        if (!args_ptr) {
            return;
        }
        const auto raw_args = ExtractCallRawArgs(args_ptr);
        if (raw_args.empty()) {
            return;
        }
        for (const int param_pos : math_it->second) {
            if (param_pos >= static_cast<int>(raw_args.size())) {
                return;
            }
            const auto &arg = raw_args[static_cast<size_t>(param_pos)];
            const auto it_typed = typed_map.find(arg.get());
            const auto it_comp = compare_map.find(arg.get());
            if (it_typed == typed_map.end() || it_comp == compare_map.end()) {
                continue;
            }
            // typed_map 中该实参有类型但 compare_map 中没有：说明存在改善/退化。
            if ((it_typed->second == T_INT || it_typed->second == T_FLOAT) &&
                it_comp->second != it_typed->second) {
                found = true;
                return;
            }
        }
    });
    return found;
}

bool TypeInferencer::HasArithmeticNodeTypeChange(const EvalTypeMap &typed_map,
                                                 const EvalTypeMap &compare_map,
                                                 const SyntaxTreeInterfacePtr &func_block,
                                                 const bool require_compare_dynamic) const {
    bool found = false;
    WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
        if (found || !IsArithmeticExpr(node)) {
            return;
        }
        const auto it_typed = typed_map.find(node.get());
        const auto it_compare = compare_map.find(node.get());
        if (it_typed == typed_map.end() || it_compare == compare_map.end()) {
            return;
        }
        if (!IsNumericInferredType(it_typed->second)) {
            return;
        }
        if (require_compare_dynamic) {
            if (it_compare->second == T_DYNAMIC) {
                found = true;
            }
            return;
        }
        if (it_compare->second != it_typed->second) {
            found = true;
        }
    });
    return found;
}

bool TypeInferencer::HasComparisonOperandTypeChange(const EvalTypeMap &typed_map,
                                                    const EvalTypeMap &compare_map,
                                                    const SyntaxTreeInterfacePtr &func_block,
                                                    const bool require_compare_dynamic) const {
    bool found = false;
    WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
        if (found || !IsNativeComparisonExpr(node)) {
            return;
        }
        const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
        const auto left = exp->Left();
        const auto right = exp->Right();
        if (!left || !right) {
            return;
        }
        const auto lt_typed = typed_map.find(left.get());
        const auto rt_typed = typed_map.find(right.get());
        const auto lt_compare = compare_map.find(left.get());
        const auto rt_compare = compare_map.find(right.get());
        if (lt_typed == typed_map.end() || rt_typed == typed_map.end() ||
            lt_compare == compare_map.end() || rt_compare == compare_map.end()) {
            return;
        }
        const bool both_typed = IsNumericInferredType(lt_typed->second) &&
                                IsNumericInferredType(rt_typed->second);
        if (!both_typed) {
            return;
        }
        if (require_compare_dynamic) {
            if (lt_compare->second == T_DYNAMIC || rt_compare->second == T_DYNAMIC) {
                found = true;
            }
            return;
        }
        if (lt_compare->second != lt_typed->second || rt_compare->second != rt_typed->second) {
            found = true;
        }
    });
    return found;
}

bool TypeInferencer::HasForLoopTypeChange(const EvalTypeMap &typed_map,
                                          const EvalTypeMap &compare_map,
                                          const SyntaxTreeInterfacePtr &func_block,
                                          const bool require_compare_dynamic) const {
    bool found = false;
    WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
        if (found || node->Type() != SyntaxTreeType::ForLoop) {
            return;
        }
        const auto it_typed = typed_map.find(node.get());
        const auto it_compare = compare_map.find(node.get());
        if (it_typed == typed_map.end() || it_compare == compare_map.end()) {
            return;
        }
        if (!IsNumericInferredType(it_typed->second)) {
            return;
        }
        if (require_compare_dynamic) {
            if (it_compare->second == T_DYNAMIC) {
                found = true;
            }
            return;
        }
        if (it_compare->second != it_typed->second) {
            found = true;
        }
    });
    return found;
}

// 检查 block_node 的所有执行路径是否均以 return 语句结束。
// 与简单的"最后一条语句是 return"不同，此函数还能识别 if-elseif-else 结构
// 中所有分支均返回的情况（如 fibonacci），从而正确标记所有路径均返回数值。
// 不递归进入嵌套函数体（Function / LocalFunction）。
bool TypeInferencer::AllPathsReturn(const SyntaxTreeInterfacePtr &block_node) const {
    if (!block_node) {
        return false;
    }
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(block_node);
    if (!block || block->Stmts().empty()) {
        return false;
    }
    const auto &last = block->Stmts().back();
    if (last->Type() == SyntaxTreeType::Return) {
        return true;
    }
    if (last->Type() == SyntaxTreeType::Block) {
        // do...end 块：无条件执行（不同于 if/while 可能跳过），内部可含任意控制流。
        // 若其所有路径均以 return 结束，则外层函数视为已返回。
        return AllPathsReturn(last);
    }
    if (last->Type() == SyntaxTreeType::If) {
        const auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(last);
        // if 分支必须返回。
        if (!AllPathsReturn(if_node->Block())) {
            return false;
        }
        // 所有 elseif 分支必须返回。
        if (const auto elseifs = if_node->ElseIfs()) {
            const auto el = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseifs);
            for (const auto &blk : el->ElseifBlocks()) {
                if (!AllPathsReturn(blk)) {
                    return false;
                }
            }
        }
        // 必须有 else 分支且它也返回，否则无法保证所有路径均返回。
        if (!if_node->ElseBlock()) {
            return false;
        }
        return AllPathsReturn(if_node->ElseBlock());
    }
    return false;
}

// 从 block_node 中浅层收集每条顶层 return 语句的第一个返回表达式，
// 不递归进入嵌套函数体（Function / LocalFunction 节点）。
// 返回值为 true 表示该 block 的所有路径均以 return 结束（无 nil 隐式返回路径）。
// 对于 nil 返回（无表达式或空列表），在 ret_exps 中压入 nullptr 作为占位符。
bool TypeInferencer::CollectReturnExps(const SyntaxTreeInterfacePtr &block_node,
                               std::vector<SyntaxTreeInterfacePtr> &ret_exps) const {
    if (!block_node) {
        return false;
    }
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(block_node);
    if (!block) {
        return false;
    }
    const auto &stmts = block->Stmts();
    if (stmts.empty()) {
        return false;
    }
    const bool ends_with_return = AllPathsReturn(block_node);
    for (const auto &stmt : stmts) {
        if (stmt->Type() == SyntaxTreeType::Return) {
            const auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(stmt);
            const auto explist = ret->Explist();
            if (explist) {
                const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
                if (el && !el->Exps().empty()) {
                    ret_exps.push_back(el->Exps()[0]);
                    continue;
                }
            }
            ret_exps.push_back(nullptr);
        } else if (stmt->Type() == SyntaxTreeType::If) {
            const auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);
            CollectReturnExps(if_node->Block(), ret_exps);
            if (const auto elseifs = if_node->ElseIfs()) {
                const auto el = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseifs);
                for (const auto &blk : el->ElseifBlocks()) {
                    CollectReturnExps(blk, ret_exps);
                }
            }
            CollectReturnExps(if_node->ElseBlock(), ret_exps);
        } else if (stmt->Type() == SyntaxTreeType::While) {
            const auto while_node = std::dynamic_pointer_cast<SyntaxTreeWhile>(stmt);
            CollectReturnExps(while_node->Block(), ret_exps);
        } else if (stmt->Type() == SyntaxTreeType::Repeat) {
            const auto rep = std::dynamic_pointer_cast<SyntaxTreeRepeat>(stmt);
            CollectReturnExps(rep->Block(), ret_exps);
        } else if (stmt->Type() == SyntaxTreeType::ForLoop) {
            const auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt);
            CollectReturnExps(for_loop->Block(), ret_exps);
        } else if (stmt->Type() == SyntaxTreeType::ForIn) {
            const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt);
            CollectReturnExps(for_in->Block(), ret_exps);
        } else if (stmt->Type() == SyntaxTreeType::Block) {
            // do...end 块：递归收集其内部的 return 表达式。
            CollectReturnExps(stmt, ret_exps);
        }
        // Function / LocalFunction: 不递归进入嵌套函数体。
    }
    return ends_with_return;
}

// 在给定的特化上下文（spec_ctx：数学参数名 → 类型）和假定返回类型表（assumed_ret）下，
// 递归计算返回表达式 exp 的类型。
// 对于指向数学参数函数的调用节点，使用 assumed_ret 中的假定返回类型，
// 从而支持不动点迭代中的循环/递归推断。
InferredType TypeInferencer::EvalReturnExpType(
        const SyntaxTreeInterfacePtr &exp,
        const EvalTypeSnapshot &snapshot,
        const std::unordered_map<std::string, InferredType> &spec_ctx,
        const std::unordered_map<std::string, std::vector<int>> &math_param_positions,
        const std::unordered_map<std::string, std::vector<InferredType>> &assumed_ret) const {
    if (!exp || exp->Type() != SyntaxTreeType::Exp) {
        return T_DYNAMIC;
    }
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto &exp_type = e->ExpType();

    if (exp_type == "number") {
        const auto it = snapshot.find(exp.get());
        if (it != snapshot.end() && (it->second == T_INT || it->second == T_FLOAT)) {
            return it->second;
        }
        return T_DYNAMIC;
    }

    if (exp_type == "prefixexp") {
        const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e->Right());
        if (!pe) {
            return T_DYNAMIC;
        }
        if (pe->GetType() == "var") {
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue());
            if (!var || var->GetType() != "simple") {
                return T_DYNAMIC;
            }
            const auto &vname = var->GetName();
            const auto ctx_it = spec_ctx.find(vname);
            if (ctx_it != spec_ctx.end()) {
                return ctx_it->second;
            }
            const auto snap_it = snapshot.find(exp.get());
            if (snap_it != snapshot.end() && (snap_it->second == T_INT || snap_it->second == T_FLOAT)) {
                return snap_it->second;
            }
            return T_DYNAMIC;
        }
        if (pe->GetType() == "exp") {
            return EvalReturnExpType(pe->GetValue(), snapshot, spec_ctx, math_param_positions, assumed_ret);
        }
        if (pe->GetType() == "functioncall") {
            const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(pe->GetValue());
            if (!fc) {
                return T_DYNAMIC;
            }
            const auto callee_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
            if (!callee_pe || callee_pe->GetType() != "var") {
                return T_DYNAMIC;
            }
            const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(callee_pe->GetValue());
            if (!callee_var || callee_var->GetType() != "simple") {
                return T_DYNAMIC;
            }
            const auto &callee_name = callee_var->GetName();
            const auto math_it = math_param_positions.find(callee_name);
            if (math_it == math_param_positions.end()) {
                return T_DYNAMIC;
            }
            const auto &math_params = math_it->second;
            const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
            if (!args_ptr) {
                return T_DYNAMIC;
            }
            const auto raw_args = ExtractCallRawArgs(args_ptr);
            if (raw_args.empty()) {
                return T_DYNAMIC;
            }
            int bitmask = 0;
            for (int i = 0; i < static_cast<int>(math_params.size()); ++i) {
                const int param_pos = math_params[i];
                if (param_pos >= static_cast<int>(raw_args.size())) {
                    return T_DYNAMIC;
                }
                const auto &arg = raw_args[param_pos];
                if (!arg || arg->Type() != SyntaxTreeType::Exp) {
                    return T_DYNAMIC;
                }
                const auto t = EvalReturnExpType(arg, snapshot, spec_ctx, math_param_positions, assumed_ret);
                if (t == T_DYNAMIC) {
                    return T_DYNAMIC;
                }
                if (t == T_FLOAT) {
                    bitmask |= (1 << i);
                }
            }
            const auto ret_it = assumed_ret.find(callee_name);
            if (ret_it == assumed_ret.end()) {
                return T_DYNAMIC;
            }
            const auto &ret_types = ret_it->second;
            if (bitmask >= static_cast<int>(ret_types.size())) {
                return T_DYNAMIC;
            }
            return ret_types[static_cast<size_t>(bitmask)];
        }
        return T_DYNAMIC;
    }

    if (exp_type == "binop") {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(e->Op());
        if (!op) {
            return T_DYNAMIC;
        }
        const auto left = EvalReturnExpType(e->Left(), snapshot, spec_ctx, math_param_positions, assumed_ret);
        const auto right = EvalReturnExpType(e->Right(), snapshot, spec_ctx, math_param_positions, assumed_ret);
        return InferNumericBinopResultType(op->GetOp(), left, right);
    }

    if (exp_type == "unop") {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(e->Op());
        if (!op) {
            return T_DYNAMIC;
        }
        if (op->GetOp() == "MINUS") {
            return EvalReturnExpType(e->Right(), snapshot, spec_ctx, math_param_positions, assumed_ret);
        }
        if (op->GetOp() == "BITNOT") {
            const auto t = EvalReturnExpType(e->Right(), snapshot, spec_ctx, math_param_positions, assumed_ret);
            if (t == T_INT) {
                return T_INT;
            }
        }
        if (op->GetOp() == "NUMBER_SIGN") {
            // # 运算符始终返回整数（字符串字节数或表元素数），与操作数类型无关。
            return T_INT;
        }
        return T_DYNAMIC;
    }

    return T_DYNAMIC;
}

// 扫描函数块顶层的 local 声明，将由数学函数调用（或其他能推断出数值类型的表达式）
// 初始化的局部变量的类型追加到 spec_ctx 中。处理按声明顺序进行（单遍），
// 从而支持链式传播（如 local y = x + 1，其中 x 已由前面的 local x = f(n) 加入 spec_ctx）。
// 仅处理顶层 LocalVar 语句，不递归进入嵌套函数体（Function / LocalFunction）。
void TypeInferencer::BuildLocalVarExtensions(
        const SyntaxTreeInterfacePtr &func_block,
        const EvalTypeSnapshot &snapshot,
        std::unordered_map<std::string, InferredType> &spec_ctx,
        const std::unordered_map<std::string, std::vector<int>> &math_param_positions,
        const std::unordered_map<std::string, std::vector<InferredType>> &assumed_ret) const {
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(func_block);
    if (!block) {
        return;
    }
    for (const auto &stmt : block->Stmts()) {
        if (stmt->Type() != SyntaxTreeType::LocalVar) {
            continue;
        }
        const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
        const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
        if (!nl) {
            continue;
        }
        const auto explist_node = lv->Explist();
        if (!explist_node) {
            continue;
        }
        const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist_node);
        if (!el) {
            continue;
        }
        const auto &names = nl->Names();
        const auto &exps = el->Exps();
        for (size_t i = 0; i < names.size() && i < exps.size(); ++i) {
            // 若该名称已在 spec_ctx 中（例如与数学参数同名），跳过以避免覆盖。
            if (spec_ctx.count(names[i])) {
                continue;
            }
            // 先查 snapshot：若推断阶段已正确得出数值类型（算术表达式等），直接跳过
            // （EvalReturnExpType 会通过 snapshot 得到正确结果，无需手动插入）。
            const auto snap_it = snapshot.find(exps[i].get());
            if (snap_it != snapshot.end() && (snap_it->second == T_INT || snap_it->second == T_FLOAT)) {
                continue;
            }
            // snapshot 中为 T_DYNAMIC：尝试通过 EvalReturnExpType（含当前 spec_ctx）
            // 推断初始化表达式的类型，以捕获数学函数调用返回值（如 local x = f(n)）。
            const auto t = EvalReturnExpType(exps[i], snapshot, spec_ctx, math_param_positions, assumed_ret);
            if (t == T_INT || t == T_FLOAT) {
                spec_ctx[names[i]] = t;
            }
        }
    }
}

std::vector<TypeInferencer::FunctionSpecInfo> TypeInferencer::CollectFunctionSpecInfos(const CompileResult &cr) const {
    std::vector<FunctionSpecInfo> infos;
    const auto chunk = cr.chunk;
    if (!chunk || chunk->Type() != SyntaxTreeType::Block) {
        return infos;
    }

    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
    for (const auto &stmt : top_block->Stmts()) {
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
        const auto parlist_node = funcbody_ptr->Parlist();
        if (!parlist_node) {
            continue;
        }
        const auto parlist_ptr = std::dynamic_pointer_cast<SyntaxTreeParlist>(parlist_node);
        const auto namelist_node = parlist_ptr->Namelist();
        if (!namelist_node) {
            continue;
        }
        const auto params = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist_node)->Names();
        if (params.empty()) {
            continue;
        }
        FunctionSpecInfo info;
        info.name = name;
        info.block = funcbody_ptr->Block();
        info.params = params;
        WalkSyntaxTree(info.block, [&](const SyntaxTreeInterfacePtr &n) {
            info.original_snapshot.emplace_back(n.get(), n->EvalType());
        });
        infos.emplace_back(std::move(info));
    }
    return infos;
}

std::vector<int> TypeInferencer::FindMathParamIndices(
        const FunctionSpecInfo &info,
        const EvalTypeMap &baseline,
        const EvalTypeMap &all_int,
        const std::unordered_map<std::string, std::vector<int>> &known_math_positions) {
    std::vector<int> math_indices;
    if (!HasArithmeticImprovement(all_int, baseline, info.block, known_math_positions)) {
        return math_indices;
    }

    const auto make_assumed = [&](const std::string &special_param, const InferredType special_type,
                                  const InferredType default_type) {
        std::unordered_map<std::string, InferredType> assumed;
        for (const auto &param : info.params) {
            assumed[param] = (!special_param.empty() && param == special_param) ? special_type : default_type;
        }
        return assumed;
    };

    for (int i = 0; i < static_cast<int>(info.params.size()); ++i) {
        const auto without_p_assumed = make_assumed(info.params[i], T_DYNAMIC, T_INT);
        const auto without_p_map = RunTrialInference(info.block, info.params, without_p_assumed);
        if (ParamAffectsArithmetic(all_int, without_p_map, info.block, known_math_positions)) {
            math_indices.push_back(i);
        }
    }
    return math_indices;
}

void TypeInferencer::GenerateFunctionSpecializationSnapshots(CompileResult &cr,
                                                             const FunctionSpecInfo &info,
                                                             const std::vector<int> &math_indices) {
    const int num_specs = 1 << static_cast<int>(math_indices.size());
    auto &snapshots = cr.specialization_snapshots[info.name];
    snapshots.resize(num_specs);
    for (int bitmask = 0; bitmask < num_specs; ++bitmask) {
        std::unordered_map<std::string, InferredType> assumed;
        for (const auto &param : info.params) {
            assumed[param] = T_DYNAMIC;
        }
        for (int i = 0; i < static_cast<int>(math_indices.size()); ++i) {
            const auto param_index = static_cast<size_t>(math_indices[i]);
            assumed[info.params[param_index]] =
                    (MathParamKindOf(bitmask, i) == kMathParamFloat) ? T_FLOAT : T_INT;
        }
        snapshots[bitmask] = RunTrialInference(info.block, info.params, assumed);
    }
}

std::unordered_map<std::string, TypeInferencer::FuncRetInfo> TypeInferencer::BuildFunctionReturnCache(
        const std::unordered_map<std::string, std::pair<SyntaxTreeInterfacePtr, std::vector<std::string>>> &math_func_info) const {
    std::unordered_map<std::string, FuncRetInfo> func_ret_cache;
    for (const auto &[func_name, info] : math_func_info) {
        FuncRetInfo ret_info;
        ret_info.ends_with_return = CollectReturnExps(info.first, ret_info.ret_exps);
        func_ret_cache[func_name] = std::move(ret_info);
    }
    return func_ret_cache;
}

std::unordered_map<std::string, std::vector<InferredType>> TypeInferencer::InferSpecializationReturnTypes(
        const CompileResult &cr,
        const std::unordered_map<std::string, std::pair<SyntaxTreeInterfacePtr, std::vector<std::string>>> &math_func_info,
        const std::unordered_map<std::string, FuncRetInfo> &func_ret_cache) const {
    std::unordered_map<std::string, std::vector<InferredType>> assumed_ret;
    for (const auto &[func_name, math_params] : cr.math_param_positions) {
        assumed_ret[func_name].assign(static_cast<size_t>(1 << static_cast<int>(math_params.size())), T_INT);
    }

    for (int round = 0; round < kMaxSpecIterations; ++round) {
        bool changed = false;
        for (const auto &[func_name, info] : math_func_info) {
            const auto &[func_block, func_params] = info;
            const auto &math_indices = cr.math_param_positions.at(func_name);
            const auto &snapshots = cr.specialization_snapshots.at(func_name);
            const auto &ret_info = func_ret_cache.at(func_name);
            const int num_specs = static_cast<int>(snapshots.size());

            for (int bitmask = 0; bitmask < num_specs; ++bitmask) {
                const auto &snapshot = snapshots[static_cast<size_t>(bitmask)];
                std::unordered_map<std::string, InferredType> spec_ctx;
                for (int i = 0; i < static_cast<int>(math_indices.size()); ++i) {
                    const auto param_index = static_cast<size_t>(math_indices[i]);
                    spec_ctx[func_params[param_index]] =
                            (MathParamKindOf(bitmask, i) == kMathParamFloat) ? T_FLOAT : T_INT;
                }

                BuildLocalVarExtensions(func_block, snapshot, spec_ctx, cr.math_param_positions, assumed_ret);

                InferredType actual_ret = T_DYNAMIC;
                if (ret_info.ends_with_return && !ret_info.ret_exps.empty()) {
                    actual_ret = T_INT;
                    for (const auto &ret_exp : ret_info.ret_exps) {
                        if (!ret_exp) {
                            actual_ret = T_DYNAMIC;
                            break;
                        }
                        const auto inferred = EvalReturnExpType(ret_exp, snapshot, spec_ctx,
                                                                cr.math_param_positions, assumed_ret);
                        if (inferred == T_FLOAT) {
                            if (actual_ret == T_INT) {
                                actual_ret = T_FLOAT;
                            }
                        } else if (inferred != T_INT) {
                            actual_ret = T_DYNAMIC;
                            break;
                        }
                    }
                }

                if (actual_ret != assumed_ret[func_name][static_cast<size_t>(bitmask)]) {
                    assumed_ret[func_name][static_cast<size_t>(bitmask)] = actual_ret;
                    changed = true;
                }
            }
        }
        if (!changed) {
            break;
        }
    }

    return assumed_ret;
}

void TypeInferencer::DiscoverMathParams(CompileResult &cr) {
    std::unordered_map<std::string, std::pair<SyntaxTreeInterfacePtr, std::vector<std::string>>> math_func_info;
    const auto function_infos = CollectFunctionSpecInfos(cr);
    for (const auto &info : function_infos) {
        const auto make_assumed = [&](const std::string &special_param, const InferredType special_type,
                                      const InferredType default_type) {
            std::unordered_map<std::string, InferredType> assumed;
            for (const auto &param : info.params) {
                assumed[param] = (!special_param.empty() && param == special_param) ? special_type : default_type;
            }
            return assumed;
        };
        const auto baseline = RunTrialInference(info.block, info.params, make_assumed("", T_DYNAMIC, T_DYNAMIC));
        const auto all_int = RunTrialInference(info.block, info.params, make_assumed("", T_INT, T_INT));
        const auto math_indices = FindMathParamIndices(info, baseline, all_int, cr.math_param_positions);
        for (const auto &[ptr, type] : info.original_snapshot) {
            ptr->SetEvalType(type);
        }

        if (math_indices.empty()) {
            continue;
        }
        cr.math_param_positions[info.name] = math_indices;
        math_func_info[info.name] = {info.block, info.params};
        LOG_INFO("TypeInferencer: {} math params for {}", math_indices.size(), info.name);
        GenerateFunctionSpecializationSnapshots(cr, info, math_indices);
    }

    if (math_func_info.empty()) {
        return;
    }
    const auto func_ret_cache = BuildFunctionReturnCache(math_func_info);
    cr.specialization_return_types = InferSpecializationReturnTypes(cr, math_func_info, func_ret_cache);
}

}// namespace fakelua
