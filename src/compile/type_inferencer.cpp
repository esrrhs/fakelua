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
    DEBUG_ASSERT(old_type != T_UNKNOWN);
    DEBUG_ASSERT(new_type != T_UNKNOWN);
    if (old_type == new_type) {
        return old_type;
    }
    return T_DYNAMIC;
}

InferResult TypeInferencer::Process(const ParseResult &pr) {
    InferResult ir;
    InferNode(pr.chunk);
    // 将当前推断结果复制为全局主快照，供 CGen 在非特化路径下查询节点类型。
    ir.main_eval_types = current_map_;

    // 在正常推断之后，通过迭代不动点试推断发现数学参数，写入 ir.math_param_positions。
    DiscoverMathParams(pr, ir);
    return ir;
}

InferredType TypeInferencer::InferNode(const SyntaxTreeInterfacePtr &node) {
    if (!node) {
        return T_UNKNOWN;
    }

    switch (node->Type()) {
        case SyntaxTreeType::Block: {
            const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            // 独立的 do...end 块必须引入自己的作用域，使内部的
            // 局部声明不会污染（或覆盖）外围作用域。
            InferBlock(block, true);
            current_map_[block.get()] = T_UNKNOWN;
            return T_UNKNOWN;
        }
        case SyntaxTreeType::LocalVar: {
            const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
            const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(local_var->Namelist());
            std::vector<SyntaxTreeInterfacePtr> exps;
            if (const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(local_var->Explist())) {
                exps = explist->Exps();
            }

            if (!namelist) {                        // GCOVR_EXCL_START
                current_map_[node.get()] = T_UNKNOWN;// 防御性检查：语法解析器保证 LocalVar 始终有非空 namelist，此分支不可达。
                return T_UNKNOWN;
            }                                        // GCOVR_EXCL_STOP

            const auto &names = namelist->Names();
            for (size_t i = 0; i < names.size(); ++i) {
                InferredType type = T_DYNAMIC;
                if (i < exps.size()) {
                    type = InferNode(exps[i]);
                }
                env_.Define(names[i], type);
                // 文件顶层（!in_funcbody_）数值类型局部变量：
                // 将其记录到 file_level_types_，供 RunTrialInference
                // 在重置 env_ 后重新注入，使函数特化试推断能看到正确类型。
                if (!in_funcbody_ && IsNumericInferredType(type)) {
                    file_level_types_[names[i]] = type;
                }
            }

            current_map_[node.get()] = T_UNKNOWN;
            return T_UNKNOWN;
        }
        case SyntaxTreeType::Assign: {
            const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(node);
            const auto varlist = std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist());
            const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(assign->Explist());
            DEBUG_ASSERT(varlist && explist && !varlist->Vars().empty() && !explist->Exps().empty());

            DEBUG_ASSERT(varlist->Vars().size() == 1 && explist->Exps().size() == 1);// 预处理阶段已将多赋值拆分成单赋值

            const InferredType rhs_type = InferNode(explist->Exps()[0]);
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(varlist->Vars()[0]);
            DEBUG_ASSERT(var && var->GetVarKind() == VarKind::kSimple);

            const std::string name = var->GetName();
            // 若变量被固定（数学参数在特化试推断中），跳过 env 降级，
            // 以保持与运行时类型检查语义一致：赋值失败时会抛异常，不会改变变量类型。
            if (pinned_vars_.contains(name)) {
                const auto current = env_.Lookup(name);
                current_map_[var.get()] = current;
                current_map_[node.get()] = current;
                return current;
            }
            if (!env_.Update(name, rhs_type)) {
                current_map_[var.get()] = T_DYNAMIC;
                current_map_[node.get()] = T_DYNAMIC;
                return T_DYNAMIC;
            }

            const auto current = env_.Lookup(name);
            current_map_[var.get()] = current;
            current_map_[node.get()] = current;
            return current;
        }
        case SyntaxTreeType::ForLoop: {
            const auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
            const InferredType begin_type = InferNode(for_loop->ExpBegin());
            const InferredType end_type = InferNode(for_loop->ExpEnd());
            const InferredType step_type = InferNode(for_loop->ExpStep());

            // 仅当所有边界都是 T_INT 时才将循环变量标记为 T_INT，
            // 这与 CGen 使用的整型特化路径相匹配。当所有边界均为数值（T_INT 或 T_FLOAT）
            // 但并非全为 T_INT 时，标记为 T_FLOAT 以启用 double 快路径。
            // 当任何边界为 T_DYNAMIC 时，CGen 会生成 CVar 循环控制变量，
            // 因此循环变量也必须是 T_DYNAMIC 以保持类型一致。
            const bool begin_valid = for_loop->ExpBegin() != nullptr;
            const bool end_valid = for_loop->ExpEnd() != nullptr;
            const bool step_numeric = !for_loop->ExpStep() || step_type == T_INT || step_type == T_FLOAT;
            const bool all_int = begin_valid && end_valid && begin_type == T_INT &&
                                 end_type == T_INT &&
                                 (!for_loop->ExpStep() || step_type == T_INT);
            const bool all_numeric =
                    !all_int && begin_valid && end_valid &&
                    (begin_type == T_INT || begin_type == T_FLOAT) &&
                    (end_type == T_INT || end_type == T_FLOAT) && step_numeric;
            const InferredType loop_var_type = all_int ? T_INT : (all_numeric ? T_FLOAT : T_DYNAMIC);

            env_.EnterScope();
            env_.Define(for_loop->Name(), loop_var_type);
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(for_loop->Block()), false);
            const InferredType final_loop_var_type = env_.Lookup(for_loop->Name());
            env_.ExitScope();

            current_map_[node.get()] = final_loop_var_type;
            return final_loop_var_type;
        }
        case SyntaxTreeType::Function: {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(node);
            InferNode(func->Funcbody());
            current_map_[node.get()] = T_UNKNOWN;
            return T_UNKNOWN;
        }
        case SyntaxTreeType::LocalFunction: {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node);
            InferNode(func->Funcbody());
            current_map_[node.get()] = T_UNKNOWN;
            return T_UNKNOWN;
        }
        case SyntaxTreeType::FuncBody: {
            const auto funcbody = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(node);
            const bool saved_in_funcbody = in_funcbody_;
            in_funcbody_ = true;
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
            in_funcbody_ = saved_in_funcbody;
            current_map_[node.get()] = T_UNKNOWN;
            return T_UNKNOWN;
        }
        case SyntaxTreeType::Return: {
            const auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(node);
            InferNode(ret->Explist());
            current_map_[node.get()] = T_UNKNOWN;
            return T_UNKNOWN;
        }
        case SyntaxTreeType::ExpList: {
            const auto exp_list = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
            InferredType last = T_UNKNOWN;
            for (const auto &exp: exp_list->Exps()) {
                last = InferNode(exp);
            }
            current_map_[node.get()] = last;
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
            InferNode(functioncall->prefixexp());
            InferNode(functioncall->Args());
            // 当正在运行携带被调函数返回类型提示的试推断时，尝试解析该函数调用的实际返回类型。
            // 主推断遍（trial_assumed_ret_ == nullptr）保持原有的 T_DYNAMIC 行为。
            const auto ret_type = ResolveCallReturnType(functioncall);
            current_map_[node.get()] = ret_type;
            return ret_type;
        }
        case SyntaxTreeType::TableConstructor: {
            const auto tableconstructor = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node);
            InferNode(tableconstructor->Fieldlist());
            current_map_[node.get()] = T_DYNAMIC;
            return T_DYNAMIC;
        }
        case SyntaxTreeType::Args: {
            const auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node);
            InferNode(args->Explist());
            InferNode(args->Tableconstructor());
            InferNode(args->String());
            current_map_[node.get()] = T_DYNAMIC;
            return T_DYNAMIC;
        }
        case SyntaxTreeType::FieldList: {
            const auto fieldlist = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
            for (const auto &field: fieldlist->Fields()) {
                InferNode(field);
            }
            current_map_[node.get()] = T_DYNAMIC;
            return T_DYNAMIC;
        }
        case SyntaxTreeType::Field: {
            const auto field = std::dynamic_pointer_cast<SyntaxTreeField>(node);
            InferNode(field->Key());
            InferNode(field->Value());
            current_map_[node.get()] = T_DYNAMIC;
            return T_DYNAMIC;
        }
        case SyntaxTreeType::If: {
            const auto if_stmt = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
            InferNode(if_stmt->Exp());
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(if_stmt->Block()), true);
            if (const auto elseifs = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(if_stmt->ElseIfs())) {
                for (size_t i = 0; i < elseifs->ElseifSize(); ++i) {
                    InferNode(elseifs->ElseifExp(i));
                    InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(elseifs->ElseifBlock(i)), true);
                }
            }
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(if_stmt->ElseBlock()), true);
            current_map_[node.get()] = T_UNKNOWN;
            return T_UNKNOWN;
        }
        case SyntaxTreeType::While: {
            const auto while_stmt = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
            InferNode(while_stmt->Exp());
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(while_stmt->Block()), true);
            current_map_[node.get()] = T_UNKNOWN;
            return T_UNKNOWN;
        }
        case SyntaxTreeType::Repeat: {
            const auto repeat_stmt = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
            // Lua 语义：until 条件可以访问 repeat 块内声明的 local 变量。
            // 因此必须在 until 条件推断完成之后再退出作用域，而不能在块结束时立即退出。
            env_.EnterScope();
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(repeat_stmt->Block()), false);
            InferNode(repeat_stmt->Exp());
            env_.ExitScope();
            current_map_[node.get()] = T_UNKNOWN;
            return T_UNKNOWN;
        }
        case SyntaxTreeType::ForIn: {
            const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
            InferNode(for_in->Namelist());
            InferNode(for_in->Explist());
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(for_in->Block()), true);
            current_map_[node.get()] = T_UNKNOWN;
            return T_UNKNOWN;
        }
        case SyntaxTreeType::Empty:
        case SyntaxTreeType::Label:
        case SyntaxTreeType::Break:
        case SyntaxTreeType::Goto:
        case SyntaxTreeType::NameList: {
            // 这些节点是语句或辅助结构，没有表达式类型
            current_map_[node.get()] = T_UNKNOWN;
            return T_UNKNOWN;
        }
        default: {
            ThrowFakeluaException(
                    std::format("InferNode: unexpected SyntaxTreeType: {}", SyntaxTreeTypeToString(node->Type())));
        }
    }
}

