#include "compile/func_body_compiler.h"

#include <ranges>

#include "state/state.h"
#include "util/common.h"
#include "util/exception.h"

namespace fakelua {

FuncBodyCompiler::FuncBodyCompiler(State *s) : s_(s) {
}

void FuncBodyCompiler::SetContext(
        const std::string *file_name,
        const std::unordered_map<std::string, int> *local_func_names,
        const std::unordered_set<std::string> *global_const_vars,
        bool *in_global_init,
        int *tmp_var_counter,
        const std::unordered_map<std::string, std::vector<int>> *math_param_positions,
        const std::unordered_map<std::string, std::vector<EvalTypeSnapshot>> *specialization_snapshots,
        const std::unordered_map<std::string, std::vector<InferredType>> *specialization_return_types,
        const EvalTypeSnapshot *main_eval_types) {
    file_name_ = file_name;
    local_func_names_ = local_func_names;
    global_const_vars_ = global_const_vars;
    in_global_init_ = in_global_init;
    tmp_var_counter_ = tmp_var_counter;
    math_param_positions_ = math_param_positions;
    specialization_snapshots_ = specialization_snapshots;
    specialization_return_types_ = specialization_return_types;
    main_eval_types_ = main_eval_types;
}

[[noreturn]] void FuncBodyCompiler::ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr) const {
    ThrowFakeluaException(
            std::format("Code generate failed, {} at {}:{}:{}", msg, *file_name_, ptr->Loc().begin.line, ptr->Loc().begin.column));
}

std::string FuncBodyCompiler::GenTab() const {
    const auto tab_size = static_cast<size_t>(cur_tab_) * 4;
    std::string tabs(tab_size, ' ');
    return tabs;
}

InferredType FuncBodyCompiler::LookupNodeType(SyntaxTreeInterface *node) const {
    if (cur_spec_snapshot_) {
        if (const auto it = cur_spec_snapshot_->find(node); it != cur_spec_snapshot_->end()) {
            return it->second;
        }
    }
    if (main_eval_types_) {
        if (const auto it = main_eval_types_->find(node); it != main_eval_types_->end()) {
            return it->second;
        }
    }
    return T_UNKNOWN;
}

InferredType FuncBodyCompiler::GetSpecReturnType(const std::string &func_name, int bitmask) const {
    if (!specialization_return_types_) {
        return T_DYNAMIC;
    }
    const auto it = specialization_return_types_->find(func_name);
    if (it == specialization_return_types_->end()) {
        return T_DYNAMIC;
    }
    if (bitmask < 0 || bitmask >= static_cast<int>(it->second.size())) {
        return T_DYNAMIC;
    }
    return it->second[static_cast<size_t>(bitmask)];
}

void FuncBodyCompiler::CompileFuncBody(const std::string &func_name,
                                        const std::vector<std::string> &func_params,
                                        const SyntaxTreeInterfacePtr &func_block,
                                        int spec_bitmask,
                                        std::ostream &out) {
    // 初始化特化上下文。
    spec_param_types_.clear();
    cur_spec_bitmask_ = spec_bitmask;
    cur_spec_func_name_ = (spec_bitmask >= 0) ? func_name : "";
    cur_spec_snapshot_ = nullptr;

    if (spec_bitmask >= 0) {
        const auto &math_params = math_param_positions_->at(func_name);
        for (int i = 0; i < static_cast<int>(math_params.size()); ++i) {
            const auto &param_name = func_params[static_cast<size_t>(math_params[i])];
            spec_param_types_[param_name] =
                    (MathParamKindOf(spec_bitmask, i) == kMathParamFloat) ? T_FLOAT : T_INT;
        }
        // 查找当前位掩码对应的快照，以便 CompileStmtLocalVar / CompileVar
        // 在此参数类型组合下能查询到每个 AST 节点的正确类型。
        if (specialization_snapshots_) {
            if (const auto snap_it = specialization_snapshots_->find(func_name);
                snap_it != specialization_snapshots_->end()) {
                const auto &snaps = snap_it->second;
                if (spec_bitmask < static_cast<int>(snaps.size())) {
                    cur_spec_snapshot_ = &snaps[static_cast<size_t>(spec_bitmask)];
                }
            }
        }
    }

    // 将函数体编译到 body 缓冲区。
    func_temp_decls_.str("");
    func_temp_decls_.clear();
    body_ss_.str("");
    body_ss_.clear();
    native_var_scope_.Clear();
    EnterNativeVarScope();
    for (const auto &param_name: func_params) {
        // 在特化模式中，数学参数已在函数签名中声明为原生类型（int64_t/double），
        // 因此将其注册为对应的原生类型，以便 CompileStmtAssign / InferArgTypeForSpec
        // 能正确处理对这些参数的引用和赋值。
        // 非特化或非数学参数仍注册为 T_DYNAMIC（CVar）。
        const InferredType param_native_type =
                (spec_bitmask >= 0 && spec_param_types_.contains(param_name))
                        ? spec_param_types_.at(param_name)
                        : T_DYNAMIC;
        DeclareNativeVar(param_name, param_native_type);
    }
    cur_output_ = &body_ss_;
    cur_tab_++;
    CompileStmtBlock(func_block);
    cur_tab_--;
    ExitNativeVarScope();

    // 写入调用方提供的输出流。
    out << func_temp_decls_.str();
    out << body_ss_.str();

    // 清除特化上下文。
    spec_param_types_.clear();
    cur_spec_bitmask_ = -1;
    cur_spec_func_name_ = "";
    cur_spec_snapshot_ = nullptr;
    cur_output_ = nullptr;
}

bool FuncBodyCompiler::TryInferMathCallBitmask(const std::string &callee_name,
                                               const std::vector<SyntaxTreeInterfacePtr> &raw_args,
                                               int &bitmask) const {
    const auto math_it = math_param_positions_->find(callee_name);
    if (math_it == math_param_positions_->end()) {
        return false;
    }
    const auto &math_params = math_it->second;
    bitmask = 0;
    for (int i = 0; i < static_cast<int>(math_params.size()); ++i) {
        const int param_pos = math_params[i];
        if (param_pos >= static_cast<int>(raw_args.size())) {
            return false;
        }
        const auto &arg = raw_args[static_cast<size_t>(param_pos)];
        if (!arg || arg->Type() != SyntaxTreeType::Exp) {
            return false;
        }
        const auto arg_type = InferArgTypeForSpec(arg);
        if (arg_type == T_DYNAMIC) {
            return false;
        }
        if (arg_type == T_FLOAT) {
            bitmask |= (1 << i);
        }
    }
    return true;
}

bool FuncBodyCompiler::TryInferMathCallSpec(const std::string &callee_name,
                                            const std::vector<SyntaxTreeInterfacePtr> &raw_args,
                                            int &bitmask,
                                            InferredType &spec_ret) const {
    if (!TryInferMathCallBitmask(callee_name, raw_args, bitmask)) {
        return false;
    }
    spec_ret = GetSpecReturnType(callee_name, bitmask);
    return true;
}

InferredType FuncBodyCompiler::InferArgTypeForSpec(const SyntaxTreeInterfacePtr &exp) const {
    DEBUG_ASSERT(exp && exp->Type() == SyntaxTreeType::Exp);
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto exp_kind = e->GetExpKind();

    if (exp_kind == ExpKind::kNumber) {
        // 使用 LookupNodeType 查询 TypeInferencer 的推断结果（特化快照或主推断）。
        const auto node_type = LookupNodeType(e.get());
        if (node_type == T_INT) {
            return T_INT;
        }
        if (node_type == T_FLOAT) {
            return T_FLOAT;
        }
        // 回退：探查字符串值。
        const auto &val = e->ExpValue();
        if (val.find('.') == std::string::npos && val.find('e') == std::string::npos &&
            val.find('E') == std::string::npos) {
            return T_INT;
        }
        return T_FLOAT;
    }

    if (exp_kind == ExpKind::kPrefixExp) {
        const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e->Right());
        DEBUG_ASSERT(pe);
        if (pe->GetPrefixKind() == PrefixExpKind::kVar) {
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue());
            if (!var || var->GetVarKind() != VarKind::kSimple) {
                return T_DYNAMIC;
            }
            const auto &vname = var->GetName();
            // 特化上下文优先。
            if (const auto it = spec_param_types_.find(vname); it != spec_param_types_.end()) {
                return it->second;
            }
            // 使用 GetNativeVarType 获取变量声明时的实际 C 类型，而不依赖 EvalType。
            const auto native_type = GetNativeVarType(vname);
            if (native_type == T_INT || native_type == T_FLOAT) {
                return native_type;
            }
            return T_DYNAMIC;
        }
        if (pe->GetPrefixKind() == PrefixExpKind::kExp) {
            return InferArgTypeForSpec(pe->GetValue());
        }
        if (pe->GetPrefixKind() == PrefixExpKind::kFunctionCall) {
            // Check whether the callee is a local function with math params.
            const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(pe->GetValue());
            if (!fc) {
                return T_DYNAMIC;
            }
            const auto callee_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
            if (!callee_pe || callee_pe->GetPrefixKind() != PrefixExpKind::kVar) {
                return T_DYNAMIC;
            }
            const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(callee_pe->GetValue());
            if (!callee_var || callee_var->GetVarKind() != VarKind::kSimple) {
                return T_DYNAMIC;
            }
            const auto &callee_name = callee_var->GetName();
            const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
            if (!args_ptr) {
                return T_DYNAMIC;
            }
            const auto raw_args = ExtractCallRawArgs(args_ptr);
            if (raw_args.empty()) {
                return T_DYNAMIC;
            }
            int bitmask = 0;
            InferredType spec_ret = T_DYNAMIC;
            if (!TryInferMathCallSpec(callee_name, raw_args, bitmask, spec_ret)) {
                return T_DYNAMIC;
            }
            return spec_ret;
        }
        return T_DYNAMIC;
    }

    if (exp_kind == ExpKind::kBinop) {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(e->Op());
        DEBUG_ASSERT(op);
        const auto lt = InferArgTypeForSpec(e->Left());
        const auto rt = InferArgTypeForSpec(e->Right());
        return InferNumericBinopResultType(op->GetOpKind(), lt, rt);
    }

    if (exp_kind == ExpKind::kUnop) {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(e->Op());
        DEBUG_ASSERT(op);
        if (op->GetOpKind() == UnOpKind::kMinus) {
            return InferArgTypeForSpec(e->Right());
        }
        if (op->GetOpKind() == UnOpKind::kBitNot) {
            const auto t = InferArgTypeForSpec(e->Right());
            if (t == T_INT) {
                return T_INT;
            }
            return T_DYNAMIC;
        }
        if (op->GetOpKind() == UnOpKind::kNumberSign) {
            // # 运算符始终返回整数（字符串长度或表大小）。
            return T_INT;
        }
        return T_DYNAMIC;
    }

    return T_DYNAMIC;
}

