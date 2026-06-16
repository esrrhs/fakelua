#include "compile/semantic_analysis.h"
#include "state/state.h"
#include "util/common.h"
#include "util/exception.h"
#include <algorithm>

namespace fakelua {

SemanticAnalysis::SemanticAnalysis(State *s) : s_(s) {
}

AnalysisResult SemanticAnalysis::Analyze(const ParseResult &pr, const CompileConfig &cfg) {
    file_name_ = pr.file_name;

    AnalysisResult ar;
    AnalyzeGlobalConstNames(pr.chunk, ar);
    CheckUnsupportedSyntax(pr.chunk, ar);
    AnalyzeFunctionReturnCounts(pr.chunk, ar);

    WalkSyntaxTree(pr.chunk, [&](const SyntaxTreeInterfacePtr &node) {
        if (IsFunctionCallExp(node)) {
            ar.function_call_exps.insert(node.get());
            std::string name = GetCalleeName(node);
            ar.callee_names[node.get()] = name;

            const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
            const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(exp->Right());
            const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(pe->GetValue());
            if (fc) {
                ar.callee_names[fc.get()] = name;
            }
        }
    });

    return ar;
}

void SemanticAnalysis::AnalyzeGlobalConstNames(const SyntaxTreeInterfacePtr &chunk, AnalysisResult &ar) {
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
    for (const auto &stmt: block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::LocalVar) {
            const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
            const auto namelist = local_var->Namelist();
            const auto explist = local_var->Explist();
            if (!namelist || !explist) {
                continue;
            }
            const auto namelist_ptr = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist);
            const auto &names = namelist_ptr->Names();
            for (const auto &name: names) {
                if (ar.global_const_names.contains(name)) {
                    ThrowError("duplicate global const variable: " + name, stmt);
                }
                ar.global_const_names.insert(name);
            }
        }
    }
}

void SemanticAnalysis::AnalyzeFunctionReturnCounts(const SyntaxTreeInterfacePtr &chunk, AnalysisResult &ar) {
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
    for (const auto &stmt: block->Stmts()) {
        std::string name;
        SyntaxTreeInterfacePtr funcbody;
        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            const auto funcname_ptr = std::dynamic_pointer_cast<SyntaxTreeFuncname>(func->Funcname());
            const auto funcnamelist = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(funcname_ptr->FuncNameList());
            name = funcnamelist->Funcnames()[0];
            funcbody = func->Funcbody();
        } else if (stmt->Type() == SyntaxTreeType::LocalFunction) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt);
            name = func->Name();
            funcbody = func->Funcbody();
        }
        if (!funcbody) {
            continue;
        }

        const auto funcbody_ptr = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
        const auto func_block = funcbody_ptr->Block();

        std::vector<SyntaxTreeInterfacePtr> returns;
        CollectReturnsForBlock(func_block, returns);

        int max_returns = 0;
        for (const auto &ret_node: returns) {
            const auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(ret_node);
            const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(ret->Explist());
            if (!el || el->Exps().empty()) {
                // return count is 0
            } else {
                int count = static_cast<int>(el->Exps().size());
                if (IsFunctionCallExp(el->Exps().back())) {
                    max_returns = -1;// dynamic
                } else if (max_returns >= 0) {
                    max_returns = std::max(max_returns, count);
                }
            }
        }
        ar.function_max_returns[name] = max_returns;
    }
}