InferredType TypeInferencer::InferExp(const std::shared_ptr<SyntaxTreeExp> &exp) {
    const auto exp_kind = exp->GetExpKind();

    if (exp_kind == ExpKind::kNumber) {
        const auto &value = exp->ExpValue();
        const auto ret = IsInteger(value) ? T_INT : T_FLOAT;
        current_map_[exp.get()] = ret;
        return ret;
    }
    if (exp_kind == ExpKind::kPrefixExp) {
        const auto ret = InferNode(exp->Right());
        current_map_[exp.get()] = ret;
        return ret;
    }
    if (exp_kind == ExpKind::kBinop) {
        const auto left_type = InferNode(exp->Left());
        const auto right_type = InferNode(exp->Right());

        if (left_type == T_DYNAMIC || right_type == T_DYNAMIC) {
            current_map_[exp.get()] = T_DYNAMIC;
            return T_DYNAMIC;
        }

        const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(exp->Op());
        DEBUG_ASSERT(op);

        const auto op_kind = op->GetOpKind();

        // 保持 INT+INT=INT、混合→FLOAT 语义的算术运算
        if (op_kind == BinOpKind::kPlus || op_kind == BinOpKind::kMinus || op_kind == BinOpKind::kStar ||
            op_kind == BinOpKind::kDoubleSlash || op_kind == BinOpKind::kMod) {
            if (left_type == T_INT && right_type == T_INT) {
                current_map_[exp.get()] = T_INT;
                return T_INT;
            }
            if ((left_type == T_INT || left_type == T_FLOAT) && (right_type == T_INT || right_type == T_FLOAT)) {
                current_map_[exp.get()] = T_FLOAT;
                return T_FLOAT;
            }
        }

        // 结果始终为 FLOAT 的运算
        if (op_kind == BinOpKind::kSlash || op_kind == BinOpKind::kPow) {
            if ((left_type == T_INT || left_type == T_FLOAT) && (right_type == T_INT || right_type == T_FLOAT)) {
                current_map_[exp.get()] = T_FLOAT;
                return T_FLOAT;
            }
        }

        // 位运算：两个操作数均为 T_INT 时结果为 T_INT
        if (op_kind == BinOpKind::kBitAnd || op_kind == BinOpKind::kBitOr || op_kind == BinOpKind::kXor ||
            op_kind == BinOpKind::kLeftShift || op_kind == BinOpKind::kRightShift) {
            if (left_type == T_INT && right_type == T_INT) {
                current_map_[exp.get()] = T_INT;
                return T_INT;
            }
        }

        // AND/OR：Lua 中整数和浮点数始终为真值（包括 0），因此：
        //   a and b（a 为 T_INT/T_FLOAT）：a 始终为真，结果为 b → 类型为 right_type
        //   a or  b（a 为 T_INT/T_FLOAT）：a 始终为真，结果为 a → 类型为 left_type
        if (op_kind == BinOpKind::kAnd) {
            if ((left_type == T_INT || left_type == T_FLOAT) && (right_type == T_INT || right_type == T_FLOAT)) {
                current_map_[exp.get()] = right_type;
                return right_type;
            }
        }
        if (op_kind == BinOpKind::kOr) {
            if ((left_type == T_INT || left_type == T_FLOAT) && (right_type == T_INT || right_type == T_FLOAT)) {
                current_map_[exp.get()] = left_type;
                return left_type;
            }
        }

        current_map_[exp.get()] = T_DYNAMIC;
        return T_DYNAMIC;
    }

    if (exp_kind == ExpKind::kUnop) {
        const auto operand_type = InferNode(exp->Right());
        const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(exp->Op());
        DEBUG_ASSERT(op);
        const auto op_kind = op->GetOpKind();
        if (op_kind == UnOpKind::kMinus) {
            if (operand_type == T_INT) {
                current_map_[exp.get()] = T_INT;
                return T_INT;
            }
            if (operand_type == T_FLOAT) {
                current_map_[exp.get()] = T_FLOAT;
                return T_FLOAT;
            }
        }
        if (op_kind == UnOpKind::kBitNot) {
            if (operand_type == T_INT) {
                current_map_[exp.get()] = T_INT;
                return T_INT;
            }
        }
        if (op_kind == UnOpKind::kNumberSign) {
            // # 运算符始终返回整数（字符串字节数或表元素数）。
            // 无论操作数是字符串还是表（均为 T_DYNAMIC），结果类型始终为 T_INT。
            current_map_[exp.get()] = T_INT;
            return T_INT;
        }
        current_map_[exp.get()] = T_DYNAMIC;
        return T_DYNAMIC;
    }

    current_map_[exp.get()] = T_DYNAMIC;
    return T_DYNAMIC;
}

