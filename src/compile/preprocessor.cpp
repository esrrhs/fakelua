#include "compile/preprocessor.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "util/exception.h"
#include "util/file_util.h"

namespace fakelua {

PreProcessor::PreProcessor(State *s) : s_(s) {
}

void PreProcessor::Process(const ParseResult &pr, const CompileConfig &cfg) {
    LOG_INFO("start PreProcessor::process {}", pr.file_name);

    const auto chunk = pr.chunk;
    file_name_ = pr.file_name;

    int debug_step = 0;

    // 将顶层 "local f = function(...) ... end" 转换为 "local function f(...) ... end"，
    // 使其能被后续特化发现流程识别。必须在 CheckUnsupportedSyntax 之前执行，
    // 否则 CheckNode 会因遇到 functiondef Exp 节点而报错。
    PreprocessFunctiondefLocalVars(chunk);

    // 检测不支持的语法，提前抛出异常
    CheckUnsupportedSyntax(chunk);

    // 拆分多赋值语句 比如 'a, b = 1, 2' to 'a = 1; b = 2;'
    PreprocessSplitAssigns(chunk);
    if (cfg.debug_mode) {
        DumpDebugFile(chunk, debug_step++);
    }

    // 替换table赋值语句 比如 'a.b.c = 1' to 'FAKELUA_SET_TABLE(a.b, "c", 1)'
    PreprocessTableAssigns(chunk);
    if (cfg.debug_mode) {
        DumpDebugFile(chunk, debug_step);
    }

    LOG_INFO("end PreProcessor::compile {}", pr.file_name);
}

void PreProcessor::DumpDebugFile(const SyntaxTreeInterfacePtr &chunk, int step) {
    const auto dumpfile = GenerateTmpFilename("fakelua_debug_", std::format(".pre{}", step));
    std::ofstream file(dumpfile);
    DEBUG_ASSERT(file.is_open());
    file << chunk->Dump(0);
    file.close();
}

[[noreturn]] void PreProcessor::ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr) {
    ThrowFakeluaException(std::format("PreProcess file failed, {} at {}", msg, LocationStr(ptr)));
}

std::string PreProcessor::LocationStr(const SyntaxTreeInterfacePtr &ptr) {
    return std::format("{}:{}:{}", file_name_, ptr->Loc().begin.line, ptr->Loc().begin.column);
}

void PreProcessor::PreprocessSplitAssigns(const SyntaxTreeInterfacePtr &chunk) {
    LOG_INFO("start PreprocessSplitAssigns");
    WalkSyntaxTree(chunk, [this](const SyntaxTreeInterfacePtr &node) { PreprocessSplitAssign(node); });
}