void SemanticAnalysis::CollectReturnsForBlock(const SyntaxTreeInterfacePtr &node, std::vector<SyntaxTreeInterfacePtr> &returns) {
    if (!node) {
        return;
    }
    switch (node->Type()) {
        case SyntaxTreeType::Return: {
            returns.push_back(node);
            break;
        }
        case SyntaxTreeType::Block: {
            const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            for (const auto &stmt: block->Stmts()) {
                CollectReturnsForBlock(stmt, returns);
            }
            break;
        }
        case SyntaxTreeType::If: {
            const auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
            CollectReturnsForBlock(if_node->Block(), returns);
            CollectReturnsForBlock(if_node->ElseIfs(), returns);
            CollectReturnsForBlock(if_node->ElseBlock(), returns);
            break;
        }
        case SyntaxTreeType::ElseIfList: {
            const auto el = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(node);
            for (const auto &blk: el->ElseifBlocks()) {
                CollectReturnsForBlock(blk, returns);
            }
            break;
        }
        case SyntaxTreeType::While: {
            const auto while_node = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
            CollectReturnsForBlock(while_node->Block(), returns);
            break;
        }
        case SyntaxTreeType::Repeat: {
            const auto rep = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
            CollectReturnsForBlock(rep->Block(), returns);
            break;
        }
        case SyntaxTreeType::ForLoop: {
            const auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
            CollectReturnsForBlock(for_loop->Block(), returns);
            break;
        }
        case SyntaxTreeType::ForIn: {
            const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
            CollectReturnsForBlock(for_in->Block(), returns);
            break;
        }
        case SyntaxTreeType::Function:
        case SyntaxTreeType::LocalFunction:
        case SyntaxTreeType::FunctionDef:
        case SyntaxTreeType::Empty:
        case SyntaxTreeType::Label:
        case SyntaxTreeType::Assign:
        case SyntaxTreeType::FunctionCall:
        case SyntaxTreeType::Break:
        case SyntaxTreeType::Goto:
        case SyntaxTreeType::LocalVar: {
            // These are valid statement types but do not contain return statements that belong to the current function scope.
            break;
        }
        case SyntaxTreeType::None:
        case SyntaxTreeType::VarList:
        case SyntaxTreeType::ExpList:
        case SyntaxTreeType::Var:
        case SyntaxTreeType::TableConstructor:
        case SyntaxTreeType::FieldList:
        case SyntaxTreeType::Field:
        case SyntaxTreeType::NameList:
        case SyntaxTreeType::FuncNameList:
        case SyntaxTreeType::FuncName:
        case SyntaxTreeType::FuncBody:
        case SyntaxTreeType::ParList:
        case SyntaxTreeType::Exp:
        case SyntaxTreeType::Binop:
        case SyntaxTreeType::Unop:
        case SyntaxTreeType::Args:
        case SyntaxTreeType::PrefixExp: {
            ThrowFakeluaException(std::format("unexpected non-statement syntax tree type in CollectReturnsForBlock: {}",
                                              SyntaxTreeTypeToString(node->Type())));
        }
        default: {
            ThrowFakeluaException(std::format("unknown syntax tree type in CollectReturnsForBlock: {}", static_cast<int>(node->Type())));
        }
    }
}

bool SemanticAnalysis::IsFunctionCallExp(const SyntaxTreeInterfacePtr &exp_node) {
    if (!exp_node || exp_node->Type() != SyntaxTreeType::Exp) {
        return false;
    }
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(exp_node);
    if (exp->GetExpKind() != ExpKind::kPrefixExp) {
        return false;
    }
    const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(exp->Right());
    return pe && pe->GetPrefixKind() == PrefixExpKind::kFunctionCall;
}

std::string SemanticAnalysis::GetCalleeName(const SyntaxTreeInterfacePtr &exp_node) {
    if (!IsFunctionCallExp(exp_node)) {
        return "";
    }
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(exp_node);
    const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(exp->Right());
    const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(pe->GetValue());
    const auto pe_pre = fc->prefixexp();
    if (pe_pre->Type() != SyntaxTreeType::PrefixExp) {
        return "";
    }
    const auto pe_pre_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(pe_pre);
    if (pe_pre_ptr->GetPrefixKind() == PrefixExpKind::kVar) {
        const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe_pre_ptr->GetValue());
        if (callee_var->GetVarKind() == VarKind::kSimple) {
            return callee_var->GetName();
        }
    }
    return "";
}