InferredType TypeInferencer::InferPrefixExp(const std::shared_ptr<SyntaxTreePrefixexp> &prefix_exp) {
    const auto prefix_kind = prefix_exp->GetPrefixKind();
    InferredType ret = T_DYNAMIC;

    if (prefix_kind == PrefixExpKind::kVar || prefix_kind == PrefixExpKind::kExp) {
        ret = InferNode(prefix_exp->GetValue());
    } else if (prefix_kind == PrefixExpKind::kFunctionCall) {
        // 传播 InferNode(FunctionCall) 的返回值：携带提示时为实际返回类型，否则为 T_DYNAMIC。
        ret = InferNode(prefix_exp->GetValue());
    }

    current_map_[prefix_exp.get()] = ret;
    return ret;
}

InferredType TypeInferencer::InferVar(const std::shared_ptr<SyntaxTreeVar> &var) {
    if (var->GetVarKind() == VarKind::kSimple) {
        const auto ret = env_.Lookup(var->GetName());
        current_map_[var.get()] = ret;
        return ret;
    }

    // 对于"方括号"和"点号"变量，处理子表达式以便内部变量
    //（例如用作表索引的整型循环变量）被记录到 current_map_，
    // 从而使 CGen 在生成变量引用时能通过 LookupNodeType 查到其原生类型，
    // 而不会在需要 CVar 的地方错误地发出原始 int64_t 变量名。
    if (const auto pe = var->GetPrefixexp()) {
        InferNode(pe);
    }
    if (var->GetVarKind() == VarKind::kSquare) {
        if (const auto exp = var->GetExp()) {
            InferNode(exp);
        }
    }

    current_map_[var.get()] = T_DYNAMIC;
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
        InferNode(stmt);
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
        DEBUG_ASSERT(namelist);
        std::vector<SyntaxTreeInterfacePtr> exps;
        if (const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(local_var->Explist())) {
            exps = explist->Exps();
        }
        const auto &names = namelist->Names();
        for (size_t i = 0; i < names.size(); ++i) {
            if (i < exps.size()) {
                const auto final_type = env_.Lookup(names[i]);
                current_map_[exps[i].get()] = final_type;
            }
        }
    }

    current_map_[block.get()] = T_UNKNOWN;
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
    if (exp->GetExpKind() == ExpKind::kBinop) {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(exp->Op());
        DEBUG_ASSERT(op);
        const auto k = op->GetOpKind();
        return k == BinOpKind::kPlus || k == BinOpKind::kMinus || k == BinOpKind::kStar ||
               k == BinOpKind::kSlash || k == BinOpKind::kDoubleSlash || k == BinOpKind::kPow ||
               k == BinOpKind::kMod || k == BinOpKind::kBitAnd || k == BinOpKind::kXor ||
               k == BinOpKind::kBitOr || k == BinOpKind::kLeftShift || k == BinOpKind::kRightShift;
    }
    if (exp->GetExpKind() == ExpKind::kUnop) {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(exp->Op());
        DEBUG_ASSERT(op);
        // 一元负号：-T_INT=T_INT，-T_FLOAT=T_FLOAT，随参数类型改变。
        // 按位取反：~T_INT=T_INT，仅对整数参数有意义。
        return op->GetOpKind() == UnOpKind::kMinus || op->GetOpKind() == UnOpKind::kBitNot;
    }
    return false;
}