std::string FuncBodyCompiler::TryCompileNativeExpr(const SyntaxTreeInterfacePtr &exp) {
    try {
        return CompileNumericExp(exp);
    } catch (...) {
        return {};
    }
}

// 将比较运算符映射到对应的 C 操作符。
static const std::unordered_map<BinOpKind, std::string_view> kCmpOpMap = {
        {BinOpKind::kLess, "<"}, {BinOpKind::kLessEqual, "<="}, {BinOpKind::kMore, ">"},
        {BinOpKind::kMoreEqual, ">="}, {BinOpKind::kEqual, "=="}, {BinOpKind::kNotEqual, "!="}
};

std::string FuncBodyCompiler::TryCompileNativeBoolExpr(const SyntaxTreeInterfacePtr &exp) {
    // 只处理 Exp 节点。
    if (!exp || exp->Type() != SyntaxTreeType::Exp) {
        return {};
    }
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);

    // 透明地解包括括号表达式：(expr) → expr。
    if (e->GetExpKind() == ExpKind::kPrefixExp) {
        const auto pexp = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e->Right());
        if (!pexp || pexp->GetPrefixKind() != PrefixExpKind::kExp) {
            return {};
        }
        return TryCompileNativeBoolExpr(pexp->GetValue());
    }

    // 处理 not 一元逻辑取反：将 not <bool_expr> 编译为 !(<bool_expr>)。
    if (e->GetExpKind() == ExpKind::kUnop) {
        const auto unop = std::dynamic_pointer_cast<SyntaxTreeUnop>(e->Op());
        if (!unop || unop->GetOpKind() != UnOpKind::kNot) {
            return {};
        }
        const auto inner = TryCompileNativeBoolExpr(e->Right());
        if (inner.empty()) {
            return {};
        }
        return std::format("!({})", inner);
    }

    if (e->GetExpKind() != ExpKind::kBinop) {
        return {};
    }
    const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(e->Op());
    if (!op) {
        return {};
    }
    const auto op_kind = op->GetOpKind();

    // 处理 and/or 逻辑运算符：递归将两侧编译为原生布尔表达式。
    if (op_kind == BinOpKind::kAnd || op_kind == BinOpKind::kOr) {
        const auto left_bool = TryCompileNativeBoolExpr(e->Left());
        const auto right_bool = TryCompileNativeBoolExpr(e->Right());
        if (left_bool.empty() || right_bool.empty()) {
            return {};
        }
        const auto c_op = (op_kind == BinOpKind::kAnd) ? "&&" : "||";
        return std::format("({}) {} ({})", left_bool, c_op, right_bool);
    }

    const auto op_it = kCmpOpMap.find(op_kind);
    if (op_it == kCmpOpMap.end()) {
        return {};
    }
    const auto left_type = e->Left() ? InferArgTypeForSpec(e->Left()) : T_DYNAMIC;
    const auto right_type = e->Right() ? InferArgTypeForSpec(e->Right()) : T_DYNAMIC;
    if ((left_type != T_INT && left_type != T_FLOAT) || (right_type != T_INT && right_type != T_FLOAT)) {
        return {};
    }
    const auto left_native = TryCompileNativeExpr(e->Left());
    const auto right_native = TryCompileNativeExpr(e->Right());
    if (left_native.empty() || right_native.empty()) {
        return {};
    }
    return std::format("({}) {} ({})", left_native, op_it->second, right_native);
}

void FuncBodyCompiler::CompileStmtBlock(const SyntaxTreeInterfacePtr &block) {
    DEBUG_ASSERT(block->Type() == SyntaxTreeType::Block);
    const auto block_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(block);

    for (const auto &stmts = block_ptr->Stmts(); auto &stmt: stmts) {
        CompileStmt(stmt);
    }
}

void FuncBodyCompiler::CompileStmt(const SyntaxTreeInterfacePtr &stmt) {
    switch (stmt->Type()) {
        case SyntaxTreeType::Return: {
            CompileStmtReturn(stmt);
            break;
        }
        case SyntaxTreeType::LocalVar: {
            CompileStmtLocalVar(stmt);
            break;
        }
        case SyntaxTreeType::Assign: {
            CompileStmtAssign(stmt);
            break;
        }
        case SyntaxTreeType::FunctionCall: {
            CompileStmtFunctioncall(stmt);
            break;
        }
        case SyntaxTreeType::Label: {
            // PreProcessor 已确保不存在 label
            DEBUG_ASSERT(false && "label should have been caught by PreProcessor");
            break;
        }
        case SyntaxTreeType::Goto: {
            // PreProcessor 已确保不存在 goto
            DEBUG_ASSERT(false && "goto should have been caught by PreProcessor");
            break;
        }
        case SyntaxTreeType::Block: {
            // do...end 块：发出一个 C 复合语句，使内部的 `local`
            // 声明遮蔽外部变量而不是重新声明它们。
            *cur_output_ << GenTab() << "{\n";
            cur_tab_++;
            CompileScopedBlock(stmt);
            cur_tab_--;
            *cur_output_ << GenTab() << "}\n";
            break;
        }
        case SyntaxTreeType::While: {
            CompileStmtWhile(stmt);
            break;
        }
        case SyntaxTreeType::Repeat: {
            CompileStmtRepeat(stmt);
            break;
        }
        case SyntaxTreeType::If: {
            CompileStmtIf(stmt);
            break;
        }
        case SyntaxTreeType::Break: {
            CompileStmtBreak(stmt);
            break;
        }
        case SyntaxTreeType::ForLoop: {
            CompileStmtForLoop(stmt);
            break;
        }
        case SyntaxTreeType::ForIn: {
            CompileStmtForIn(stmt);
            break;
        }
        case SyntaxTreeType::Empty: {
            // Semicolons produce empty statements; they are no-ops in Lua.
            break;
        }
        default: {
            ThrowError(std::format("not support stmt type: {}", SyntaxTreeTypeToString(stmt->Type())), stmt);
        }
    }
}

void FuncBodyCompiler::CompileStmtReturn(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::Return);
    const auto return_stmt = std::dynamic_pointer_cast<SyntaxTreeReturn>(stmt);

    auto explist = return_stmt->Explist();
    if (!explist) {
        // 默认返回 nil
        explist = std::make_shared<SyntaxTreeExplist>(return_stmt->Loc());
        const auto exp = std::make_shared<SyntaxTreeExp>(return_stmt->Loc());
        exp->SetExpKind(ExpKind::kNil);
        std::dynamic_pointer_cast<SyntaxTreeExplist>(explist)->AddExp(exp);
    }

    const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
    if (explist_ptr->Exps().empty()) {
        // 默认返回 nil
        const auto exp = std::make_shared<SyntaxTreeExp>(return_stmt->Loc());
        exp->SetExpKind(ExpKind::kNil);
        explist_ptr->AddExp(exp);
    } else {
        // PreProcessor 已确保不存在多返回值
        DEBUG_ASSERT(explist_ptr->Exps().size() == 1);
    }

    const auto exp = explist_ptr->Exps()[0];
    // 若当前处于原生返回类型的特化函数中，直接将返回表达式编译为原生数值并返回，
    // 跳过 CompileExp 的装箱步骤，消除一次 CVar 封箱拆箱开销。
    if (cur_spec_bitmask_ >= 0 && !cur_spec_func_name_.empty()) {
        const auto spec_ret = GetSpecReturnType(cur_spec_func_name_, cur_spec_bitmask_);
        if (spec_ret == T_INT || spec_ret == T_FLOAT) {
            const auto native_ret = CompileNumericExp(exp);
            *cur_output_ << GenTab() << "return " << native_ret << ";\n";
            return;
        }
    }
    // 始终通过 CompileExp 编译返回表达式。
    const std::string ret = CompileExp(exp);
    *cur_output_ << GenTab() << "return " << ret << ";\n";
}