void SemanticAnalysis::CheckUnsupportedSyntax(const SyntaxTreeInterfacePtr &chunk, const AnalysisResult &ar) {
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);

    top_level_stmts_.clear();
    for (const auto &stmt: top_block->Stmts()) {
        top_level_stmts_.insert(stmt.get());

        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            const auto funcname = std::dynamic_pointer_cast<SyntaxTreeFuncname>(func->Funcname());
            if (funcname) {
                if (!funcname->ColonName().empty()) {
                    ThrowError("Unsupported function name with method definition", stmt);
                }
                const auto fnlist = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(funcname->FuncNameList());
                if (fnlist && fnlist->Funcnames().size() != 1) {
                    ThrowError(std::format("Unsupported function name with {} parts", fnlist->Funcnames().size()), stmt);
                }
            }
        }

        if (stmt->Type() == SyntaxTreeType::LocalVar) {
            const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
            const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
            if (!namelist) {
                ThrowError("local variable namelist is missing", stmt);
            }
            const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(lv->Explist());
            if (el) {
                bool last_is_func = !el->Exps().empty() && IsFunctionCallExp(el->Exps().back());
                if (namelist->Names().size() != el->Exps().size() && !(last_is_func && namelist->Names().size() > el->Exps().size())) {
                    ThrowError(std::format("local variable count {} not match expression count {}", namelist->Names().size(),
                                           el->Exps().size()),
                               stmt);
                }
                for (const auto &exp: el->Exps()) {
                    CheckGlobalConstExp(exp);
                }
            }
        }
    }

    WalkSyntaxTree(chunk, [this, &ar](const SyntaxTreeInterfacePtr &node) { CheckNode(node, ar); });
}

void SemanticAnalysis::CheckNode(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar) {
    switch (node->Type()) {
        case SyntaxTreeType::Goto:
        case SyntaxTreeType::Label: {
            CheckGotoOrLabel(node);
            break;
        }
        case SyntaxTreeType::FunctionCall: {
            CheckFunctionCall(node);
            break;
        }
        case SyntaxTreeType::ParList: {
            CheckParList(node, ar);
            break;
        }
        case SyntaxTreeType::LocalVar: {
            CheckLocalVar(node, ar);
            break;
        }
        case SyntaxTreeType::Return: {
            break;
        }
        case SyntaxTreeType::ForLoop: {
            CheckForLoop(node);
            break;
        }
        case SyntaxTreeType::ForIn: {
            CheckForIn(node);
            break;
        }
        case SyntaxTreeType::Exp: {
            CheckExp(node);
            break;
        }
        case SyntaxTreeType::None:
        case SyntaxTreeType::Empty:
        case SyntaxTreeType::Block:
        case SyntaxTreeType::Assign:
        case SyntaxTreeType::VarList:
        case SyntaxTreeType::ExpList:
        case SyntaxTreeType::Var:
        case SyntaxTreeType::TableConstructor:
        case SyntaxTreeType::FieldList:
        case SyntaxTreeType::Field:
        case SyntaxTreeType::Break:
        case SyntaxTreeType::While:
        case SyntaxTreeType::Repeat:
        case SyntaxTreeType::If:
        case SyntaxTreeType::ElseIfList:
        case SyntaxTreeType::NameList:
        case SyntaxTreeType::Function:
        case SyntaxTreeType::FuncNameList:
        case SyntaxTreeType::FuncName:
        case SyntaxTreeType::FuncBody:
        case SyntaxTreeType::FunctionDef:
        case SyntaxTreeType::LocalFunction:
        case SyntaxTreeType::Binop:
        case SyntaxTreeType::Unop:
        case SyntaxTreeType::Args:
        case SyntaxTreeType::PrefixExp: {
            break;
        }
        default: {
            ThrowFakeluaException(std::format("unexpected SyntaxTreeType in CheckNode: {}", SyntaxTreeTypeToString(node->Type())));
        }
    }
}