// 判断 exp 节点是否为可原生化的有序比较运算符（<、<=、>、>=）。
// 当两侧操作数均为数值类型时，CGen 可通过 TryCompileNativeBoolExpr
// 直接生成原生 C 比较，无需 OpXxx 宏 + IsTrue 调用。
// 注意：== / ~= 在 Lua 中可用于任意类型（字符串、布尔值、nil 等），
// 不纳入此检测，否则会将仅含 == 的函数错误特化为数值函数，导致
// 非数值调用方（如传入字符串）触发运行时 "attempt to perform arithmetic" 错误。
bool TypeInferencer::IsNativeComparisonExpr(const SyntaxTreeInterfacePtr &node) const {
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
    return k == BinOpKind::kLess || k == BinOpKind::kLessEqual || k == BinOpKind::kMore ||
           k == BinOpKind::kMoreEqual;
}

TypeInferencer::EvalTypeMap TypeInferencer::RunTrialInference(const SyntaxTreeInterfacePtr &func_block,
                                               const std::vector<std::string> &params,
                                               const std::unordered_map<std::string, InferredType> &assumed_types,
                                               const std::unordered_map<std::string, std::vector<int>> *math_positions,
                                               const std::unordered_map<std::string, std::vector<InferredType>> *assumed_ret) {
    // 保存当前推断器状态，trial 结束后恢复。
    TypeEnvironment saved_env = env_;
    const bool saved_in_funcbody = in_funcbody_;
    // 临时注入被调函数返回类型提示，供 ResolveCallReturnType 使用。
    const auto saved_trial_math = trial_math_positions_;
    const auto saved_trial_ret = trial_assumed_ret_;
    trial_math_positions_ = math_positions;
    trial_assumed_ret_ = assumed_ret;

    // 收集被固定的变量名（数学参数，即被分配了 T_INT/T_FLOAT 的参数）。
    // 在 InferNode(Assign) 中，对这些变量的 env 更新将被跳过，以模拟运行时类型检查
    // 对变量类型的保证（类似于旧 spec_ctx 对数学参数的 "始终持有特化类型" 语义）。
    std::unordered_set<std::string> saved_pinned = std::move(pinned_vars_);
    pinned_vars_.clear();
    for (const auto &[name, t] : assumed_types) {
        if (t == T_INT || t == T_FLOAT) {
            pinned_vars_.insert(name);
        }
    }

    EvalTypeMap prev_map;

    for (int round = 0; round < kMaxSpecIterations; ++round) {
        // 每轮清除 func_block 节点在 current_map_ 中的旧条目，保证推断从干净状态开始。
        WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &n) { current_map_.erase(n.get()); });

        // 以假定的参数类型初始化 trial 环境；同时注入文件级数值常量，
        // 使函数体能看到正确的文件级局部变量类型（T_INT/T_FLOAT）。
        env_ = TypeEnvironment{};
        in_funcbody_ = true;
        for (const auto &[fname, ftype]: file_level_types_) {
            env_.Define(fname, ftype);
        }
        for (const auto &p: params) {
            const auto it = assumed_types.find(p);
            env_.Define(p, it != assumed_types.end() ? it->second : T_DYNAMIC);
        }

        // 运行函数体类型推断（不新开作用域，参数已在当前作用域中定义）。
        InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(func_block), false);

        // 快照本轮推断结果（仅 func_block 节点）。
        EvalTypeMap curr_map;
        WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &n) {
            const auto it = current_map_.find(n.get());
            curr_map[n.get()] = (it != current_map_.end()) ? it->second : T_UNKNOWN;
        });

        if (curr_map == prev_map) {
            // 已达到不动点，提前退出。
            break;
        }
        prev_map = std::move(curr_map);
    }

    // 恢复推断器状态（含被调函数提示指针和固定变量集合）。
    env_ = std::move(saved_env);
    in_funcbody_ = saved_in_funcbody;
    trial_math_positions_ = saved_trial_math;
    trial_assumed_ret_ = saved_trial_ret;
    pinned_vars_ = std::move(saved_pinned);

    return prev_map;
}