void FuncBodyCompiler::CompileStmtLocalVar(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::LocalVar);
    const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
    const auto namelist = local_var->Namelist();

    if (!namelist) {
        return;
    }

    DEBUG_ASSERT(namelist->Type() == SyntaxTreeType::NameList);
    const auto namelist_ptr = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist);
    const auto &names = namelist_ptr->Names();

    std::vector<SyntaxTreeInterfacePtr> exps;
    if (const auto explist = local_var->Explist()) {
        DEBUG_ASSERT(explist->Type() == SyntaxTreeType::ExpList);
        const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
        exps = explist_ptr->Exps();
    }

    for (size_t i = 0; i < names.size(); ++i) {
        const auto &name = names[i];

        if (global_const_vars_->contains(name)) {
            ThrowError("local variable conflicts with global constant: " + name, stmt);
        }

        if (i < exps.size() && LookupNodeType(exps[i].get()) == T_INT) {
            const auto native_expr = CompileNumericExp(exps[i]);
            if (IsTypedNativeVar(name)) {
                const auto tmp = std::format("flua_local_{}", (*tmp_var_counter_)++);
                func_temp_decls_ << "    int64_t " << tmp << ";\n";
                *cur_output_ << GenTab() << tmp << " = " << native_expr << ";\n";
                *cur_output_ << GenTab() << "int64_t " << name << " = " << tmp << ";\n";
            } else {
                *cur_output_ << GenTab() << "int64_t " << name << " = " << native_expr << ";\n";
            }
            DeclareNativeVar(name, T_INT);
        } else if (i < exps.size() && LookupNodeType(exps[i].get()) == T_FLOAT) {
            const auto native_expr = CompileNumericExp(exps[i]);
            if (IsTypedNativeVar(name)) {
                const auto tmp = std::format("flua_local_{}", (*tmp_var_counter_)++);
                func_temp_decls_ << "    double " << tmp << ";\n";
                *cur_output_ << GenTab() << tmp << " = " << native_expr << ";\n";
                *cur_output_ << GenTab() << "double " << name << " = " << tmp << ";\n";
            } else {
                *cur_output_ << GenTab() << "double " << name << " = " << native_expr << ";\n";
            }
            DeclareNativeVar(name, T_FLOAT);
        } else if (i < exps.size()) {
            // EvalType 为 T_DYNAMIC：尝试通过 InferArgTypeForSpec 捕获数学函数调用返回值。
            const auto init_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(exps[i]);
            const bool is_degraded_literal =
                    init_exp && init_exp->GetExpKind() == ExpKind::kNumber && LookupNodeType(exps[i].get()) == T_DYNAMIC;
            if (!is_degraded_literal) {
                const auto spec_type = InferArgTypeForSpec(exps[i]);
                if (spec_type == T_INT || spec_type == T_FLOAT) {
                    const auto native_expr = TryCompileNativeExpr(exps[i]);
                    if (!native_expr.empty()) {
                        const auto c_type = (spec_type == T_INT) ? "int64_t" : "double";
                        if (IsTypedNativeVar(name)) {
                            const auto tmp = std::format("flua_local_{}", (*tmp_var_counter_)++);
                            func_temp_decls_ << "    " << c_type << " " << tmp << ";\n";
                            *cur_output_ << GenTab() << tmp << " = " << native_expr << ";\n";
                            *cur_output_ << GenTab() << c_type << " " << name << " = " << tmp << ";\n";
                        } else {
                            *cur_output_ << GenTab() << c_type << " " << name << " = " << native_expr << ";\n";
                        }
                        DeclareNativeVar(name, spec_type);
                        continue;
                    }
                }
            }
            const std::string init = CompileExp(exps[i]);
            *cur_output_ << GenTab() << "CVar " << name << " = " << init << ";\n";
            DeclareNativeVar(name, T_DYNAMIC);
        } else {
            *cur_output_ << GenTab() << "CVar " << name << " = kNil;\n";
            DeclareNativeVar(name, T_DYNAMIC);
        }
    }
}

void FuncBodyCompiler::CompileStmtAssign(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::Assign);
    const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);

    const auto varlist = assign->Varlist();
    DEBUG_ASSERT(varlist->Type() == SyntaxTreeType::VarList);
    const auto varlist_ptr = std::dynamic_pointer_cast<SyntaxTreeVarlist>(varlist);
    const auto &vars = varlist_ptr->Vars();

    std::vector<SyntaxTreeInterfacePtr> exps;
    if (const auto explist = assign->Explist()) {
        DEBUG_ASSERT(explist->Type() == SyntaxTreeType::ExpList);
        const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
        exps = explist_ptr->Exps();
    }

    // PreprocessSplitAssign 保证此时恰好有 1 个变量和 1 个表达式
    DEBUG_ASSERT(vars.size() == 1);
    DEBUG_ASSERT(exps.size() == 1);

    DEBUG_ASSERT(vars[0]->Type() == SyntaxTreeType::Var);
    const auto v_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(vars[0]);

    // PreprocessTableAssign 将方括号/点号赋值重写为 FAKELUA_SET_TABLE 调用，
    // 所以只有简单变量赋值能到达此处。
    DEBUG_ASSERT(v_ptr->GetVarKind() == VarKind::kSimple);
    if (const auto &name = v_ptr->GetName(); IsTypedNativeVar(name)) {
        const auto var_type = GetNativeVarType(name);
        const auto native_rhs = TryCompileNativeExpr(exps[0]);
        if (!native_rhs.empty()) {
            const auto rhs_type = InferArgTypeForSpec(exps[0]);
            if (rhs_type == T_FLOAT && var_type == T_INT) {
                *cur_output_ << GenTab() << name << " = (int64_t)(" << native_rhs << ");\n";
            } else if (rhs_type == T_INT && var_type == T_FLOAT) {
                *cur_output_ << GenTab() << name << " = (double)(" << native_rhs << ");\n";
            } else {
                *cur_output_ << GenTab() << name << " = " << native_rhs << ";\n";
            }
        } else {
            const std::string rhs = CompileExp(exps[0]);
            const auto tmp = std::format("flua_assign_tmp_{}", (*tmp_var_counter_)++);
            func_temp_decls_ << "    CVar " << tmp << ";\n";
            *cur_output_ << GenTab() << tmp << " = " << rhs << ";\n";
            if (var_type == T_FLOAT) {
                *cur_output_ << GenTab() << name << " = (" << tmp << ".type_ == VAR_FLOAT) ? " << tmp << ".data_.f : (double)" << tmp
                             << ".data_.i;\n";
            } else {
                *cur_output_ << GenTab() << name << " = (" << tmp << ".type_ == VAR_INT) ? " << tmp << ".data_.i : (int64_t)" << tmp
                             << ".data_.f;\n";
            }
        }
    } else {
        const std::string rhs = CompileExp(exps[0]);
        *cur_output_ << GenTab() << name << " = " << rhs << ";\n";
    }
}

void FuncBodyCompiler::CompileStmtFunctioncall(const SyntaxTreeInterfacePtr &stmt) {
    CompileFunctioncall(stmt);
}

void FuncBodyCompiler::CompileScopedBlock(const SyntaxTreeInterfacePtr &block) {
    EnterNativeVarScope();
    CompileStmtBlock(block);
    ExitNativeVarScope();
}

std::string FuncBodyCompiler::CompileCondBoolExpr(const SyntaxTreeInterfacePtr &exp, const std::string &tmp_prefix) {
    auto native_cond = TryCompileNativeBoolExpr(exp);
    if (!native_cond.empty()) {
        return native_cond;
    }
    const auto tmp_bool = std::format("{}_{}", tmp_prefix, (*tmp_var_counter_)++);
    func_temp_decls_ << "    bool " << tmp_bool << ";\n";
    const auto cond = CompileExp(exp);
    *cur_output_ << GenTab() << std::format("IsTrue(({}), {});\n", cond, tmp_bool);
    return tmp_bool;
}

void FuncBodyCompiler::CompileStmtWhile(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::While);
    const auto while_stmt = std::dynamic_pointer_cast<SyntaxTreeWhile>(stmt);

    const auto native_cond = TryCompileNativeBoolExpr(while_stmt->Exp());
    if (!native_cond.empty()) {
        *cur_output_ << GenTab() << "while (" << native_cond << ") {\n";
        cur_tab_++;
        CompileScopedBlock(while_stmt->Block());
        cur_tab_--;
        *cur_output_ << GenTab() << "}\n";
        return;
    }

    const auto tmp_bool = std::format("flua_wbt_{}", (*tmp_var_counter_)++);
    func_temp_decls_ << "    bool " << tmp_bool << ";\n";
    *cur_output_ << GenTab() << "while (1) {\n";
    cur_tab_++;
    const auto cond = CompileExp(while_stmt->Exp());
    *cur_output_ << GenTab() << std::format("IsTrue(({}), {});\n", cond, tmp_bool);
    *cur_output_ << GenTab() << std::format("if (!{}) break;\n", tmp_bool);
    CompileScopedBlock(while_stmt->Block());
    cur_tab_--;
    *cur_output_ << GenTab() << "}\n";
}

void FuncBodyCompiler::CompileStmtRepeat(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::Repeat);
    const auto repeat_stmt = std::dynamic_pointer_cast<SyntaxTreeRepeat>(stmt);

    *cur_output_ << GenTab() << "do {\n";
    cur_tab_++;

    CompileScopedBlock(repeat_stmt->Block());
    const auto cond_bool = CompileCondBoolExpr(repeat_stmt->Exp(), "flua_rbt");
    *cur_output_ << GenTab() << std::format("if ({}) break;\n", cond_bool);

    cur_tab_--;
    *cur_output_ << GenTab() << "} while (1);\n";
}

void FuncBodyCompiler::CompileStmtIf(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::If);
    const auto if_stmt = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);

    const auto cond_bool = CompileCondBoolExpr(if_stmt->Exp(), "flua_ibt");
    *cur_output_ << GenTab() << std::format("if ({}) {{\n", cond_bool);
    cur_tab_++;
    CompileScopedBlock(if_stmt->Block());
    cur_tab_--;
    *cur_output_ << GenTab() << "}";

    int elseif_depth = 0;
    if (const auto elseifs_node = if_stmt->ElseIfs()) {
        const auto elseif_list = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseifs_node);
        for (size_t i = 0; i < elseif_list->ElseifSize(); ++i) {
            *cur_output_ << " else {\n";
            cur_tab_++;
            elseif_depth++;

            const auto econd_bool = CompileCondBoolExpr(elseif_list->ElseifExp(i), "flua_ibt");
            *cur_output_ << GenTab() << std::format("if ({}) {{\n", econd_bool);
            cur_tab_++;
            CompileScopedBlock(elseif_list->ElseifBlock(i));
            cur_tab_--;
            *cur_output_ << GenTab() << "}";
        }
    }

    if (const auto else_block = if_stmt->ElseBlock()) {
        *cur_output_ << " else {\n";
        cur_tab_++;
        CompileScopedBlock(else_block);
        cur_tab_--;
        *cur_output_ << GenTab() << "}";
    }

    for (int i = 0; i < elseif_depth; ++i) {
        *cur_output_ << "\n";
        cur_tab_--;
        *cur_output_ << GenTab() << "}";
    }

    *cur_output_ << "\n";
}

void FuncBodyCompiler::CompileStmtBreak(const SyntaxTreeInterfacePtr &stmt) {
    *cur_output_ << GenTab() << "break;\n";
}