void PreProcessor::PreprocessSplitAssign(const SyntaxTreeInterfacePtr &node) {
    if (!node || node->Type() != SyntaxTreeType::Block) {
        return;
    }

    const auto block_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
    std::vector<SyntaxTreeInterfacePtr> new_stmts;
    for (const auto stmts = block_ptr->Stmts(); auto &stmt: stmts) {
        if (stmt->Type() == SyntaxTreeType::Assign) {
            const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);
            const auto varlist = assign->Varlist();
            const auto explist = assign->Explist();
            DEBUG_ASSERT(varlist->Type() == SyntaxTreeType::VarList);
            DEBUG_ASSERT(explist->Type() == SyntaxTreeType::ExpList);
            const auto varlist_ptr = std::dynamic_pointer_cast<SyntaxTreeVarlist>(varlist);
            const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
            auto &vars = varlist_ptr->Vars();
            auto &exps = explist_ptr->Exps();

            // 如果赋值语句中变量数量和表达式数量不匹配，则抛出异常
            if (vars.size() != exps.size()) {
                ThrowError(std::format("PreprocessSplitAssigns: assign stmt var count {} not match exp count {}", vars.size(), exps.size()),
                           explist_ptr);
            }

            if (vars.size() == 1) {
                new_stmts.push_back(stmt);
            } else {
                // 1. 先生成临时变量名列表
                const auto tmp_namelist = std::make_shared<SyntaxTreeNamelist>(stmt->Loc());
                const auto tmp_explist = std::make_shared<SyntaxTreeExplist>(stmt->Loc());
                std::vector<std::string> tmp_names;

                for (size_t i = 0; i < vars.size(); ++i) {
                    // 使用实例递增计数器（而非行号）生成唯一的临时变量名，
                    // 避免同一行多条多重赋值语句产生名称冲突。
                    std::string tmp_name = std::format("__fakelua_tmp_{}_{}", tmp_var_counter_, i);
                    tmp_namelist->AddName(tmp_name);
                    tmp_explist->AddExp(exps[i]);
                    tmp_names.push_back(tmp_name);
                }
                ++tmp_var_counter_;

                // 2. 创建局部变量定义语句：local tmp1, tmp2, ... = exp1, exp2, ...
                const auto local_var_stmt = std::make_shared<SyntaxTreeLocalVar>(stmt->Loc());
                local_var_stmt->SetNamelist(tmp_namelist);
                local_var_stmt->SetExplist(tmp_explist);
                new_stmts.push_back(local_var_stmt);

                // 3. 为每个变量生成单赋值语句：vars[i] = tmp_names[i]
                for (size_t i = 0; i < vars.size(); ++i) {
                    const auto new_assign = std::make_shared<SyntaxTreeAssign>(stmt->Loc());

                    const auto single_varlist = std::make_shared<SyntaxTreeVarlist>(stmt->Loc());
                    single_varlist->AddVar(vars[i]);
                    new_assign->SetVarlist(single_varlist);

                    const auto single_explist = std::make_shared<SyntaxTreeExplist>(stmt->Loc());
                    const auto tmp_exp = std::make_shared<SyntaxTreeExp>(stmt->Loc());
                    tmp_exp->SetExpKind(ExpKind::kPrefixExp);
                    
                    const auto tmp_prefix = std::make_shared<SyntaxTreePrefixexp>(stmt->Loc());
                    tmp_prefix->SetPrefixKind(PrefixExpKind::kVar);
                    const auto tmp_var = std::make_shared<SyntaxTreeVar>(stmt->Loc());
                    tmp_var->SetVarKind(VarKind::kSimple);
                    tmp_var->SetName(tmp_names[i]);
                    tmp_prefix->SetValue(tmp_var);
                    
                    tmp_exp->SetRight(tmp_prefix);
                    single_explist->AddExp(tmp_exp);
                    new_assign->SetExplist(single_explist);

                    new_stmts.push_back(new_assign);
                }
            }
        } else {
            new_stmts.push_back(stmt);
        }
    }
    block_ptr->SetStmts(new_stmts);
}

void PreProcessor::PreprocessTableAssigns(const SyntaxTreeInterfacePtr &chunk) {
    LOG_INFO("start PreprocessTableAssigns");
    WalkSyntaxTree(chunk, [this](const SyntaxTreeInterfacePtr &node) { PreprocessTableAssign(node); });
}

