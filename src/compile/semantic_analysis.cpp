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
            if (!namelist) {
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
            ThrowFakeluaException(std::format("unexpected non-statement syntax tree type in CollectReturnsForBlock: {}", SyntaxTreeTypeToString(node->Type())));
        }
        default: {
            ThrowFakeluaException(std::format("unknown syntax tree type in CollectReturnsForBlock: {}", static_cast<int>(node->Type())));
        }
    }
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
            if (!el) {
                ThrowError("global constant must be initialized", stmt);
            }
            bool last_is_func = !el->Exps().empty() && IsFunctionCallExp(el->Exps().back());
            if (namelist->Names().size() != el->Exps().size() && !(last_is_func && namelist->Names().size() > el->Exps().size())) {
                ThrowError(std::format("local variable count {} not match expression count {}", namelist->Names().size(), el->Exps().size()), stmt);
            }
            for (const auto &exp: el->Exps()) {
                CheckGlobalConstExp(exp);
            }
        }
    }

    WalkSyntaxTree(chunk, [this, &ar](const SyntaxTreeInterfacePtr &node) { CheckNode(node, ar); });
    std::unordered_map<std::string, SyntaxTreeInterfacePtr> visible_labels;
    ValidateGotoInBlock(chunk, visible_labels);
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
    // 不再直接拒绝，具体验证在 ValidateGotoInBlock 中完成
}

void SemanticAnalysis::CollectBlockLabels(const SyntaxTreeInterfacePtr &block, std::unordered_map<std::string, SyntaxTreeInterfacePtr> &labels) {
    const auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(block);
    if (!blk) return;
    for (const auto &stmt : blk->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::Label) {
            const auto label = std::dynamic_pointer_cast<SyntaxTreeLabel>(stmt);
            labels[label->GetName()] = stmt;
        }
    }
}

void SemanticAnalysis::ValidateGotoInBlock(const SyntaxTreeInterfacePtr &chunk, std::unordered_map<std::string, SyntaxTreeInterfacePtr> visible_labels) {
    const auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
    if (!blk) return;

    // 收集当前 block 自身的 label 并加入可见集合
    CollectBlockLabels(chunk, visible_labels);

    // 收集当前 block 的局部变量声明位置
    std::vector<size_t> local_positions;
    for (size_t i = 0; i < blk->Stmts().size(); ++i) {
        if (blk->Stmts()[i]->Type() == SyntaxTreeType::LocalVar) {
            local_positions.push_back(i);
        }
    }

    // 检查 goto
    for (size_t i = 0; i < blk->Stmts().size(); ++i) {
        const auto &stmt = blk->Stmts()[i];
        if (stmt->Type() == SyntaxTreeType::Goto) {
            const auto goto_stmt = std::dynamic_pointer_cast<SyntaxTreeGoto>(stmt);
            const auto &target_name = goto_stmt->GetLabel();
            auto it = visible_labels.find(target_name);
            if (it == visible_labels.end()) {
                ThrowError(std::format("goto target '{}' not found", target_name), stmt);
            }
            // 检查 label 是否在当前 block 内（若在，则检查是否跳过局部变量）
            size_t label_pos = blk->Stmts().size();
            for (size_t j = 0; j < blk->Stmts().size(); ++j) {
                if (blk->Stmts()[j].get() == it->second.get()) {
                    label_pos = j;
                    break;
                }
            }
            if (i < label_pos && label_pos < blk->Stmts().size()) {
                for (auto lp : local_positions) {
                    if (lp > i && lp <= label_pos) {
                        ThrowError(std::format("goto '{}' jumps over local variable declaration", target_name), stmt);
                    }
                }
            }
        }
    }

    // 递归检查嵌套 block，传递可见 label 集合
    for (const auto &stmt : blk->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::Block) {
            ValidateGotoInBlock(stmt, visible_labels);
        } else if (stmt->Type() == SyntaxTreeType::While) {
            ValidateGotoInBlock(std::dynamic_pointer_cast<SyntaxTreeWhile>(stmt)->Block(), visible_labels);
        } else if (stmt->Type() == SyntaxTreeType::Repeat) {
            ValidateGotoInBlock(std::dynamic_pointer_cast<SyntaxTreeRepeat>(stmt)->Block(), visible_labels);
        } else if (stmt->Type() == SyntaxTreeType::If) {
            const auto if_stmt = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);
            ValidateGotoInBlock(if_stmt->Block(), visible_labels);
            if (if_stmt->ElseIfs()) {
                const auto elseif_list = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(if_stmt->ElseIfs());
                if (elseif_list) {
                    for (const auto &elseif_blk : elseif_list->ElseifBlocks()) {
                        ValidateGotoInBlock(elseif_blk, visible_labels);
                    }
                }
            }
            if (if_stmt->ElseBlock()) ValidateGotoInBlock(if_stmt->ElseBlock(), visible_labels);
        } else if (stmt->Type() == SyntaxTreeType::ForLoop) {
            ValidateGotoInBlock(std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt)->Block(), visible_labels);
        } else if (stmt->Type() == SyntaxTreeType::ForIn) {
            ValidateGotoInBlock(std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt)->Block(), visible_labels);
        } else if (stmt->Type() == SyntaxTreeType::Function) {
            std::unordered_map<std::string, SyntaxTreeInterfacePtr> func_labels;
            const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(
                std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt)->Funcbody());
            if (fb) ValidateGotoInBlock(fb->Block(), func_labels);
        } else if (stmt->Type() == SyntaxTreeType::LocalFunction) {
            std::unordered_map<std::string, SyntaxTreeInterfacePtr> func_labels;
            const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(
                std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt)->Funcbody());
            if (fb) ValidateGotoInBlock(fb->Block(), func_labels);
        }
    }
}

void SemanticAnalysis::CheckFunctionCall(const SyntaxTreeInterfacePtr &node) {
    const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
    const auto callee_prefixexp = fc->prefixexp();
    if (!callee_prefixexp || callee_prefixexp->Type() != SyntaxTreeType::PrefixExp) {
        ThrowError("function call callee must be a prefix expression", node);
    }
    const auto callee_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(callee_prefixexp);
    if (callee_pe->GetPrefixKind() == PrefixExpKind::kVar) {
        const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(callee_pe->GetValue());
        if (callee_var && callee_var->GetVarKind() == VarKind::kSimple && callee_var->GetName() == "FAKELUA_SET_TABLE") {
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
    if (!namelist || namelist->Names().empty()) {
        ThrowError("for in loop requires at least one variable name", node);
    }

    const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(for_in->Explist());
    if (!explist || explist->Exps().empty()) {
        ThrowError("for in loop requires an expression list", node);
    }
}

void SemanticAnalysis::CheckExp(const SyntaxTreeInterfacePtr &node) {
    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
    const auto kind = exp->GetExpKind();
    if (kind == ExpKind::kVarParams) {
        ThrowError("... is not supported", node);
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
    ThrowFakeluaException(std::format("SemanticAnalysis check failed, {} at {}", msg, SyntaxTreeLocationStr(file_name_, ptr)));
}

}// namespace fakelua