bool TypeInferencer::HasArithmeticImprovement(const EvalTypeMap &all_int, const EvalTypeMap &baseline,
                                               const SyntaxTreeInterfacePtr &func_block,
                                               const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const {
    return CheckArithmeticTypeChanges(all_int, baseline, func_block, true, math_param_positions);
}

bool TypeInferencer::ParamAffectsArithmetic(const EvalTypeMap &all_int, const EvalTypeMap &without_p,
                                              const SyntaxTreeInterfacePtr &func_block,
                                              const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const {
    return CheckArithmeticTypeChanges(all_int, without_p, func_block, false, math_param_positions);
}

bool TypeInferencer::CheckArithmeticTypeChanges(const EvalTypeMap &typed_map, const EvalTypeMap &compare_map,
                                                 const SyntaxTreeInterfacePtr &func_block,
                                                 bool require_compare_dynamic,
                                                 const std::unordered_map<std::string, std::vector<int>> &math_param_positions) const {
    if (HasArithmeticNodeTypeChange(typed_map, compare_map, func_block, require_compare_dynamic)) {
        return true;
    }
    if (HasComparisonOperandTypeChange(typed_map, compare_map, func_block)) {
        return true;
    }
    if (HasForLoopTypeChange(typed_map, compare_map, func_block)) {
        return true;
    }
    return HasMathCallImprovement(func_block, typed_map, compare_map, math_param_positions);
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
        DEBUG_ASSERT(fc);
        const auto callee_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
        DEBUG_ASSERT(callee_pe && callee_pe->GetPrefixKind() == PrefixExpKind::kVar);
        const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(callee_pe->GetValue());
        DEBUG_ASSERT(callee_var && callee_var->GetVarKind() == VarKind::kSimple);
        const auto &callee_name = callee_var->GetName();
        const auto math_it = math_param_positions.find(callee_name);
        if (math_it == math_param_positions.end()) {
            return;
        }
        const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
        DEBUG_ASSERT(args_ptr);
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
            DEBUG_ASSERT(it_typed != typed_map.end() && it_comp != compare_map.end());
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
        DEBUG_ASSERT(it_typed != typed_map.end() && it_compare != compare_map.end());
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
                                                    const SyntaxTreeInterfacePtr &func_block) const {
    bool found = false;
    WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
        if (found || !IsNativeComparisonExpr(node)) {
            return;
        }
        const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
        const auto left = exp->Left();
        const auto right = exp->Right();
        DEBUG_ASSERT(left && right);
        const auto lt_typed = typed_map.find(left.get());
        const auto rt_typed = typed_map.find(right.get());
        const auto lt_compare = compare_map.find(left.get());
        const auto rt_compare = compare_map.find(right.get());
        DEBUG_ASSERT(lt_typed != typed_map.end() && rt_typed != typed_map.end() &&
                     lt_compare != compare_map.end() && rt_compare != compare_map.end());
        const bool both_typed = IsNumericInferredType(lt_typed->second) &&
                                IsNumericInferredType(rt_typed->second);
        if (!both_typed) {
            return;
        }
        if (lt_compare->second == T_DYNAMIC || rt_compare->second == T_DYNAMIC) {
            found = true;
        }
    });
    return found;
}

bool TypeInferencer::HasForLoopTypeChange(const EvalTypeMap &typed_map,
                                          const EvalTypeMap &compare_map,
                                          const SyntaxTreeInterfacePtr &func_block) const {
    bool found = false;
    WalkSyntaxTree(func_block, [&](const SyntaxTreeInterfacePtr &node) {
        if (found || node->Type() != SyntaxTreeType::ForLoop) {
            return;
        }
        const auto it_typed = typed_map.find(node.get());
        const auto it_compare = compare_map.find(node.get());
        DEBUG_ASSERT(it_typed != typed_map.end() && it_compare != compare_map.end());
        if (!IsNumericInferredType(it_typed->second)) {
            return;
        }
        if (it_compare->second == T_DYNAMIC) {
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
    switch (last->Type()) {
        case SyntaxTreeType::Return:
            return true;
        case SyntaxTreeType::Block:
            // do...end 块：无条件执行（不同于 if/while 可能跳过），内部可含任意控制流。
            // 若其所有路径均以 return 结束，则外层函数视为已返回。
            return AllPathsReturn(last);
        case SyntaxTreeType::If: {
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
        case SyntaxTreeType::Assign:
        case SyntaxTreeType::LocalVar:
        case SyntaxTreeType::LocalFunction:
        case SyntaxTreeType::Function:
        case SyntaxTreeType::FunctionCall:
        case SyntaxTreeType::While:
        case SyntaxTreeType::Repeat:
        case SyntaxTreeType::ForLoop:
        case SyntaxTreeType::ForIn:
        case SyntaxTreeType::Break:
        case SyntaxTreeType::Goto:
        case SyntaxTreeType::Label:
        case SyntaxTreeType::Empty:
            return false;
        default:
            ThrowFakeluaException(
                    std::format("AllPathsReturn: unexpected statement type {}", SyntaxTreeTypeToString(last->Type())));
    }
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
    DEBUG_ASSERT(block);
    const auto &stmts = block->Stmts();
    if (stmts.empty()) {
        return false;
    }
    const bool ends_with_return = AllPathsReturn(block_node);
    for (const auto &stmt : stmts) {
        switch (stmt->Type()) {
            case SyntaxTreeType::Return: {
                const auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(stmt);
                const auto explist = ret->Explist();
                if (explist) {
                    const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
                    if (el && !el->Exps().empty()) {
                        ret_exps.push_back(el->Exps()[0]);
                        break;
                    }
                }
                ret_exps.push_back(nullptr);
                break;
            }
            case SyntaxTreeType::If: {
                const auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);
                CollectReturnExps(if_node->Block(), ret_exps);
                if (const auto elseifs = if_node->ElseIfs()) {
                    const auto el = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseifs);
                    for (const auto &blk : el->ElseifBlocks()) {
                        CollectReturnExps(blk, ret_exps);
                    }
                }
                CollectReturnExps(if_node->ElseBlock(), ret_exps);
                break;
            }
            case SyntaxTreeType::While: {
                const auto while_node = std::dynamic_pointer_cast<SyntaxTreeWhile>(stmt);
                CollectReturnExps(while_node->Block(), ret_exps);
                break;
            }
            case SyntaxTreeType::Repeat: {
                const auto rep = std::dynamic_pointer_cast<SyntaxTreeRepeat>(stmt);
                CollectReturnExps(rep->Block(), ret_exps);
                break;
            }
            case SyntaxTreeType::ForLoop: {
                const auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt);
                CollectReturnExps(for_loop->Block(), ret_exps);
                break;
            }
            case SyntaxTreeType::ForIn: {
                const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt);
                CollectReturnExps(for_in->Block(), ret_exps);
                break;
            }
            case SyntaxTreeType::Block:
                // do...end 块：递归收集其内部的 return 表达式。
                CollectReturnExps(stmt, ret_exps);
                break;
            case SyntaxTreeType::Assign:
            case SyntaxTreeType::LocalVar:
            case SyntaxTreeType::LocalFunction:
            case SyntaxTreeType::Function:
            case SyntaxTreeType::FunctionCall:
            case SyntaxTreeType::Break:
            case SyntaxTreeType::Goto:
            case SyntaxTreeType::Label:
            case SyntaxTreeType::Empty:
                // 不含 return，无需递归。
                break;
            default:
                ThrowFakeluaException(
                        std::format("CollectReturnExps: unexpected statement type {}",
                                    SyntaxTreeTypeToString(stmt->Type())));
        }
    }
    return ends_with_return;
}

