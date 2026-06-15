#include "compile/semantic_analysis.h"
#include "state/state.h"
#include "util/common.h"
#include "util/exception.h"
#include <algorithm>

namespace fakelua {

SemanticAnalysis::SemanticAnalysis(State *s) : s_(s) {
}

AnalysisResult SemanticAnalysis::Analyze(const ParseResult &pr, const CompileConfig &cfg) {
    AnalysisResult ar;
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
                    max_returns = -1; // dynamic
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
            ThrowFakeluaException(std::format("unknown syntax tree type in CollectReturnsForBlock: {}", 
                                              static_cast<int>(node->Type())));
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

} // namespace fakelua