void SemanticAnalysis::CheckGotoOrLabel(const SyntaxTreeInterfacePtr &node) {
    ThrowError(node->Type() == SyntaxTreeType::Goto ? "goto is not supported" : "label is not supported", node);
}

void SemanticAnalysis::CheckFunctionCall(const SyntaxTreeInterfacePtr &node) {
    const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
    if (!fc->Name().empty()) {
        ThrowError("method calls (:) are not supported", node);
    }
    const auto callee_prefixexp = fc->prefixexp();
    if (!callee_prefixexp || callee_prefixexp->Type() != SyntaxTreeType::PrefixExp) {
        ThrowError("function call callee must be a prefix expression", node);
    }
    const auto callee_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(callee_prefixexp);
    if (callee_pe->GetPrefixKind() != PrefixExpKind::kVar) {
        ThrowError("function call callee must be a variable", node);
    }
    const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(callee_pe->GetValue());
    if (!callee_var || callee_var->GetVarKind() != VarKind::kSimple) {
        ThrowError("function call callee must be a simple variable", node);
    }
    if (callee_var->GetName() == "FAKELUA_SET_TABLE") {
        const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
        if (!args_ptr || args_ptr->GetArgsKind() != ArgsKind::kExpList) {
            ThrowError("FAKELUA_SET_TABLE expects exactly 3 arguments", node);
        }
        const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(args_ptr->Explist());
        if (!explist_ptr || explist_ptr->Exps().size() != 3) {
            ThrowError("FAKELUA_SET_TABLE expects exactly 3 arguments", node);
        }
    }
}

void SemanticAnalysis::CheckParList(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar) {
    const auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(node);
    if (parlist->VarParams()) {
        ThrowError("varargs (...) is not supported", node);
    }
    const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(parlist->Namelist());
    if (namelist) {
        std::set<std::string> param_names_set;
        for (const auto &name: ar.global_const_names) {
            param_names_set.insert(name);
        }
        for (const auto &name: namelist->Names()) {
            if (param_names_set.contains(name)) {
                ThrowError("the param name is duplicated: " + name, namelist);
            }
            param_names_set.insert(name);
        }
    }
    if (const size_t param_size = namelist ? namelist->Names().size() : 0; param_size > kMaxFunctionInputParams) {
        ThrowError(std::format("function input parameters exceed limit {}, got {}", kMaxFunctionInputParams, param_size), node);
    }
}

void SemanticAnalysis::CheckLocalVar(const SyntaxTreeInterfacePtr &node, const AnalysisResult &ar) {
    const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
    const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
    if (!namelist) {
        ThrowError("local variable namelist is missing", node);
    }
    if (!top_level_stmts_.contains(node.get())) {
        if (namelist) {
            for (const auto &name: namelist->Names()) {
                if (ar.global_const_names.contains(name)) {
                    ThrowError("local variable conflicts with global constant: " + name, node);
                }
            }
        }
    }
}

void SemanticAnalysis::CheckForLoop(const SyntaxTreeInterfacePtr &node) {
    const auto for_loop = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
    if (const auto step_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(for_loop->ExpStep())) {
        if (step_exp->GetExpKind() == ExpKind::kNumber) {
            const auto &val = step_exp->ExpValue();
            if (IsInteger(val)) {
                if (ToInteger(val) == 0) {
                    ThrowError("'for' step is zero", step_exp);
                }
            } else {
                if (ToFloat(val) == 0.0) {
                    ThrowError("'for' step is zero", step_exp);
                }
            }
        }
    }
}