void FuncBodyCompiler::CompileStmtForLoop(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::ForLoop);
    const auto for_stmt = std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt);

    const bool typed_int_for = for_stmt->ExpBegin() && for_stmt->ExpEnd() && LookupNodeType(for_stmt->ExpBegin().get()) == T_INT &&
                               LookupNodeType(for_stmt->ExpEnd().get()) == T_INT &&
                               (!for_stmt->ExpStep() || LookupNodeType(for_stmt->ExpStep().get()) == T_INT);
    if (typed_int_for) {
        CompileTypedIntForLoop(for_stmt);
        return;
    }

    // T_FLOAT 快路径：所有边界为数值（T_INT 或 T_FLOAT）但并非全为 T_INT。
    const bool step_is_numeric = !for_stmt->ExpStep() || LookupNodeType(for_stmt->ExpStep().get()) == T_INT ||
                                 LookupNodeType(for_stmt->ExpStep().get()) == T_FLOAT;
    const bool typed_float_for =
            !typed_int_for && for_stmt->ExpBegin() && for_stmt->ExpEnd() &&
            (LookupNodeType(for_stmt->ExpBegin().get()) == T_INT || LookupNodeType(for_stmt->ExpBegin().get()) == T_FLOAT) &&
            (LookupNodeType(for_stmt->ExpEnd().get()) == T_INT || LookupNodeType(for_stmt->ExpEnd().get()) == T_FLOAT) && step_is_numeric;
    if (typed_float_for) {
        CompileTypedFloatForLoop(for_stmt);
        return;
    }

    CompileDynamicForLoop(for_stmt);
}

void FuncBodyCompiler::CompileTypedIntForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_stmt) {
    const auto ctrl_var = std::format("flua_for_ctrl_{}", (*tmp_var_counter_)++);
    const auto end_var = std::format("flua_for_end_{}", (*tmp_var_counter_)++);
    const auto step_var = std::format("flua_for_step_{}", (*tmp_var_counter_)++);

    func_temp_decls_ << "    int64_t " << ctrl_var << ";\n";
    func_temp_decls_ << "    int64_t " << end_var << ";\n";
    func_temp_decls_ << "    int64_t " << step_var << ";\n";

    const auto native_begin = CompileNumericExp(for_stmt->ExpBegin());
    *cur_output_ << GenTab() << ctrl_var << " = " << native_begin << ";\n";
    const auto native_end = CompileNumericExp(for_stmt->ExpEnd());
    *cur_output_ << GenTab() << end_var << " = " << native_end << ";\n";
    if (for_stmt->ExpStep()) {
        if (const auto step_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(for_stmt->ExpStep());
            step_exp && step_exp->GetExpKind() == ExpKind::kNumber && LookupNodeType(step_exp.get()) == T_INT &&
            ToInteger(step_exp->ExpValue()) == 0) {
            ThrowError("'for' step is zero", for_stmt->ExpStep());
        }
        const auto native_step = CompileNumericExp(for_stmt->ExpStep());
        *cur_output_ << GenTab() << step_var << " = " << native_step << ";\n";
    } else {
        *cur_output_ << GenTab() << step_var << " = 1;\n";
    }
    *cur_output_ << GenTab() << "if (" << step_var << " == 0) { FakeluaThrowError(_S, \"'for' step is zero\"); }\n";

    *cur_output_ << GenTab() << "for (; (" << step_var << " > 0) ? (" << ctrl_var << " <= " << end_var << ") : (" << ctrl_var
                 << " >= " << end_var << "); " << ctrl_var << " += " << step_var << ") {\n";
    cur_tab_++;
    EnterNativeVarScope();
    if (LookupNodeType(for_stmt.get()) == T_INT) {
        *cur_output_ << GenTab() << "int64_t " << for_stmt->Name() << " = " << ctrl_var << ";\n";
        DeclareNativeVar(for_stmt->Name(), T_INT);
    } else {
        *cur_output_ << GenTab() << "CVar " << for_stmt->Name() << " = " << BoxNativeValue(ctrl_var, T_INT) << ";\n";
        DeclareNativeVar(for_stmt->Name(), T_DYNAMIC);
    }
    // 用内层作用域包裹循环体，避免 local 同名变量与循环变量在同一 C 作用域中重复声明。
    *cur_output_ << GenTab() << "{\n";
    cur_tab_++;
    EnterNativeVarScope();
    CompileStmtBlock(for_stmt->Block());
    ExitNativeVarScope();
    cur_tab_--;
    *cur_output_ << GenTab() << "}\n";
    ExitNativeVarScope();
    cur_tab_--;
    *cur_output_ << GenTab() << "}\n";
}

void FuncBodyCompiler::CompileTypedFloatForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_stmt) {
    const auto ctrl_var = std::format("flua_for_ctrl_{}", (*tmp_var_counter_)++);
    const auto end_var = std::format("flua_for_end_{}", (*tmp_var_counter_)++);
    const auto step_var = std::format("flua_for_step_{}", (*tmp_var_counter_)++);

    func_temp_decls_ << "    double " << ctrl_var << ";\n";
    func_temp_decls_ << "    double " << end_var << ";\n";
    func_temp_decls_ << "    double " << step_var << ";\n";

    const auto native_begin = CompileNumericExp(for_stmt->ExpBegin());
    *cur_output_ << GenTab() << ctrl_var << " = (double)(" << native_begin << ");\n";
    const auto native_end = CompileNumericExp(for_stmt->ExpEnd());
    *cur_output_ << GenTab() << end_var << " = (double)(" << native_end << ");\n";
    if (for_stmt->ExpStep()) {
        if (const auto step_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(for_stmt->ExpStep());
            step_exp && step_exp->GetExpKind() == ExpKind::kNumber) {
            const double step_val = (LookupNodeType(step_exp.get()) == T_INT)
                                            ? static_cast<double>(ToInteger(step_exp->ExpValue()))
                                            : ToFloat(step_exp->ExpValue());
            if (step_val == 0.0) {
                ThrowError("'for' step is zero", for_stmt->ExpStep());
            }
        }
        const auto native_step = CompileNumericExp(for_stmt->ExpStep());
        *cur_output_ << GenTab() << step_var << " = (double)(" << native_step << ");\n";
    } else {
        *cur_output_ << GenTab() << step_var << " = 1.0;\n";
    }
    *cur_output_ << GenTab() << "if (" << step_var << " == 0.0) { FakeluaThrowError(_S, \"'for' step is zero\"); }\n";

    *cur_output_ << GenTab() << "for (; (" << step_var << " > 0.0) ? (" << ctrl_var << " <= " << end_var << ") : (" << ctrl_var
                 << " >= " << end_var << "); " << ctrl_var << " += " << step_var << ") {\n";
    cur_tab_++;
    EnterNativeVarScope();
    if (LookupNodeType(for_stmt.get()) == T_FLOAT) {
        *cur_output_ << GenTab() << "double " << for_stmt->Name() << " = " << ctrl_var << ";\n";
        DeclareNativeVar(for_stmt->Name(), T_FLOAT);
    } else {
        *cur_output_ << GenTab() << "CVar " << for_stmt->Name() << " = " << BoxNativeValue(ctrl_var, T_FLOAT) << ";\n";
        DeclareNativeVar(for_stmt->Name(), T_DYNAMIC);
    }
    // 用内层作用域包裹循环体，避免 local 同名变量与循环变量在同一 C 作用域中重复声明。
    *cur_output_ << GenTab() << "{\n";
    cur_tab_++;
    EnterNativeVarScope();
    CompileStmtBlock(for_stmt->Block());
    ExitNativeVarScope();
    cur_tab_--;
    *cur_output_ << GenTab() << "}\n";
    ExitNativeVarScope();
    cur_tab_--;
    *cur_output_ << GenTab() << "}\n";
}

void FuncBodyCompiler::CompileDynamicForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_stmt) {
    const auto ctrl_var = std::format("flua_for_ctrl_{}", (*tmp_var_counter_)++);
    const auto end_var = std::format("flua_for_end_{}", (*tmp_var_counter_)++);
    const auto step_var = std::format("flua_for_step_{}", (*tmp_var_counter_)++);
    const auto step_pos_var = std::format("flua_for_step_pos_{}", (*tmp_var_counter_)++);
    const auto cond_var = std::format("flua_for_cond_{}", (*tmp_var_counter_)++);
    const auto cmp_var = std::format("flua_for_cmp_{}", (*tmp_var_counter_)++);

    func_temp_decls_ << "    CVar " << ctrl_var << ";\n";
    func_temp_decls_ << "    CVar " << end_var << ";\n";
    func_temp_decls_ << "    CVar " << step_var << ";\n";
    func_temp_decls_ << "    bool " << step_pos_var << ";\n";
    func_temp_decls_ << "    bool " << cond_var << ";\n";
    func_temp_decls_ << "    CVar " << cmp_var << ";\n";

    const auto begin_expr = CompileExp(for_stmt->ExpBegin());
    *cur_output_ << GenTab() << ctrl_var << " = " << begin_expr << ";\n";

    const auto end_expr = CompileExp(for_stmt->ExpEnd());
    *cur_output_ << GenTab() << end_var << " = " << end_expr << ";\n";

    if (for_stmt->ExpStep()) {
        const auto step_expr = CompileExp(for_stmt->ExpStep());
        *cur_output_ << GenTab() << step_var << " = " << step_expr << ";\n";
    } else {
        *cur_output_ << GenTab() << "SET_INT(" << step_var << ", 1);\n";
    }

    *cur_output_ << GenTab() << "if (" << step_var << ".type_ == VAR_INT) {\n";
    *cur_output_ << GenTab() << "    if (" << step_var << ".data_.i == 0) { FakeluaThrowError(_S, \"'for' step is zero\"); }\n";
    *cur_output_ << GenTab() << "    " << step_pos_var << " = (" << step_var << ".data_.i > 0);\n";
    *cur_output_ << GenTab() << "} else if (" << step_var << ".type_ == VAR_FLOAT) {\n";
    *cur_output_ << GenTab() << "    if (" << step_var << ".data_.f == 0.0) { FakeluaThrowError(_S, \"'for' step is zero\"); }\n";
    *cur_output_ << GenTab() << "    " << step_pos_var << " = (" << step_var << ".data_.f > 0.0);\n";
    *cur_output_ << GenTab() << "} else { FakeluaThrowError(_S, \"'for' step must be a number\"); " << step_pos_var << " = 1; }\n";

    *cur_output_ << GenTab() << "while (1) {\n";
    cur_tab_++;

    *cur_output_ << GenTab() << "if (" << step_pos_var << ") {\n";
    cur_tab_++;
    *cur_output_ << GenTab() << std::format("OpLe(({0}), ({1}), {2});\n", ctrl_var, end_var, cmp_var);
    cur_tab_--;
    *cur_output_ << GenTab() << "} else {\n";
    cur_tab_++;
    *cur_output_ << GenTab() << std::format("OpGe(({0}), ({1}), {2});\n", ctrl_var, end_var, cmp_var);
    cur_tab_--;
    *cur_output_ << GenTab() << "}\n";

    *cur_output_ << GenTab() << std::format("IsTrue(({0}), {1});\n", cmp_var, cond_var);
    *cur_output_ << GenTab() << std::format("if (!{}) break;\n", cond_var);

    const auto &loop_var_name = for_stmt->Name();
    *cur_output_ << GenTab() << "CVar " << loop_var_name << " = " << ctrl_var << ";\n";
    EnterNativeVarScope();
    DeclareNativeVar(loop_var_name, T_DYNAMIC);

    *cur_output_ << GenTab() << "{\n";
    cur_tab_++;
    EnterNativeVarScope();
    CompileStmtBlock(for_stmt->Block());
    ExitNativeVarScope();
    cur_tab_--;
    *cur_output_ << GenTab() << "}\n";
    ExitNativeVarScope();

    *cur_output_ << GenTab() << std::format("OpAdd(({0}), ({1}), {2});\n", ctrl_var, step_var, ctrl_var);

    cur_tab_--;
    *cur_output_ << GenTab() << "}\n";
}