// ---------------------------------------------------------------------------
// ResolveCallReturnType —— 试推断期间查询被调函数的返回类型
//
// 仅在 trial_math_positions_ 和 trial_assumed_ret_ 均非 null 时生效（即携带被调函数
// 返回类型提示的试推断轮次中）。对于普通主推断遍，始终返回 T_DYNAMIC（行为与以前相同）。
//
// 算法：
//   1. 从函数调用 AST 节点中提取被调函数名；
//   2. 查找该函数是否为已知数学函数（存在于 trial_math_positions_ 中）；
//   3. 从 current_map_ 中读取各数学参数位置的实参类型，构造 bitmask；
//   4. 在 trial_assumed_ret_ 中查找对应 bitmask 的返回类型并返回。
// ---------------------------------------------------------------------------
InferredType TypeInferencer::ResolveCallReturnType(
        const std::shared_ptr<SyntaxTreeFunctioncall> &fc) const {
    if (!trial_math_positions_ || !trial_assumed_ret_) {
        return T_DYNAMIC;
    }
    // 提取被调函数名（仅支持简单形式 callee(...)，方法调用形式不处理）。
    const auto callee_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
    DEBUG_ASSERT(callee_pe && callee_pe->GetPrefixKind() == PrefixExpKind::kVar);
    const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(callee_pe->GetValue());
    DEBUG_ASSERT(callee_var && callee_var->GetVarKind() == VarKind::kSimple);
    const auto &callee_name = callee_var->GetName();

    const auto math_it = trial_math_positions_->find(callee_name);
    if (math_it == trial_math_positions_->end()) {
        return T_DYNAMIC;
    }
    const auto ret_it = trial_assumed_ret_->find(callee_name);
    DEBUG_ASSERT(ret_it != trial_assumed_ret_->end());

    // 从已推断的实参类型（current_map_ 中已由 InferNode(Args) 填充）构造 bitmask。
    const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
    DEBUG_ASSERT(args_ptr);
    const auto raw_args = ExtractCallRawArgs(args_ptr);
    const auto &math_params = math_it->second;

    int bitmask = 0;
    for (int i = 0; i < static_cast<int>(math_params.size()); ++i) {
        const int param_pos = math_params[static_cast<size_t>(i)];
        if (param_pos >= static_cast<int>(raw_args.size())) {
            return T_DYNAMIC;
        }
        const auto arg_it = current_map_.find(raw_args[static_cast<size_t>(param_pos)].get());
        DEBUG_ASSERT(arg_it != current_map_.end());
        if (arg_it->second == T_FLOAT) {
            bitmask |= (1 << i);
        } else if (arg_it->second != T_INT) {
            // 实参不是数值类型，无法确定特化 bitmask。
            return T_DYNAMIC;
        }
    }

    const auto &ret_types = ret_it->second;
    DEBUG_ASSERT(bitmask < static_cast<int>(ret_types.size()));
    return ret_types[static_cast<size_t>(bitmask)];
}

