#include "compile/type_inferencer.h"

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
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (const auto found = it->find(name); found != it->end()) {
            found->second = MergeType(found->second, type);
            return true;
        }
    }
    return false;
}

InferredType TypeEnvironment::Lookup(const std::string &name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (const auto found = it->find(name); found != it->end()) {
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

void TypeInferencer::Process(const CompileResult &cr, const CompileConfig &cfg) {
    (void)cfg;
    WalkSyntaxTree(cr.chunk, [](const SyntaxTreeInterfacePtr &ptr) { ptr->SetEvalType(T_UNKNOWN); });
    InferNode(cr.chunk);
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
                    const auto inferred = InferNode(exps[i]);
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

            const InferredType rhs_type = InferNode(explist->Exps()[0]);
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
            InferNode(for_loop->ExpBegin());
            InferNode(for_loop->ExpEnd());
            InferNode(for_loop->ExpStep());

            // 仅当所有边界都是 T_INT 时才将循环变量标记为 T_INT，
            // 这与 CGen 使用的整型特化路径相匹配。当任何边界为 T_DYNAMIC 时，
            // CGen 会生成 CVar 循环控制变量，因此循环变量也必须是 T_DYNAMIC
            // 以保持类型一致。
            const bool all_int = for_loop->ExpBegin() && for_loop->ExpEnd() &&
                                 for_loop->ExpBegin()->EvalType() == T_INT &&
                                 for_loop->ExpEnd()->EvalType() == T_INT &&
                                 (!for_loop->ExpStep() || for_loop->ExpStep()->EvalType() == T_INT);
            const InferredType loop_var_type = all_int ? T_INT : T_DYNAMIC;

            env_.EnterScope();
            env_.Define(for_loop->Name(), loop_var_type);
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(for_loop->Block()), false);
            env_.ExitScope();

            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::Function: {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(node);
            InferNode(func->Funcbody());
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::LocalFunction: {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node);
            InferNode(func->Funcbody());
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::FunctionDef: {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(node);
            InferNode(func->Funcbody());
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
            InferNode(ret->Explist());
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::ExpList: {
            const auto exp_list = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
            InferredType last = T_UNKNOWN;
            for (const auto &exp: exp_list->Exps()) {
                last = InferNode(exp);
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
        case SyntaxTreeType::FunctionCall:
        case SyntaxTreeType::TableConstructor: {
            if (node->Type() == SyntaxTreeType::FunctionCall) {
                const auto functioncall = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
                InferNode(functioncall->prefixexp());
                InferNode(functioncall->Args());
            } else {
                const auto tableconstructor = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node);
                InferNode(tableconstructor->Fieldlist());
            }
            node->SetEvalType(T_DYNAMIC);
            return T_DYNAMIC;
        }
        case SyntaxTreeType::Args: {
            const auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node);
            InferNode(args->Explist());
            InferNode(args->Tableconstructor());
            InferNode(args->String());
            node->SetEvalType(T_DYNAMIC);
            return T_DYNAMIC;
        }
        case SyntaxTreeType::FieldList: {
            const auto fieldlist = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
            for (const auto &field: fieldlist->Fields()) {
                InferNode(field);
            }
            node->SetEvalType(T_DYNAMIC);
            return T_DYNAMIC;
        }
        case SyntaxTreeType::Field: {
            const auto field = std::dynamic_pointer_cast<SyntaxTreeField>(node);
            InferNode(field->Key());
            InferNode(field->Value());
            node->SetEvalType(T_DYNAMIC);
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
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::While: {
            const auto while_stmt = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
            InferNode(while_stmt->Exp());
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(while_stmt->Block()), true);
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::Repeat: {
            const auto repeat_stmt = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
            InferBlock(std::dynamic_pointer_cast<SyntaxTreeBlock>(repeat_stmt->Block()), true);
            InferNode(repeat_stmt->Exp());
            node->SetEvalType(T_UNKNOWN);
            return T_UNKNOWN;
        }
        case SyntaxTreeType::ForIn: {
            const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
            InferNode(for_in->Namelist());
            InferNode(for_in->Explist());
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
        const auto ret = InferNode(exp->Right());
        exp->SetEvalType(ret);
        return ret;
    }
    if (exp_type == "binop") {
        const auto left_type = InferNode(exp->Left());
        const auto right_type = InferNode(exp->Right());

        if (left_type == T_DYNAMIC || right_type == T_DYNAMIC) {
            exp->SetEvalType(T_DYNAMIC);
            return T_DYNAMIC;
        }

        const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(exp->Op());
        if (!op) {
            exp->SetEvalType(T_DYNAMIC);
            return T_DYNAMIC;
        }

        if (op->GetOp() == "PLUS") {
            if (left_type == T_INT && right_type == T_INT) {
                exp->SetEvalType(T_INT);
                return T_INT;
            }
            if ((left_type == T_INT || left_type == T_FLOAT) && (right_type == T_INT || right_type == T_FLOAT)) {
                exp->SetEvalType(T_FLOAT);
                return T_FLOAT;
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
        ret = InferNode(prefix_exp->GetValue());
    } else if (prefix_type == "functioncall") {
        ret = T_DYNAMIC;
        InferNode(prefix_exp->GetValue());
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
        InferNode(pe);
    }
    if (var->GetType() == "square") {
        if (const auto exp = var->GetExp()) {
            InferNode(exp);
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

}// namespace fakelua