void FuncBodyCompiler::CompileStmtForIn(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::ForIn);
    const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt);

    const auto namelist = for_in->Namelist();
    DEBUG_ASSERT(namelist->Type() == SyntaxTreeType::NameList);
    const auto namelist_ptr = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist);
    const auto &names = namelist_ptr->Names();
    DEBUG_ASSERT(!names.empty() && names.size() <= 2);

    const auto explist = for_in->Explist();
    DEBUG_ASSERT(explist->Type() == SyntaxTreeType::ExpList);
    const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
    DEBUG_ASSERT(explist_ptr->Exps().size() == 1);

    const auto exp = explist_ptr->Exps()[0];
    DEBUG_ASSERT(exp->Type() == SyntaxTreeType::Exp);
    const auto exp_ptr = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    DEBUG_ASSERT(exp_ptr->GetExpKind() == ExpKind::kPrefixExp);
    const auto prefixexp = exp_ptr->Right();
    DEBUG_ASSERT(prefixexp->Type() == SyntaxTreeType::PrefixExp);
    const auto pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(prefixexp);
    DEBUG_ASSERT(pe_ptr->GetPrefixKind() == PrefixExpKind::kFunctionCall);
    const auto functioncall = pe_ptr->GetValue();
    DEBUG_ASSERT(functioncall->Type() == SyntaxTreeType::FunctionCall);
    const auto fc_ptr = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(functioncall);

    const auto func_pe = fc_ptr->prefixexp();
    DEBUG_ASSERT(func_pe->Type() == SyntaxTreeType::PrefixExp);
    const auto func_pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(func_pe);
    DEBUG_ASSERT(func_pe_ptr->GetPrefixKind() == PrefixExpKind::kVar);
    const auto func_var = std::dynamic_pointer_cast<SyntaxTreeVar>(func_pe_ptr->GetValue());
    const auto &func_name = func_var->GetName();
    DEBUG_ASSERT(func_name == "pairs" || func_name == "ipairs");

    const auto args_node = fc_ptr->Args();
    DEBUG_ASSERT(args_node->Type() == SyntaxTreeType::Args);
    const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(args_node);
    DEBUG_ASSERT(args_ptr->GetArgsKind() == ArgsKind::kExpList);
    const auto args_explist = args_ptr->Explist();
    const auto args_explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(args_explist);
    DEBUG_ASSERT(args_explist_ptr->Exps().size() == 1);

    const auto tbl_expr = CompileExp(args_explist_ptr->Exps()[0]);

    const auto tbl_var = std::format("flua_fi_tbl_{}", (*tmp_var_counter_)++);
    const auto sz_var = std::format("flua_fi_sz_{}", (*tmp_var_counter_)++);
    const auto idx_var = std::format("flua_fi_idx_{}", (*tmp_var_counter_)++);

    func_temp_decls_ << "    CVar " << tbl_var << ";\n";
    func_temp_decls_ << "    uint32_t " << sz_var << ";\n";
    func_temp_decls_ << "    uint32_t " << idx_var << ";\n";

    *cur_output_ << GenTab() << tbl_var << " = " << tbl_expr << ";\n";
    *cur_output_ << GenTab() << "if (" << tbl_var << ".type_ != VAR_TABLE) { FakeluaThrowError(_S, \"for in: not a table\"); }\n";
    *cur_output_ << GenTab() << sz_var << " = " << tbl_var << ".data_.t->count_;\n";

    *cur_output_ << GenTab() << "for (" << idx_var << " = 0; " << idx_var << " < " << sz_var << "; " << idx_var << "++) {\n";
    cur_tab_++;

    const auto &key_name = names[0];
    if (names.size() >= 2) {
        const auto &val_name = names[1];
        *cur_output_ << GenTab() << "CVar " << key_name << "; CVar " << val_name << ";\n";
        *cur_output_ << GenTab() << std::format("GET_TABLE_ENTRY({}, {}, {}, {});\n", tbl_var, idx_var, key_name, val_name);
    } else {
        const auto dummy_val = std::format("flua_fi_dummy_val_{}", (*tmp_var_counter_)++);
        func_temp_decls_ << "    CVar " << dummy_val << ";\n";
        *cur_output_ << GenTab() << "CVar " << key_name << ";\n";
        *cur_output_ << GenTab() << std::format("GET_TABLE_ENTRY({}, {}, {}, {});\n", tbl_var, idx_var, key_name, dummy_val);
    }

    EnterNativeVarScope();
    DeclareNativeVar(key_name, T_DYNAMIC);
    if (names.size() >= 2) {
        DeclareNativeVar(names[1], T_DYNAMIC);
    }
    *cur_output_ << GenTab() << "{\n";
    cur_tab_++;
    EnterNativeVarScope();
    CompileStmtBlock(for_in->Block());
    ExitNativeVarScope();
    cur_tab_--;
    *cur_output_ << GenTab() << "}\n";
    ExitNativeVarScope();

    cur_tab_--;
    *cur_output_ << GenTab() << "}\n";
}

std::string FuncBodyCompiler::CompileExp(const SyntaxTreeInterfacePtr &exp) {
    DEBUG_ASSERT(exp->Type() == SyntaxTreeType::Exp);
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto exp_kind = e->GetExpKind();
    const auto &value = e->ExpValue();

    DEBUG_ASSERT(exp_kind != ExpKind::kVarParams && "VarParams should have been caught by PreProcessor");

    if (exp_kind == ExpKind::kNil) {
        if (*in_global_init_) {
            return "(CVar){.type_ = VAR_NIL}";
        } else {
            return "kNil";
        }
    } else if (exp_kind == ExpKind::kFalse) {
        if (*in_global_init_) {
            return "(CVar){.type_ = VAR_BOOL, .data_.b = false}";
        } else {
            return "kFalse";
        }
    } else if (exp_kind == ExpKind::kTrue) {
        if (*in_global_init_) {
            return "(CVar){.type_ = VAR_BOOL, .data_.b = true}";
        } else {
            return "kTrue";
        }
    } else if (exp_kind == ExpKind::kNumber) {
        if (IsInteger(value)) {
            auto i = ToInteger(value);
            return std::format("(CVar){{.type_ = VAR_INT, .data_.i = {}}}", i);
        } else {
            auto f = ToFloat(value);
            return std::format("(CVar){{.type_ = VAR_FLOAT, .data_.f = {}}}", f);
        }
    } else if (exp_kind == ExpKind::kString) {
        auto id = s_->GetConstString().Alloc(value);
        return std::format("(CVar){{.type_ = VAR_STRINGID, .data_.i = {}}}", id);
    } else if (exp_kind == ExpKind::kPrefixExp) {
        const auto pe = e->Right();
        return CompilePrefixexp(pe);
    } else if (exp_kind == ExpKind::kTableConstructor) {
        const auto tc = e->Right();
        return CompileTableconstructor(tc);
    } else if (exp_kind == ExpKind::kBinop) {
        const auto left = e->Left();
        const auto right = e->Right();
        const auto op = e->Op();
        return CompileBinop(left, right, op);
    } else if (exp_kind == ExpKind::kUnop) {
        const auto right = e->Right();
        const auto op = e->Op();
        return CompileUnop(right, op);
    }

    DEBUG_ASSERT(false && "unreachable");
    ThrowError("unsupported expression kind", e);
}

std::string FuncBodyCompiler::CompilePrefixexp(const SyntaxTreeInterfacePtr &pe) {
    DEBUG_ASSERT(pe->Type() == SyntaxTreeType::PrefixExp);
    const auto pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(pe);

    const auto pe_kind = pe_ptr->GetPrefixKind();
    const auto value = pe_ptr->GetValue();

    DEBUG_ASSERT(pe_kind == PrefixExpKind::kVar || pe_kind == PrefixExpKind::kFunctionCall ||
                 pe_kind == PrefixExpKind::kExp);

    if (pe_kind == PrefixExpKind::kVar) {
        return CompileVar(value);
    } else if (pe_kind == PrefixExpKind::kFunctionCall) {
        return CompileFunctioncall(value);
    } else /*if (pe_kind == PrefixExpKind::kExp)*/ {
        return CompileExp(value);
    }
}