// ---------------------------------------------------------------------------
// ComputeReturnTypeFromSnapshot —— 从快照中直接读取 return 表达式的推断类型
//
// 与旧的 EvalReturnExpType 不同，本函数不递归重新计算表达式，
// 而是直接在（由 RunTrialInference 生成的、已注入被调函数返回类型提示的）
// 快照中查找每个 return 表达式节点的类型。
// 快照已通过提示注入而精确，函数调用节点的类型就是被调函数的实际返回类型。
// ---------------------------------------------------------------------------
InferredType TypeInferencer::ComputeReturnTypeFromSnapshot(
        const EvalTypeSnapshot &snapshot,
        const FuncRetInfo &ret_info) const {
    if (!ret_info.ends_with_return || ret_info.ret_exps.empty()) {
        return T_DYNAMIC;
    }
    InferredType actual_ret = T_INT;  // 乐观初值
    for (const auto &ret_exp : ret_info.ret_exps) {
        if (!ret_exp) {
            // nullptr 代表显式的 nil return（return 无表达式）。
            return T_DYNAMIC;
        }
        const auto it = snapshot.find(ret_exp.get());
        const auto inferred = (it != snapshot.end()) ? it->second : T_DYNAMIC;
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

std::vector<TypeInferencer::FunctionSpecInfo> TypeInferencer::CollectFunctionSpecInfos(const ParseResult &pr) const {
    std::vector<FunctionSpecInfo> infos;
    const auto chunk = pr.chunk;
    DEBUG_ASSERT(chunk && chunk->Type() == SyntaxTreeType::Block);

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
        DEBUG_ASSERT(namelist_node);
        const auto params = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist_node)->Names();
        DEBUG_ASSERT(!params.empty());
        FunctionSpecInfo info;
        info.name = name;
        info.block = funcbody_ptr->Block();
        info.params = params;
        infos.emplace_back(std::move(info));
    }
    return infos;
}

// ---------------------------------------------------------------------------
// FindMathParamIndices —— 逐参数敏感性测试
//
// 算法：
//   (1) 先检查全参数 T_INT（all_int）相对于 baseline 是否有算术改善。
//       若无改善，则没有任何参数是数学参数，直接返回空列表。
//   (2) 对每个参数 p_i，构造"无 p_i"假设：其余参数均为 T_INT，p_i 为 T_DYNAMIC，
//       运行 RunTrialInference 得到 without_p 快照。
//   (3) 对比 all_int 与 without_p：若去掉 p_i 后算术表达式或比较/for-loop 退化，
//       则 p_i 确实影响算术运算，将其记录为数学参数。
//
// 注意：此处使用"去掉一个参数后是否退化"而非"加上一个参数后是否改善"，
// 目的是检测参数之间的耦合——若 p_i 仅在与 p_j 共同为 T_INT 时才能使算术
// 节点特化，单独将 p_i 置回 T_DYNAMIC 也会触发退化，两者都会被记录。
// ---------------------------------------------------------------------------
std::vector<int> TypeInferencer::FindMathParamIndices(
        const FunctionSpecInfo &info,
        const EvalTypeMap &baseline,
        const EvalTypeMap &all_int,
        const std::unordered_map<std::string, std::vector<int>> &known_math_positions) {
    std::vector<int> math_indices;
    // 快速剪枝：若全 T_INT 与 baseline 无改善，函数不具备特化价值。
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
        // without_p：除 p_i 为 T_DYNAMIC 外，其余参数均为 T_INT。
        const auto without_p_assumed = make_assumed(info.params[i], T_DYNAMIC, T_INT);
        const auto without_p_map = RunTrialInference(info.block, info.params, without_p_assumed);
        // 若去掉 p_i 后算术/比较/for-loop 退化，则 p_i 是数学参数。
        if (ParamAffectsArithmetic(all_int, without_p_map, info.block, known_math_positions)) {
            math_indices.push_back(i);
        }
    }
    return math_indices;
}

// 为函数 info 的所有 2^k 个特化版本生成 AST 节点类型快照并存入 ir。
// 对每个 bitmask（0 ~ 2^k-1），按各数学参数的 int/float 分配构造假设类型表，
// 运行 RunTrialInference 得到该参数组合下整个函数体的节点类型快照。
// assumed_ret 非 null 时，试推断期间会将被调函数返回类型注入 InferNode(FunctionCall)，
// 使函数调用节点和下游（如 local x = f(n)）在快照中获得精确类型。
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

