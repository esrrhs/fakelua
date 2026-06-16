#include "compile/preprocessor.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "util/exception.h"
#include "util/file_util.h"

namespace fakelua {

bool PreProcessor::IsFunctionCallExp(const SyntaxTreeInterfacePtr &exp_node) {
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

std::shared_ptr<SyntaxTreePrefixexp> PreProcessor::MakeSimpleVarPrefixexp(const SyntaxTreeLocation &loc, const std::string &name) {
    auto var = std::make_shared<SyntaxTreeVar>(loc);
    var->SetVarKind(VarKind::kSimple);
    var->SetName(name);
    auto pe = std::make_shared<SyntaxTreePrefixexp>(loc);
    pe->SetPrefixKind(PrefixExpKind::kVar);
    pe->SetValue(var);
    return pe;
}

std::shared_ptr<SyntaxTreeExp> PreProcessor::MakePrefixexpExp(const SyntaxTreeLocation &loc, const SyntaxTreeInterfacePtr &pe) {
    auto exp = std::make_shared<SyntaxTreeExp>(loc);
    exp->SetExpKind(ExpKind::kPrefixExp);
    exp->SetRight(pe);
    return exp;
}

std::shared_ptr<SyntaxTreeExp> PreProcessor::MakeStringExp(const SyntaxTreeLocation &loc, const std::string &val) {
    auto exp = std::make_shared<SyntaxTreeExp>(loc);
    exp->SetExpKind(ExpKind::kString);
    exp->SetValue(val);
    return exp;
}

PreProcessor::PreProcessor(State *s) : s_(s) {
}

void PreProcessor::Process(const ParseResult &pr, const CompileConfig &cfg) {
    LOG_INFO("start PreProcessor::process {}", pr.file_name);

    const auto chunk = pr.chunk;
    file_name_ = pr.file_name;

    int debug_step = 0;

    // 将顶层 "local f = function(...) ... end" 转换为 "local function f(...) ... end"，
    // 使其能被后续特化发现流程识别。
    PreprocessFunctiondefLocalVars(chunk);

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
            const auto varlist_ptr = std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist());
            const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(assign->Explist());
            DEBUG_ASSERT(varlist_ptr && explist_ptr);
            auto &vars = varlist_ptr->Vars();
            auto &exps = explist_ptr->Exps();

            // 如果赋值语句中变量数量和表达式数量不匹配，且最后不是函数调用，则抛出异常
            bool last_is_func = !exps.empty() && IsFunctionCallExp(exps.back());
            if (vars.size() != exps.size() && !(last_is_func && vars.size() > exps.size())) {
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
                    std::string tmp_name = std::format("__fakelua_tmp_{}_{}", tmp_var_counter_, i);
                    tmp_namelist->AddName(tmp_name);
                    if (i < exps.size()) {
                        tmp_explist->AddExp(exps[i]);
                    }
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
                    single_explist->AddExp(MakePrefixexpExp(stmt->Loc(), MakeSimpleVarPrefixexp(stmt->Loc(), tmp_names[i])));
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
            const auto varlist_ptr = std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist());
            const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(assign->Explist());
            DEBUG_ASSERT(varlist_ptr && explist_ptr);
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
                func_call->SetPrefixexp(MakeSimpleVarPrefixexp(stmt->Loc(), "FAKELUA_SET_TABLE"));

                const auto args = std::make_shared<SyntaxTreeArgs>(stmt->Loc());
                const auto args_explist = std::make_shared<SyntaxTreeExplist>(stmt->Loc());

                // a.b
                args_explist->AddExp(MakePrefixexpExp(stmt->Loc(), var_ptr->GetPrefixexp()));

                // "c"
                if (var_ptr->GetVarKind() == VarKind::kSquare) {
                    args_explist->AddExp(var_ptr->GetExp());
                } else {
                    args_explist->AddExp(MakeStringExp(stmt->Loc(), var_ptr->GetName()));
                }

                // 1
                args_explist->AddExp(exp);

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

void PreProcessor::PreprocessFunctiondefLocalVars(const SyntaxTreeInterfacePtr &chunk) {
    LOG_INFO("start PreprocessFunctiondefLocalVars");
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);

    std::vector<SyntaxTreeInterfacePtr> new_stmts;
    for (const auto &stmt: top_block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::LocalVar) {
            const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
            const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
            const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(lv->Explist());
            if (nl && el && nl->Names().size() == 1 && el->Exps().size() == 1) {
                if (const auto init_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(el->Exps()[0]);
                    init_exp && init_exp->GetExpKind() == ExpKind::kFunctionDef) {
                    if (const auto fdef = std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(init_exp->Right())) {
                        const auto local_func = std::make_shared<SyntaxTreeLocalFunction>(stmt->Loc());
                        local_func->SetName(nl->Names()[0]);
                        local_func->SetFuncbody(fdef->Funcbody());
                        new_stmts.push_back(local_func);
                        LOG_INFO("PreprocessFunctiondefLocalVars: converted local {} = function(...) to local function {}(...)",
                                 nl->Names()[0], nl->Names()[0]);
                        continue;
                    }
                }
            }
        }
        new_stmts.push_back(stmt);
    }
    top_block->SetStmts(new_stmts);
}

}// namespace fakelua