std::string FuncBodyCompiler::CompileTableconstructor(const SyntaxTreeInterfacePtr &tc) {
    DEBUG_ASSERT(!(*in_global_init_));

    DEBUG_ASSERT(tc->Type() == SyntaxTreeType::TableConstructor);
    const auto tc_ptr = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(tc);

    const auto var_name = std::format("flua_tbl_{}", (*tmp_var_counter_)++);

    func_temp_decls_ << "    " << "CVar " << var_name << ";\n";
    *cur_output_ << GenTab() << "SET_TABLE(" << var_name << ");\n";

    if (const auto fieldlist = tc_ptr->Fieldlist()) {
        DEBUG_ASSERT(fieldlist->Type() == SyntaxTreeType::FieldList);
        const auto fieldlist_ptr = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(fieldlist);

        int array_idx = 1;
        for (const auto &field: fieldlist_ptr->Fields()) {
            DEBUG_ASSERT(field->Type() == SyntaxTreeType::Field);
            const auto field_ptr = std::dynamic_pointer_cast<SyntaxTreeField>(field);
            const auto fkind = field_ptr->GetFieldKind();

            const auto value_exp = field_ptr->Value();
            const auto value_str = CompileExp(value_exp);

            std::string key_str;
            if (fkind == FieldKind::kObject) {
                const auto name = field_ptr->Name();
                const auto id = s_->GetConstString().Alloc(name);
                key_str = std::format("(CVar){{.type_ = VAR_STRINGID, .data_.i = {}}}", id);
            } else {
                DEBUG_ASSERT(fkind == FieldKind::kArray);
                if (const auto key = field_ptr->Key()) {
                    key_str = CompileExp(key);
                } else {
                    key_str = std::format("(CVar){{.type_ = VAR_INT, .data_.i = {}}}", array_idx);
                    ++array_idx;
                }
            }

            *cur_output_ << GenTab() << std::format("FlSetTable({}, {}, {});\n", var_name, key_str, value_str);
        }
    }

    return var_name;
}