void SemanticAnalysis::CheckForIn(const SyntaxTreeInterfacePtr &node) {
    const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);

    const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(for_in->Namelist());
    if (namelist && (namelist->Names().empty() || namelist->Names().size() > 2)) {
        ThrowError(std::format("for in namelist size must be 1 or 2, but got {}", namelist->Names().size()), node);
    }

    const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(for_in->Explist());
    if (explist) {
        if (explist->Exps().size() != 1) {
            ThrowError(std::format("for in explist size must be 1, but got {}", explist->Exps().size()), node);
        }

        const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(explist->Exps()[0]);
        if (!exp || exp->GetExpKind() != ExpKind::kPrefixExp) {
            ThrowError("for in expression must be a pairs() or ipairs() call", node);
        }
        const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(exp->Right());
        if (!pe || pe->GetPrefixKind() != PrefixExpKind::kFunctionCall) {
            ThrowError("for in expression must be a function call", node);
        }
        const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(pe->GetValue());
        if (!fc) {
            ThrowError("for in: function call node is missing", node);
        }
        const auto func_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
        if (!func_pe || func_pe->GetPrefixKind() != PrefixExpKind::kVar) {
            ThrowError("for in: only pairs() or ipairs() are supported", node);
        }
        const auto func_var = std::dynamic_pointer_cast<SyntaxTreeVar>(func_pe->GetValue());
        if (!func_var || (func_var->GetName() != "pairs" && func_var->GetName() != "ipairs")) {
            ThrowError(std::format("for in: only pairs() or ipairs() are supported, got '{}'", func_var ? func_var->GetName() : ""), node);
        }
        const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
        if (!args_ptr || args_ptr->GetArgsKind() != ArgsKind::kExpList) {
            ThrowError("for in: pairs/ipairs argument must be an expression list", node);
        }
        const auto args_explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(args_ptr->Explist());
        if (!args_explist_ptr || args_explist_ptr->Exps().size() != 1) {
            ThrowError(std::format("for in: pairs/ipairs must have exactly one argument, got {}",
                                   args_explist_ptr ? args_explist_ptr->Exps().size() : 0),
                       node);
        }
    }
}

void SemanticAnalysis::CheckExp(const SyntaxTreeInterfacePtr &node) {
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
    const auto kind = exp->GetExpKind();
    if (kind == ExpKind::kVarParams) {
        ThrowError("... is not supported", node);
    } else if (kind == ExpKind::kFunctionDef) {
        ThrowError("anonymous function expression (functiondef) is not supported inside function bodies", node);
    }
}

void SemanticAnalysis::CheckGlobalConstExp(const SyntaxTreeInterfacePtr &exp) {
    DEBUG_ASSERT(exp && exp->Type() == SyntaxTreeType::Exp);
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto exp_kind = e->GetExpKind();
    if (exp_kind == ExpKind::kTableConstructor) {
        ThrowError("table constructor is not supported in global variable initialization", exp);
    } else if (exp_kind == ExpKind::kBinop) {
        ThrowError("binary operator is not supported in global variable initialization", exp);
    } else if (exp_kind == ExpKind::kUnop) {
        ThrowError("unary operator is not supported in global variable initialization", exp);
    } else if (exp_kind == ExpKind::kPrefixExp) {
        const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e->Right());
        if (pe) {
            if (pe->GetPrefixKind() == PrefixExpKind::kVar) {
                ThrowError("variable reference is not allowed in global variable initialization", exp);
            } else if (pe->GetPrefixKind() == PrefixExpKind::kFunctionCall) {
                ThrowError("function call is not allowed in global variable initialization", exp);
            }
        }
    }
}

[[noreturn]] void SemanticAnalysis::ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr) {
    ThrowFakeluaException(std::format("SemanticAnalysis check failed, {} at {}", msg, LocationStr(ptr)));
}

std::string SemanticAnalysis::LocationStr(const SyntaxTreeInterfacePtr &ptr) {
    return std::format("{}:{}:{}", file_name_, ptr->Loc().begin.line, ptr->Loc().begin.column);
}

}// namespace fakelua