void PreProcessor::PreprocessTableAssign(const SyntaxTreeInterfacePtr &node) {
    if (!node || node->Type() != SyntaxTreeType::Block) {
        return;
    }

    const auto block_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
    std::vector<SyntaxTreeInterfacePtr> new_stmts;
    for (const auto stmts = block_ptr->Stmts(); auto &stmt: stmts) {
        if (stmt->Type() == SyntaxTreeType::Assign) {
            const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);
            const auto varlist = assign->Varlist();
            const auto explist = assign->Explist();
            DEBUG_ASSERT(varlist->Type() == SyntaxTreeType::VarList);
            DEBUG_ASSERT(explist->Type() == SyntaxTreeType::ExpList);
            const auto varlist_ptr = std::dynamic_pointer_cast<SyntaxTreeVarlist>(varlist);
            const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
            auto &vars = varlist_ptr->Vars();
            auto &exps = explist_ptr->Exps();

            // 此时应该都只有一个变量和一个表达式
            if (vars.size() != 1 || exps.size() != 1) {
                ThrowError(std::format("PreprocessTableAssigns: assign stmt var count {} or exp count {} not match 1", vars.size(),
                                       exps.size()),
                           stmt);
            }

            auto &var = vars[0];
            auto &exp = exps[0];

            // 检查是不是赋值的table
            if (const auto var_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(var);
                var_ptr->GetVarKind() == VarKind::kSquare || var_ptr->GetVarKind() == VarKind::kDot) {

                // 转为函数调用，a.b.c = 1 -> FAKELUA_SET_TABLE(a.b, "c", 1)
                const auto func_call = std::make_shared<SyntaxTreeFunctioncall>(stmt->Loc());
                const auto prefixexp = std::make_shared<SyntaxTreePrefixexp>(stmt->Loc());
                const auto CallVar = std::make_shared<SyntaxTreeVar>(stmt->Loc());
                CallVar->SetVarKind(VarKind::kSimple);
                CallVar->SetName("FAKELUA_SET_TABLE");
                prefixexp->SetPrefixKind(PrefixExpKind::kVar);
                prefixexp->SetValue(CallVar);
                func_call->SetPrefixexp(prefixexp);

                const auto args = std::make_shared<SyntaxTreeArgs>(stmt->Loc());
                const auto args_explist = std::make_shared<SyntaxTreeExplist>(stmt->Loc());

                // a.b
                {
                    const auto args_exp = std::make_shared<SyntaxTreeExp>(stmt->Loc());
                    const auto args_exp_prefixexp = var_ptr->GetPrefixexp();
                    args_exp->SetExpKind(ExpKind::kPrefixExp);
                    args_exp->SetRight(args_exp_prefixexp);
                    args_explist->AddExp(args_exp);
                }

                // "c"
                {
                    if (var_ptr->GetVarKind() == VarKind::kSquare) {
                        const auto args_exp = var_ptr->GetExp();
                        args_explist->AddExp(args_exp);
                    } else {
                        const auto args_exp = std::make_shared<SyntaxTreeExp>(stmt->Loc());
                        args_exp->SetExpKind(ExpKind::kString);
                        args_exp->SetValue(var_ptr->GetName());
                        args_explist->AddExp(args_exp);
                    }
                }

                // 1
                {
                    args_explist->AddExp(exp);
                }

                args->SetExplist(args_explist);
                args->SetArgsKind(ArgsKind::kExpList);
                func_call->SetArgs(args);

                new_stmts.push_back(func_call);
            } else {
                new_stmts.push_back(stmt);
            }
        } else {
            new_stmts.push_back(stmt);
        }
    }
    block_ptr->SetStmts(new_stmts);
}