std::string FuncBodyCompiler::CompileBinop(const SyntaxTreeInterfacePtr &left,
                                            const SyntaxTreeInterfacePtr &right,
                                            const SyntaxTreeInterfacePtr &op) {
    DEBUG_ASSERT(!(*in_global_init_));

    const auto op_ptr = std::dynamic_pointer_cast<SyntaxTreeBinop>(op);
    const auto op_kind = op_ptr->GetOpKind();

    if (op_kind == BinOpKind::kAnd || op_kind == BinOpKind::kOr) {
        const auto left_str = CompileExp(left);

        const auto tmp = std::format("flua_op_{}", (*tmp_var_counter_)++);
        func_temp_decls_ << "    " << "CVar " << tmp << ";\n";
        const auto tmp_bool = std::format("flua_bt_{}", (*tmp_var_counter_)++);
        func_temp_decls_ << "    " << "bool " << tmp_bool << ";\n";

        *cur_output_ << GenTab() << std::format("IsTrue(({}), {});\n", left_str, tmp_bool);

        if (op_kind == BinOpKind::kAnd) {
            *cur_output_ << GenTab() << std::format("if (!{}) {{\n", tmp_bool);
            cur_tab_++;
            *cur_output_ << GenTab() << std::format("{} = {};\n", tmp, left_str);
            cur_tab_--;
            *cur_output_ << GenTab() << "} else {\n";
            cur_tab_++;
            const auto right_str = CompileExp(right);
            *cur_output_ << GenTab() << std::format("{} = {};\n", tmp, right_str);
            cur_tab_--;
            *cur_output_ << GenTab() << "}\n";
        } else {
            *cur_output_ << GenTab() << std::format("if ({}) {{\n", tmp_bool);
            cur_tab_++;
            *cur_output_ << GenTab() << std::format("{} = {};\n", tmp, left_str);
            cur_tab_--;
            *cur_output_ << GenTab() << "} else {\n";
            cur_tab_++;
            const auto right_str = CompileExp(right);
            *cur_output_ << GenTab() << std::format("{} = {};\n", tmp, right_str);
            cur_tab_--;
            *cur_output_ << GenTab() << "}\n";
        }

        return tmp;
    }

    // Native arithmetic fast path
    static const std::unordered_set<BinOpKind> kNativeArithOps = {
            BinOpKind::kPlus, BinOpKind::kMinus, BinOpKind::kStar,
            BinOpKind::kSlash, BinOpKind::kDoubleSlash, BinOpKind::kPow, BinOpKind::kMod,
            BinOpKind::kBitAnd, BinOpKind::kXor, BinOpKind::kBitOr,
            BinOpKind::kLeftShift, BinOpKind::kRightShift};
    if (kNativeArithOps.contains(op_kind)) {
        const auto lt = InferArgTypeForSpec(left);
        const auto rt = InferArgTypeForSpec(right);
        if (lt != T_DYNAMIC && rt != T_DYNAMIC) {
            InferredType result_type;
            if (op_kind == BinOpKind::kSlash || op_kind == BinOpKind::kPow) {
                result_type = T_FLOAT;
            } else if (op_kind == BinOpKind::kBitAnd || op_kind == BinOpKind::kXor ||
                       op_kind == BinOpKind::kBitOr || op_kind == BinOpKind::kLeftShift ||
                       op_kind == BinOpKind::kRightShift) {
                result_type = T_INT;
            } else {
                result_type = (lt == T_INT && rt == T_INT) ? T_INT : T_FLOAT;
            }
            const auto left_native = CompileNumericExp(left);
            const auto right_native = CompileNumericExp(right);
            std::string native_expr;
            if (op_kind == BinOpKind::kPlus) {
                native_expr = std::format("(({}) + ({}))", left_native, right_native);
            } else if (op_kind == BinOpKind::kMinus) {
                native_expr = std::format("(({}) - ({}))", left_native, right_native);
            } else if (op_kind == BinOpKind::kStar) {
                native_expr = std::format("(({}) * ({}))", left_native, right_native);
            } else if (op_kind == BinOpKind::kSlash) {
                native_expr = std::format("((double)({}) / (double)({}))", left_native, right_native);
            } else if (op_kind == BinOpKind::kPow) {
                native_expr = std::format("pow((double)({}), (double)({}))", left_native, right_native);
            } else if (op_kind == BinOpKind::kDoubleSlash) {
                if (result_type == T_INT) {
                    const auto ntmp = std::format("flua_native_{}", (*tmp_var_counter_)++);
                    func_temp_decls_ << "    int64_t " << ntmp << ";\n";
                    *cur_output_ << GenTab() << std::format("FlFloorDivInt(({}), ({}), {});\n", left_native, right_native, ntmp);
                    native_expr = ntmp;
                } else {
                    native_expr = std::format("floor((double)({}) / (double)({}))", left_native, right_native);
                }
            } else if (op_kind == BinOpKind::kMod) {
                if (result_type == T_INT) {
                    const auto ntmp = std::format("flua_native_{}", (*tmp_var_counter_)++);
                    func_temp_decls_ << "    int64_t " << ntmp << ";\n";
                    *cur_output_ << GenTab() << std::format("FlModInt(({}), ({}), {});\n", left_native, right_native, ntmp);
                    native_expr = ntmp;
                } else {
                    const auto ntmp = std::format("flua_native_{}", (*tmp_var_counter_)++);
                    func_temp_decls_ << "    double " << ntmp << ";\n";
                    *cur_output_ << GenTab() << std::format("FlModFloat((double)({}), (double)({}), {});\n", left_native, right_native, ntmp);
                    native_expr = ntmp;
                }
            } else if (op_kind == BinOpKind::kBitAnd) {
                native_expr = std::format("((int64_t)({}) & (int64_t)({}))", left_native, right_native);
            } else if (op_kind == BinOpKind::kBitOr) {
                native_expr = std::format("((int64_t)({}) | (int64_t)({}))", left_native, right_native);
            } else if (op_kind == BinOpKind::kXor) {
                native_expr = std::format("((int64_t)({}) ^ (int64_t)({}))", left_native, right_native);
            } else if (op_kind == BinOpKind::kLeftShift) {
                const auto ntmp = std::format("flua_native_{}", (*tmp_var_counter_)++);
                func_temp_decls_ << "    int64_t " << ntmp << ";\n";
                *cur_output_ << GenTab() << std::format("FlLShiftInt(({}), ({}), {});\n", left_native, right_native, ntmp);
                native_expr = ntmp;
            } else if (op_kind == BinOpKind::kRightShift) {
                const auto ntmp = std::format("flua_native_{}", (*tmp_var_counter_)++);
                func_temp_decls_ << "    int64_t " << ntmp << ";\n";
                *cur_output_ << GenTab() << std::format("FlRShiftInt(({}), ({}), {});\n", left_native, right_native, ntmp);
                native_expr = ntmp;
            }
            if (!native_expr.empty()) {
                const auto tmp = std::format("flua_op_{}", (*tmp_var_counter_)++);
                func_temp_decls_ << "    CVar " << tmp << ";\n";
                *cur_output_ << GenTab() << tmp << " = " << BoxNativeValue(native_expr, result_type) << ";\n";
                return tmp;
            }
        }
    }

    const auto left_str = CompileExp(left);
    const auto right_str = CompileExp(right);

    const auto tmp = std::format("flua_op_{}", (*tmp_var_counter_)++);
    func_temp_decls_ << "    " << "CVar " << tmp << ";\n";

    const auto l = std::format("({})", left_str);
    const auto r = std::format("({})", right_str);

    if (op_kind == BinOpKind::kPlus) {
        *cur_output_ << GenTab() << std::format("OpAdd({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kMinus) {
        *cur_output_ << GenTab() << std::format("OpSub({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kStar) {
        *cur_output_ << GenTab() << std::format("OpMul({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kSlash) {
        *cur_output_ << GenTab() << std::format("OpDiv({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kDoubleSlash) {
        *cur_output_ << GenTab() << std::format("OpFloorDiv({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kPow) {
        *cur_output_ << GenTab() << std::format("OpPow({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kMod) {
        *cur_output_ << GenTab() << std::format("OpMod({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kBitAnd) {
        *cur_output_ << GenTab() << std::format("OpBitAnd({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kXor) {
        *cur_output_ << GenTab() << std::format("OpBitXor({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kBitOr) {
        *cur_output_ << GenTab() << std::format("OpBitOr({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kRightShift) {
        *cur_output_ << GenTab() << std::format("OpRightShift({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kLeftShift) {
        *cur_output_ << GenTab() << std::format("OpLeftShift({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kConcat) {
        *cur_output_ << GenTab() << std::format("{} = FlConcat({}, {});\n", tmp, l, r);
    } else if (op_kind == BinOpKind::kLess) {
        *cur_output_ << GenTab() << std::format("OpLt({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kLessEqual) {
        *cur_output_ << GenTab() << std::format("OpLe({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kMore) {
        *cur_output_ << GenTab() << std::format("OpGt({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kMoreEqual) {
        *cur_output_ << GenTab() << std::format("OpGe({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kEqual) {
        *cur_output_ << GenTab() << std::format("OpEq({}, {}, {});\n", l, r, tmp);
    } else if (op_kind == BinOpKind::kNotEqual) {
        *cur_output_ << GenTab() << std::format("OpNe({}, {}, {});\n", l, r, tmp);
    } else {
        ThrowError("binary operator not supported", op);
    }

    return tmp;
}

std::string FuncBodyCompiler::CompileUnop(const SyntaxTreeInterfacePtr &right,
                                           const SyntaxTreeInterfacePtr &op) {
    DEBUG_ASSERT(!(*in_global_init_));

    const auto op_ptr = std::dynamic_pointer_cast<SyntaxTreeUnop>(op);
    const auto op_kind = op_ptr->GetOpKind();

    const auto right_str = CompileExp(right);

    const auto tmp = std::format("flua_op_{}", (*tmp_var_counter_)++);
    func_temp_decls_ << "    " << "CVar " << tmp << ";\n";

    const auto r = std::format("({})", right_str);

    if (op_kind == UnOpKind::kNot) {
        *cur_output_ << GenTab() << std::format("OpNot({}, {});\n", r, tmp);
    } else if (op_kind == UnOpKind::kMinus) {
        *cur_output_ << GenTab() << std::format("OpUnaryMinus({}, {});\n", r, tmp);
    } else if (op_kind == UnOpKind::kBitNot) {
        *cur_output_ << GenTab() << std::format("OpBitNot({}, {});\n", r, tmp);
    } else if (op_kind == UnOpKind::kNumberSign) {
        *cur_output_ << GenTab() << std::format("OpLen({}, {});\n", r, tmp);
    } else {
        ThrowError("unary operator not supported", op);
    }

    return tmp;
}

std::string FuncBodyCompiler::CompileVar(const SyntaxTreeInterfacePtr &v) {
    DEBUG_ASSERT(v->Type() == SyntaxTreeType::Var);
    auto v_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(v);

    DEBUG_ASSERT(v_ptr->GetVarKind() == VarKind::kSimple || v_ptr->GetVarKind() == VarKind::kSquare ||
                 v_ptr->GetVarKind() == VarKind::kDot);

    if (const auto var_kind = v_ptr->GetVarKind(); var_kind == VarKind::kSimple) {
        const auto &name = v_ptr->GetName();
        DEBUG_ASSERT(!(*in_global_init_));
        if (const auto spec_it = spec_param_types_.find(name); spec_it != spec_param_types_.end()) {
            if (spec_it->second == T_INT) {
                return std::format("(CVar){{.type_ = VAR_INT, .data_.i = (int64_t)({})}}", name);
            }
            return std::format("(CVar){{.type_ = VAR_FLOAT, .data_.f = (double)({})}}", name);
        }
        const auto native_type = GetNativeVarType(name);
        if (native_type == T_INT) {
            return std::format("(CVar){{.type_ = VAR_INT, .data_.i = (int64_t)({})}}", name);
        }
        if (native_type == T_FLOAT) {
            return std::format("(CVar){{.type_ = VAR_FLOAT, .data_.f = (double)({})}}", name);
        }
        return name;
    } else if (var_kind == VarKind::kSquare) {
        DEBUG_ASSERT(!(*in_global_init_));
        const auto pe = v_ptr->GetPrefixexp();
        const auto exp = v_ptr->GetExp();
        auto pe_ret = CompilePrefixexp(pe);
        auto exp_ret = CompileExp(exp);
        return std::format("FlGetTable({}, {})", pe_ret, exp_ret);
    } else /*if (var_kind == VarKind::kDot)*/ {
        DEBUG_ASSERT(!(*in_global_init_));
        const auto pe = v_ptr->GetPrefixexp();
        const auto name = v_ptr->GetName();
        auto pe_ret = CompilePrefixexp(pe);

        const auto name_exp = std::make_shared<SyntaxTreeExp>(v_ptr->Loc());
        name_exp->SetExpKind(ExpKind::kString);
        name_exp->SetValue(name);
        auto exp_ret = CompileExp(name_exp);

        return std::format("FlGetTable({}, {})", pe_ret, exp_ret);
    }
}

std::string FuncBodyCompiler::CompileNumericExp(const SyntaxTreeInterfacePtr &exp) {
    DEBUG_ASSERT(exp && exp->Type() == SyntaxTreeType::Exp);

    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto exp_kind = e->GetExpKind();

    if (exp_kind == ExpKind::kNumber) {
        if (LookupNodeType(e.get()) == T_INT) {
            return std::to_string(ToInteger(e->ExpValue()));
        }
        if (LookupNodeType(e.get()) == T_FLOAT) {
            return std::format("{}", ToFloat(e->ExpValue()));
        }
        ThrowError("number node is not inferred as numeric", exp);
    } else if (exp_kind == ExpKind::kPrefixExp) {
        const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e->Right());
        DEBUG_ASSERT(pe);
        if (pe->GetPrefixKind() == PrefixExpKind::kVar) {
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue());
            if (!var || var->GetVarKind() != VarKind::kSimple) {
                ThrowError("only simple variable is supported in numeric specialization", exp);
            }
            const auto &vname = var->GetName();
            if (spec_param_types_.contains(vname)) {
                return vname;
            }
            if (IsTypedNativeVar(vname)) {
                return vname;
            }
            if (LookupNodeType(e.get()) == T_FLOAT) {
                return std::format("{}.data_.f", vname);
            }
            return std::format("{}.data_.i", vname);
        }
        if (pe->GetPrefixKind() == PrefixExpKind::kExp) {
            return CompileNumericExp(pe->GetValue());
        }
        if (pe->GetPrefixKind() == PrefixExpKind::kFunctionCall) {
            const auto inferred = InferArgTypeForSpec(exp);
            if (inferred == T_DYNAMIC) {
                ThrowError("function call cannot be specialized as numeric", exp);
            }
            const auto native_result = TryCompileNativeSpecCallExpr(pe->GetValue());
            if (!native_result.empty()) {
                return native_result;
            }
            const auto call_str = CompileFunctioncall(pe->GetValue());
            return inferred == T_INT ? std::format("({}.data_.i)", call_str) : std::format("({}.data_.f)", call_str);
        }
        ThrowError("function call cannot be specialized as numeric", exp);
    } else if (exp_kind == ExpKind::kBinop) {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(e->Op());
        DEBUG_ASSERT(op);
        const auto op_kind = op->GetOpKind();

        if (op_kind == BinOpKind::kAnd) {
            const auto left_type = InferArgTypeForSpec(e->Left());
            const auto left_eval_result = std::format("flua_native_{}", (*tmp_var_counter_)++);
            const auto c_type_str = (left_type == T_FLOAT) ? "double" : "int64_t";
            func_temp_decls_ << "    " << c_type_str << " " << left_eval_result << ";\n";
            const auto left_native = CompileNumericExp(e->Left());
            *cur_output_ << GenTab() << left_eval_result << " = " << left_native << ";\n";
            return CompileNumericExp(e->Right());
        }
        if (op_kind == BinOpKind::kOr) {
            return CompileNumericExp(e->Left());
        }

        const auto left = CompileNumericExp(e->Left());
        const auto right = CompileNumericExp(e->Right());

        if (op_kind == BinOpKind::kPlus) {
            return std::format("(({}) + ({}))", left, right);
        }
        if (op_kind == BinOpKind::kMinus) {
            return std::format("(({}) - ({}))", left, right);
        }
        if (op_kind == BinOpKind::kStar) {
            return std::format("(({}) * ({}))", left, right);
        }
        if (op_kind == BinOpKind::kSlash) {
            return std::format("((double)({}) / (double)({}))", left, right);
        }
        if (op_kind == BinOpKind::kPow) {
            return std::format("pow((double)({}), (double)({}))", left, right);
        }
        if (op_kind == BinOpKind::kDoubleSlash) {
            if (LookupNodeType(e.get()) == T_INT) {
                const auto ntmp = std::format("flua_native_{}", (*tmp_var_counter_)++);
                func_temp_decls_ << "    int64_t " << ntmp << ";\n";
                *cur_output_ << GenTab() << std::format("FlFloorDivInt(({}), ({}), {});\n", left, right, ntmp);
                return ntmp;
            }
            return std::format("floor((double)({}) / (double)({}))", left, right);
        }
        if (op_kind == BinOpKind::kMod) {
            if (LookupNodeType(e.get()) == T_INT) {
                const auto ntmp = std::format("flua_native_{}", (*tmp_var_counter_)++);
                func_temp_decls_ << "    int64_t " << ntmp << ";\n";
                *cur_output_ << GenTab() << std::format("FlModInt(({}), ({}), {});\n", left, right, ntmp);
                return ntmp;
            }
            const auto ntmp = std::format("flua_native_{}", (*tmp_var_counter_)++);
            func_temp_decls_ << "    double " << ntmp << ";\n";
            *cur_output_ << GenTab() << std::format("FlModFloat((double)({}), (double)({}), {});\n", left, right, ntmp);
            return ntmp;
        }
        if (op_kind == BinOpKind::kBitAnd) {
            return std::format("((int64_t)({}) & (int64_t)({}))", left, right);
        }
        if (op_kind == BinOpKind::kBitOr) {
            return std::format("((int64_t)({}) | (int64_t)({}))", left, right);
        }
        if (op_kind == BinOpKind::kXor) {
            return std::format("((int64_t)({}) ^ (int64_t)({}))", left, right);
        }
        if (op_kind == BinOpKind::kLeftShift) {
            const auto ntmp = std::format("flua_native_{}", (*tmp_var_counter_)++);
            func_temp_decls_ << "    int64_t " << ntmp << ";\n";
            *cur_output_ << GenTab() << std::format("FlLShiftInt(({}), ({}), {});\n", left, right, ntmp);
            return ntmp;
        }
        if (op_kind == BinOpKind::kRightShift) {
            const auto ntmp = std::format("flua_native_{}", (*tmp_var_counter_)++);
            func_temp_decls_ << "    int64_t " << ntmp << ";\n";
            *cur_output_ << GenTab() << std::format("FlRShiftInt(({}), ({}), {});\n", left, right, ntmp);
            return ntmp;
        }

        ThrowError("operator is not supported in numeric specialization", exp);
    } else if (exp_kind == ExpKind::kUnop) {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(e->Op());
        DEBUG_ASSERT(op);
        const auto op_kind = op->GetOpKind();
        if (op_kind == UnOpKind::kNumberSign) {
            const auto operand_cvar = CompileExp(e->Right());
            const auto ntmp = std::format("flua_native_{}", (*tmp_var_counter_)++);
            func_temp_decls_ << "    int64_t " << ntmp << ";\n";
            *cur_output_ << GenTab() << std::format("FlLenInt({}, {});\n", operand_cvar, ntmp);
            return ntmp;
        }
        const auto operand = CompileNumericExp(e->Right());
        if (op_kind == UnOpKind::kMinus) {
            return std::format("(-({}))", operand);
        }
        if (op_kind == UnOpKind::kBitNot) {
            return std::format("(~((int64_t)({})))", operand);
        }
        ThrowError("unary operator is not supported in numeric specialization", exp);
    }

    ThrowError("unsupported numeric-specialized expression", exp);
}

std::string FuncBodyCompiler::TryCompileNativeSpecCallExpr(const SyntaxTreeInterfacePtr &functioncall_node) {
    const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(functioncall_node);
    if (!fc) {
        return {};
    }
    const auto args_node = fc->Args();
    if (!args_node) {
        return {};
    }
    const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(args_node);
    if (!args_ptr || args_ptr->GetArgsKind() != ArgsKind::kExpList) {
        return {};
    }
    const auto callee_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
    if (!callee_pe || callee_pe->GetPrefixKind() != PrefixExpKind::kVar) {
        return {};
    }
    const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(callee_pe->GetValue());
    if (!callee_var || callee_var->GetVarKind() != VarKind::kSimple) {
        return {};
    }
    const auto &callee_name = callee_var->GetName();
    const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(args_ptr->Explist());
    if (!explist_ptr) {
        return {};
    }
    const auto &raw_args = explist_ptr->Exps();

    int bitmask = 0;
    InferredType spec_ret = T_DYNAMIC;
    if (!TryInferMathCallSpec(callee_name, raw_args, bitmask, spec_ret)) {
        return {};
    }
    if (spec_ret != T_INT && spec_ret != T_FLOAT) {
        return {};
    }

    const auto &math_params = math_param_positions_->at(callee_name);

    std::unordered_map<int, std::string> native_exprs;
    for (int param_pos: math_params) {
        const auto native_expr = TryCompileNativeExpr(raw_args[param_pos]);
        if (native_expr.empty()) {
            return {};
        }
        native_exprs[param_pos] = native_expr;
    }

    const auto spec_name = SpecFuncName(callee_name, math_params, bitmask);
    std::string call = spec_name + "(";
    for (int i = 0; i < static_cast<int>(raw_args.size()); ++i) {
        if (i > 0) {
            call += ", ";
        }
        const auto ne_it = native_exprs.find(i);
        if (ne_it != native_exprs.end()) {
            call += ne_it->second;
        } else {
            call += CompileExp(raw_args[i]);
        }
    }
    call += ")";

    const auto ntmp = std::format("flua_native_{}", (*tmp_var_counter_)++);
    func_temp_decls_ << "    " << SpecReturnCTypeName(spec_ret) << " " << ntmp << ";\n";
    *cur_output_ << GenTab() << ntmp << " = " << call << ";\n";
    return ntmp;
}

std::string FuncBodyCompiler::CompileFunctioncall(const SyntaxTreeInterfacePtr &functioncall) {
    DEBUG_ASSERT(!(*in_global_init_));

    DEBUG_ASSERT(functioncall->Type() == SyntaxTreeType::FunctionCall);
    const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(functioncall);

    DEBUG_ASSERT(fc->Name().empty());

    const auto args_node = fc->Args();
    DEBUG_ASSERT(args_node->Type() == SyntaxTreeType::Args);
    const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(args_node);
    const auto args_kind = args_ptr->GetArgsKind();

    const auto pe_pre = fc->prefixexp();
    DEBUG_ASSERT(pe_pre->Type() == SyntaxTreeType::PrefixExp);
    const auto pe_pre_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(pe_pre);

    // 尝试直接调用优化：若被调函数是含有数学参数的本地函数，
    // 且所有数学参数的实参类型均已知，则直接发出特化调用。
    if (pe_pre_ptr->GetPrefixKind() == PrefixExpKind::kVar &&
        args_kind == ArgsKind::kExpList) {
        const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe_pre_ptr->GetValue());
        if (callee_var && callee_var->GetVarKind() == VarKind::kSimple) {
            const auto &callee_name = callee_var->GetName();
            const auto math_it = math_param_positions_->find(callee_name);
            if (math_it != math_param_positions_->end()) {
                const auto &math_params = math_it->second;
                const auto explist_arg = args_ptr->Explist();
                DEBUG_ASSERT(explist_arg->Type() == SyntaxTreeType::ExpList);
                const auto explist_arg_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist_arg);
                const auto &raw_args = explist_arg_ptr->Exps();

                int bitmask = 0;
                if (TryInferMathCallBitmask(callee_name, raw_args, bitmask)) {
                    std::unordered_map<int, std::string> native_exprs;
                    bool can_spec = true;
                    for (int param_pos: math_params) {
                        const auto native_expr = TryCompileNativeExpr(raw_args[param_pos]);
                        if (native_expr.empty()) {
                            can_spec = false;
                            break;
                        }
                        native_exprs[param_pos] = native_expr;
                    }

                    if (can_spec) {
                        const auto spec_name = SpecFuncName(callee_name, math_params, bitmask);
                        std::string call = spec_name + "(";
                        for (int i = 0; i < static_cast<int>(raw_args.size()); ++i) {
                            if (i > 0) {
                                call += ", ";
                            }
                            const auto ne_it = native_exprs.find(i);
                            if (ne_it != native_exprs.end()) {
                                call += ne_it->second;
                            } else {
                                call += CompileExp(raw_args[i]);
                            }
                        }
                        call += ")";
                        const auto spec_ret = GetSpecReturnType(callee_name, bitmask);
                        const auto tmp = std::format("flua_call_{}", (*tmp_var_counter_)++);
                        func_temp_decls_ << "    CVar " << tmp << ";\n";
                        if (spec_ret == T_INT || spec_ret == T_FLOAT) {
                            const auto ntmp = std::format("flua_native_{}", (*tmp_var_counter_)++);
                            func_temp_decls_ << "    " << SpecReturnCTypeName(spec_ret) << " " << ntmp << ";\n";
                            *cur_output_ << GenTab() << ntmp << " = " << call << ";\n";
                            *cur_output_ << GenTab() << tmp << " = " << BoxNativeValue(ntmp, spec_ret) << ";\n";
                        } else {
                            *cur_output_ << GenTab() << tmp << " = " << call << ";\n";
                        }
                        return tmp;
                    }
                }
            }
        }
    }

    // 普通路径：将所有参数编译为 CVar。
    std::vector<std::string> compiled_args;
    if (args_kind == ArgsKind::kExpList) {
        const auto explist = args_ptr->Explist();
        DEBUG_ASSERT(explist->Type() == SyntaxTreeType::ExpList);
        const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
        for (const auto &exp: explist_ptr->Exps()) {
            compiled_args.push_back(CompileExp(exp));
        }
    } else if (args_kind == ArgsKind::kTableConstructor) {
        compiled_args.push_back(CompileTableconstructor(args_ptr->Tableconstructor()));
    } else if (args_kind == ArgsKind::kString) {
        compiled_args.push_back(CompileExp(args_ptr->String()));
    }
    DEBUG_ASSERT(pe_pre_ptr->GetPrefixKind() == PrefixExpKind::kVar &&
                 "callee must be variable prefixexp (PreProcessor should have caught it)");
    const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe_pre_ptr->GetValue());
    DEBUG_ASSERT(var && var->GetVarKind() == VarKind::kSimple &&
                 "callee must be simple variable (PreProcessor should have caught it)");
    std::string call_expr;
    const auto &func_name = var->GetName();
    if (func_name == "FAKELUA_SET_TABLE") {
        if (compiled_args.size() != 3) {
            ThrowError("FAKELUA_SET_TABLE expects exactly 3 arguments", functioncall);
        }
        const auto tmp = std::format("flua_call_{}", (*tmp_var_counter_)++);
        func_temp_decls_ << "    " << "CVar " << tmp << ";\n";
        *cur_output_ << GenTab() << std::format("FlSetTable({}, {}, {});\n", compiled_args[0], compiled_args[1], compiled_args[2]);
        *cur_output_ << GenTab() << std::format("SET_NIL({});\n", tmp);
        return tmp;
    } else if (local_func_names_->contains(func_name)) {
        call_expr = func_name + "(";
        for (size_t i = 0; i < compiled_args.size(); ++i) {
            if (i > 0) {
                call_expr += ", ";
            }
            call_expr += compiled_args[i];
        }
        call_expr += ")";
    } else {
        call_expr = std::format("FakeluaCallByName(_S, FAKELUA_JIT_TYPE, \"{}\", {}", func_name, compiled_args.size());
        for (const auto &arg: compiled_args) {
            call_expr += ", " + arg;
        }
        call_expr += ")";
    }
    const auto tmp = std::format("flua_call_{}", (*tmp_var_counter_)++);
    func_temp_decls_ << "    " << "CVar " << tmp << ";\n";
    *cur_output_ << GenTab() << tmp << " = " << call_expr << ";\n";

    return tmp;
}

}// namespace fakelua