// ---------------------------------------------------------------------------
// InferSpecializationReturnTypes —— 快照再生成 + 返回类型不动点推断
//
// 问题背景：
//   GenerateFunctionSpecializationSnapshots 生成的初始快照中函数调用节点的类型
//   为 T_DYNAMIC，无法反映被调函数的实际返回类型，导致下游局部变量（如 local x = f(n)）
//   在快照中也是 T_DYNAMIC，CGen 无法为其生成原生类型变量。
//
// 新算法（最多 kMaxSpecIterations 轮）：
//   (0) 初始假设：所有特化版本的返回类型均为 T_INT（乐观初始值）。
//       用 T_INT 而非 T_DYNAMIC 初始化，使第一轮即可让被调函数调用节点获得 T_INT 类型，
//       无循环依赖时通常一轮即可收敛。
//   (1) 对每个数学函数的每个 bitmask 特化版本：
//       (a) 以数学参数的对应类型和当前 ir.specialization_return_types 为提示，
//           重新运行 RunTrialInference，生成新的精确快照；
//       (b) 将新快照写回 ir.specialization_snapshots；
//       (c) 调用 ComputeReturnTypeFromSnapshot，直接从快照中读取 return 表达式节点的类型，
//           得出本轮实际返回类型；
//       (d) 若本轮推断与上轮不同则标记 changed。
//   (2) 若某轮 changed == false，则已收敛，提前退出。
//
// 互递归收敛分析：
//   - 类型格（lattice）为 T_INT < T_FLOAT < T_DYNAMIC，单向向上演化。
//   - 每轮至多一个函数从乐观值向真实值升格，轮次上界为函数数 × 2（T_INT→T_FLOAT→T_DYNAMIC）。
//   - kMaxSpecIterations = 16 对实际代码已足够。
// ---------------------------------------------------------------------------
void TypeInferencer::InferSpecializationReturnTypes(
        InferResult &ir,
        const std::unordered_map<std::string, std::pair<SyntaxTreeInterfacePtr, std::vector<std::string>>> &math_func_info,
        const std::unordered_map<std::string, FuncRetInfo> &func_ret_cache) {
    // 乐观初始值：所有特化版本均假设返回 T_INT。
    for (const auto &[func_name, math_params] : ir.math_param_positions) {
        ir.specialization_return_types[func_name].assign(
                static_cast<size_t>(1 << static_cast<int>(math_params.size())), T_INT);
    }

    for (int round = 0; round < kMaxSpecIterations; ++round) {
        bool changed = false;
        for (const auto &[func_name, info] : math_func_info) {
            const auto &[func_block, func_params] = info;
            const auto &math_indices = ir.math_param_positions.at(func_name);
            const auto &ret_info = func_ret_cache.at(func_name);
            const int num_specs = 1 << static_cast<int>(math_indices.size());

            for (int bitmask = 0; bitmask < num_specs; ++bitmask) {
                // 以当前 assumed_ret 为提示重新运行试推断，生成精确快照。
                std::unordered_map<std::string, InferredType> assumed;
                for (const auto &p : func_params) {
                    assumed[p] = T_DYNAMIC;
                }
                for (int i = 0; i < static_cast<int>(math_indices.size()); ++i) {
                    assumed[func_params[static_cast<size_t>(math_indices[i])]] =
                            (MathParamKindOf(bitmask, i) == kMathParamFloat) ? T_FLOAT : T_INT;
                }
                auto new_snapshot = RunTrialInference(func_block, func_params, assumed,
                                                      &ir.math_param_positions,
                                                      &ir.specialization_return_types);

                // 从新快照中直接读取 return 表达式节点的类型。
                const auto new_ret = ComputeReturnTypeFromSnapshot(new_snapshot, ret_info);

                // 若快照或返回类型有变化，则更新并标记 changed。
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

// ---------------------------------------------------------------------------
// DiscoverMathParams —— 数学参数发现的顶层驱动
//
// 总体算法（三步）：
//   1. 遍历每个顶层函数，通过 CollectFunctionSpecInfos 收集候选信息；
//   2. 对每个候选函数：
//      a. 以全 T_DYNAMIC 假设运行 baseline 推断，再以全 T_INT 假设运行 all_int 推断；
//      b. 若两次推断在算术节点、比较操作数或 for-loop 类型上存在改善
//         （HasArithmeticImprovement），则认为该函数值得特化；
//      c. 逐参数测试：将某参数还原为 T_DYNAMIC（without_p），检查是否导致算术退化
//         （ParamAffectsArithmetic）。若退化则确认该参数为数学参数；
//      d. 记录数学参数下标列表，为后续 InferSpecializationReturnTypes 准备数据。
//   3. 对所有有数学参数的函数，通过 InferSpecializationReturnTypes 的不动点迭代
//      重新生成精确快照并推断每个特化版本的实际返回类型，
//      写入 ir.specialization_snapshots 和 ir.specialization_return_types。
// ---------------------------------------------------------------------------
void TypeInferencer::DiscoverMathParams(const ParseResult &pr, InferResult &ir) {
    std::unordered_map<std::string, std::pair<SyntaxTreeInterfacePtr, std::vector<std::string>>> math_func_info;
    const auto function_infos = CollectFunctionSpecInfos(pr);
    for (const auto &info : function_infos) {
        const auto make_assumed = [&](const std::string &special_param, const InferredType special_type,
                                      const InferredType default_type) {
            std::unordered_map<std::string, InferredType> assumed;
            for (const auto &param : info.params) {
                assumed[param] = (!special_param.empty() && param == special_param) ? special_type : default_type;
            }
            return assumed;
        };
        // baseline：所有参数均假设为 T_DYNAMIC；all_int：所有参数均假设为 T_INT。
        // 两次推断的对比用于判断该函数的算术表达式是否能因参数类型已知而改善。
        const auto baseline = RunTrialInference(info.block, info.params, make_assumed("", T_DYNAMIC, T_DYNAMIC));
        const auto all_int = RunTrialInference(info.block, info.params, make_assumed("", T_INT, T_INT));
        const auto math_indices = FindMathParamIndices(info, baseline, all_int, ir.math_param_positions);

        if (math_indices.empty()) {
            continue;
        }
        ir.math_param_positions[info.name] = math_indices;
        math_func_info[info.name] = {info.block, info.params};
        LOG_INFO("TypeInferencer: {} math params for {}", math_indices.size(), info.name);
        // 生成初始快照（不含被调函数返回类型提示）供后续不动点迭代精化。
        const int init_num_specs = 1 << static_cast<int>(math_indices.size());
        auto &init_snapshots = ir.specialization_snapshots[info.name];
        init_snapshots.resize(static_cast<size_t>(init_num_specs));
        for (int bitmask = 0; bitmask < init_num_specs; ++bitmask) {
            std::unordered_map<std::string, InferredType> assumed;
            for (const auto &p : info.params) {
                assumed[p] = T_DYNAMIC;
            }
            for (int i = 0; i < static_cast<int>(math_indices.size()); ++i) {
                assumed[info.params[static_cast<size_t>(math_indices[i])]] =
                        (MathParamKindOf(bitmask, i) == kMathParamFloat) ? T_FLOAT : T_INT;
            }
            init_snapshots[static_cast<size_t>(bitmask)] =
                    RunTrialInference(info.block, info.params, assumed);
        }
    }

    if (math_func_info.empty()) {
        return;
    }
    const auto func_ret_cache = BuildFunctionReturnCache(math_func_info);
    InferSpecializationReturnTypes(ir, math_func_info, func_ret_cache);
}

}// namespace fakelua