void PreProcessor::CheckUnsupportedSyntax(const SyntaxTreeInterfacePtr &chunk) {
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);

    // 检查顶层 function 函数名约束，以及全局常量初始化表达式约束
    for (const auto &stmt: top_block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            if (const auto funcname = std::dynamic_pointer_cast<SyntaxTreeFuncname>(func->Funcname()); funcname) {
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
            if (const auto explist = lv->Explist()) {
                const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
                if (namelist->Names().size() != el->Exps().size()) {
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

    // 全量遍历 AST，检测所有其他不支持的语法
    WalkSyntaxTree(chunk, [this](const SyntaxTreeInterfacePtr &node) { CheckNode(node); });
}

void PreProcessor::CheckNode(const SyntaxTreeInterfacePtr &node) {
    switch (node->Type()) {
        case SyntaxTreeType::Goto:
            ThrowError("goto is not supported", node);
            break;
        case SyntaxTreeType::Label:
            ThrowError("label is not supported", node);
            break;
        case SyntaxTreeType::FunctionCall: {
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
            break;
        }
        case SyntaxTreeType::ParList: {
            const auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(node);
            if (parlist->VarParams()) {
                ThrowError("varargs (...) is not supported", node);
            }
            const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(parlist->Namelist());
            const size_t param_size = namelist ? namelist->Names().size() : 0;
            if (param_size > kMaxFunctionInputParams) {
                ThrowError(std::format("function input parameters exceed limit {}, got {}", kMaxFunctionInputParams, param_size), node);
            }
            break;
        }
        case SyntaxTreeType::Return: {
            const auto ret = std::dynamic_pointer_cast<SyntaxTreeReturn>(node);
            if (const auto explist = ret->Explist()) {
                const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
                if (el->Exps().size() > 1) {
                    ThrowError("multiple return values is not supported", node);
                }
            }
            break;
        }
        case SyntaxTreeType::ForIn: {
            const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);

            // 循环变量数量：1 或 2
            const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(for_in->Namelist());
            if (namelist && (namelist->Names().empty() || namelist->Names().size() > 2)) {
                ThrowError(std::format("for in namelist size must be 1 or 2, but got {}", namelist->Names().size()), node);
            }

            // 迭代器表达式必须恰好一个
            if (const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(for_in->Explist()); explist) {
                if (explist->Exps().size() != 1) {
                    ThrowError(std::format("for in explist size must be 1, but got {}", explist->Exps().size()), node);
                }

                // 迭代器表达式必须是 pairs(t) 或 ipairs(t) 调用
                const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(explist->Exps()[0]);
                if (!exp || exp->GetExpKind() != ExpKind::kPrefixExp) {
                    ThrowError("for in expression must be a pairs() or ipairs() call", node);
                } else {
                    const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(exp->Right());
                    if (!pe || pe->GetPrefixKind() != PrefixExpKind::kFunctionCall) {
                        ThrowError("for in expression must be a function call", node);
                    } else {
                        if (const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(pe->GetValue()); fc) {
                            const auto func_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
                            if (!func_pe || func_pe->GetPrefixKind() != PrefixExpKind::kVar) {
                                ThrowError("for in: only pairs() or ipairs() are supported", node);
                            } else {
                                const auto func_var = std::dynamic_pointer_cast<SyntaxTreeVar>(func_pe->GetValue());
                                if (!func_var || (func_var->GetName() != "pairs" && func_var->GetName() != "ipairs")) {
                                    ThrowError(std::format("for in: only pairs() or ipairs() are supported, got '{}'",
                                                           func_var ? func_var->GetName() : ""),
                                               node);
                                }
                                // 检查 pairs/ipairs 的参数
                                const auto args_node = fc->Args();
                                const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(args_node);
                                if (!args_ptr || args_ptr->GetArgsKind() != ArgsKind::kExpList) {
                                    ThrowError("for in: pairs/ipairs argument must be an expression list", node);
                                } else {
                                    const auto args_explist = args_ptr->Explist();
                                    const auto args_explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(args_explist);
                                    if (!args_explist_ptr || args_explist_ptr->Exps().size() != 1) {
                                        ThrowError(std::format("for in: pairs/ipairs must have exactly one argument, got {}",
                                                               args_explist_ptr ? args_explist_ptr->Exps().size() : 0),
                                                   node);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
        case SyntaxTreeType::Exp: {
            const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
            if (exp->GetExpKind() == ExpKind::kVarParams) {
                ThrowError("... is not supported", node);
            }
            if (exp->GetExpKind() == ExpKind::kFunctionDef) {
                ThrowError("anonymous function expression (functiondef) is not supported inside function bodies", node);
            }
            break;
        }
        default:
            break;
    }
}

void PreProcessor::CheckGlobalConstExp(const SyntaxTreeInterfacePtr &exp) {
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
        if (const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e->Right()); pe) {
            if (pe->GetPrefixKind() == PrefixExpKind::kVar) {
                ThrowError("variable reference is not allowed in global variable initialization", exp);
            } else if (pe->GetPrefixKind() == PrefixExpKind::kFunctionCall) {
                ThrowError("function call is not allowed in global variable initialization", exp);
            }
        }
    }
}

void PreProcessor::PreprocessFunctiondefLocalVars(const SyntaxTreeInterfacePtr &chunk) {
    LOG_INFO("start PreprocessFunctiondefLocalVars");
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);

    std::vector<SyntaxTreeInterfacePtr> new_stmts;
    for (const auto &stmt : top_block->Stmts()) {
        if (stmt->Type() != SyntaxTreeType::LocalVar) {
            new_stmts.push_back(stmt);
            continue;
        }
        const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
        const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
        const auto el_node = lv->Explist();
        if (!nl || !el_node) {
            new_stmts.push_back(stmt);
            continue;
        }
        const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(el_node);
        if (!el || nl->Names().size() != 1 || el->Exps().size() != 1) {
            new_stmts.push_back(stmt);
            continue;
        }
        const auto init_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(el->Exps()[0]);
        if (!init_exp || init_exp->GetExpKind() != ExpKind::kFunctionDef) {
            new_stmts.push_back(stmt);
            continue;
        }
        const auto fdef = std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(init_exp->Right());
        if (!fdef) {
            new_stmts.push_back(stmt);
            continue;
        }
        // 将 "local f = function(...) ... end" 转换为 "local function f(...) ... end"。
        const auto local_func = std::make_shared<SyntaxTreeLocalFunction>(stmt->Loc());
        local_func->SetName(nl->Names()[0]);
        local_func->SetFuncbody(fdef->Funcbody());
        new_stmts.push_back(local_func);
        LOG_INFO("PreprocessFunctiondefLocalVars: converted local {} = function(...) to local function {}(...)", nl->Names()[0], nl->Names()[0]);
    }
    top_block->SetStmts(new_stmts);
}

}// namespace fakelua
