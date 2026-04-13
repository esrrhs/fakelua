#include "compile/preprocessor.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "util/exception.h"
#include "util/file_util.h"

namespace fakelua {

PreProcessor::PreProcessor(State *s) : s_(s) {
}

void PreProcessor::Process(const CompileResult &cr, const CompileConfig &cfg) {
    LOG_INFO("start PreProcessor::process {}", cr.file_name);

    const auto chunk = cr.chunk;
    file_name_ = cr.file_name;

    int debug_step = 0;

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

    LOG_INFO("end PreProcessor::compile {}", cr.file_name);
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
                    // 使用行号和索引生成唯一的临时变量名
                    std::string tmp_name = std::format("__fakelua_tmp_{}_{}", stmt->Loc().begin.line, i);
                    tmp_namelist->AddName(tmp_name);
                    tmp_explist->AddExp(exps[i]);
                    tmp_names.push_back(tmp_name);
                }

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
                    tmp_exp->SetType("prefixexp");
                    
                    const auto tmp_prefix = std::make_shared<SyntaxTreePrefixexp>(stmt->Loc());
                    tmp_prefix->SetType("var");
                    const auto tmp_var = std::make_shared<SyntaxTreeVar>(stmt->Loc());
                    tmp_var->SetType("simple");
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
                var_ptr->GetType() == "square" || var_ptr->GetType() == "dot") {

                // 转为函数调用，a.b.c = 1 -> FAKELUA_SET_TABLE(a.b, "c", 1)
                const auto func_call = std::make_shared<SyntaxTreeFunctioncall>(stmt->Loc());
                const auto prefixexp = std::make_shared<SyntaxTreePrefixexp>(stmt->Loc());
                const auto CallVar = std::make_shared<SyntaxTreeVar>(stmt->Loc());
                CallVar->SetType("simple");
                CallVar->SetName("FAKELUA_SET_TABLE");
                prefixexp->SetType("var");
                prefixexp->SetValue(CallVar);
                func_call->SetPrefixexp(prefixexp);

                const auto args = std::make_shared<SyntaxTreeArgs>(stmt->Loc());
                const auto args_explist = std::make_shared<SyntaxTreeExplist>(stmt->Loc());

                // a.b
                {
                    const auto args_exp = std::make_shared<SyntaxTreeExp>(stmt->Loc());
                    const auto args_exp_prefixexp = var_ptr->GetPrefixexp();
                    args_exp->SetType("prefixexp");
                    args_exp->SetRight(args_exp_prefixexp);
                    args_explist->AddExp(args_exp);
                }

                // "c"
                {
                    if (var_ptr->GetType() == "square") {
                        const auto args_exp = var_ptr->GetExp();
                        args_explist->AddExp(args_exp);
                    } else {
                        const auto args_exp = std::make_shared<SyntaxTreeExp>(stmt->Loc());
                        args_exp->SetType("string");
                        args_exp->SetValue(var_ptr->GetName());
                        args_explist->AddExp(args_exp);
                    }
                }

                // 1
                {
                    args_explist->AddExp(exp);
                }

                args->SetExplist(args_explist);
                args->SetType("explist");
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

}// namespace fakelua
