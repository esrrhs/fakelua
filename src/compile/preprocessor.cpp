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

    // 预处理全局变量初始化
    PreprocessGlobalInitializers(chunk);

    // 替换可变参数 "..."
    PreprocessVarargs(chunk);

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

            // 如果赋值语句中变量数量和表达式数量不匹配，且最后不是函数调用或可变参数，则抛出异常
            bool last_is_func = !exps.empty() && IsFunctionCallExp(exps.back());
            bool last_is_vararg = !exps.empty() && IsVarargExp(exps.back());
            if (vars.size() != exps.size() && !((last_is_func || last_is_vararg) && vars.size() > exps.size())) {
                ThrowError(std::format("PreprocessSplitAssigns: assign stmt var count {} not match exp count {}", vars.size(), exps.size()), explist_ptr);
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
                ThrowError(std::format("PreprocessTableAssigns: assign stmt var count {} or exp count {} not match 1", vars.size(), exps.size()), stmt);
            }

            auto &var = vars[0];
            auto &exp = exps[0];

            // 检查是不是赋值的table
            if (const auto var_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(var); var_ptr->GetVarKind() == VarKind::kSquare || var_ptr->GetVarKind() == VarKind::kDot) {

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
                if (const auto init_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(el->Exps()[0]); init_exp && init_exp->GetExpKind() == ExpKind::kFunctionDef) {
                    if (const auto fdef = std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(init_exp->Right())) {
                        const auto local_func = std::make_shared<SyntaxTreeLocalFunction>(stmt->Loc());
                        local_func->SetName(nl->Names()[0]);
                        local_func->SetFuncbody(fdef->Funcbody());
                        new_stmts.push_back(local_func);
                        LOG_INFO("PreprocessFunctiondefLocalVars: converted local {} = function(...) to local function {}(...)", nl->Names()[0], nl->Names()[0]);
                        continue;
                    }
                }
            }
        }
        new_stmts.push_back(stmt);
    }
    top_block->SetStmts(new_stmts);
}

void PreProcessor::PreprocessVarargs(const SyntaxTreeInterfacePtr &chunk) {
    LOG_INFO("start PreprocessVarargs");
    WalkSyntaxTree(chunk, [this](const SyntaxTreeInterfacePtr &node) {
        if (!node) return;

        SyntaxTreeInterfacePtr parlist_node = nullptr;
        SyntaxTreeInterfacePtr block_node = nullptr;

        if (node->Type() == SyntaxTreeType::Function) {
            const auto f = std::dynamic_pointer_cast<SyntaxTreeFunction>(node);
            const auto fbody = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(f->Funcbody());
            if (fbody) {
                parlist_node = fbody->Parlist();
                block_node = fbody->Block();
            }
        } else if (node->Type() == SyntaxTreeType::FunctionDef) {
            const auto fdef = std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(node);
            const auto fbody = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(fdef->Funcbody());
            if (fbody) {
                parlist_node = fbody->Parlist();
                block_node = fbody->Block();
            }
        } else if (node->Type() == SyntaxTreeType::LocalFunction) {
            const auto lf = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node);
            const auto fbody = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(lf->Funcbody());
            if (fbody) {
                parlist_node = fbody->Parlist();
                block_node = fbody->Block();
            }
        }

        if (!parlist_node) return;

        const auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(parlist_node);
        if (!parlist || !parlist->VarParams()) return;

        // 发现可变参数函数！
        // 1. 生成唯一的隐式变量名
        std::string vararg_name = std::format("__fakelua_vararg_{}", tmp_var_counter_++);

        // 2. 将此变量加入参数列表中的 namelist
        auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(parlist->Namelist());
        if (!namelist) {
            namelist = std::make_shared<SyntaxTreeNamelist>(parlist->Loc());
            parlist->SetNamelist(namelist);
        }
        namelist->AddName(vararg_name);

        // 3. 清除变参标记，防止后续报错
        parlist->SetVarParams(false);

        // 4. 重写函数体内的所有 "..."
        if (block_node) {
            WalkSyntaxTreePruned(block_node, [this, &vararg_name](const SyntaxTreeInterfacePtr &sub_node) -> bool {
                if (!sub_node) return false;

                // 遇到嵌套函数体，检查它是否自身为可变参数函数
                if (sub_node->Type() == SyntaxTreeType::FuncBody) {
                    const auto sub_fbody = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(sub_node);
                    const auto sub_parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(sub_fbody->Parlist());
                    if (sub_parlist && sub_parlist->VarParams()) {
                        // 嵌套的变参函数在此剪枝，不要继续深入它的内部
                        return false;
                    }
                }

                // 找到 `...` 表达式进行替换
                if (sub_node->Type() == SyntaxTreeType::Exp) {
                    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(sub_node);
                    if (exp->GetExpKind() == ExpKind::kVarParams) {
                        exp->SetExpKind(ExpKind::kPrefixExp);
                        auto pe = MakeSimpleVarPrefixexp(exp->Loc(), vararg_name);
                        exp->SetRight(pe);
                        return false; // 不需要再向下递归此 Exp 节点的子节点
                    }
                }

                return true;
            });
        }
    });
    LOG_INFO("end PreprocessVarargs");
}

bool PreProcessor::IsComplexExp(const SyntaxTreeInterfacePtr &exp) {
    if (!exp || exp->Type() != SyntaxTreeType::Exp) {
        return false;
    }
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto exp_kind = e->GetExpKind();
    if (exp_kind == ExpKind::kTableConstructor ||
        exp_kind == ExpKind::kBinop ||
        exp_kind == ExpKind::kUnop) {
        return true;
    }
    if (exp_kind == ExpKind::kPrefixExp) {
        const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e->Right());
        if (pe) {
            if (pe->GetPrefixKind() == PrefixExpKind::kVar ||
                pe->GetPrefixKind() == PrefixExpKind::kFunctionCall) {
                return true;
            }
        }
    }
    return false;
}

std::shared_ptr<SyntaxTreeFunction> PreProcessor::MakeInitFunction(const SyntaxTreeLocation &loc, const std::vector<SyntaxTreeInterfacePtr> &assign_stmts) {
    auto fnlist = std::make_shared<SyntaxTreeFuncnamelist>(loc);
    fnlist->AddName("__fakelua_init");

    auto fname = std::make_shared<SyntaxTreeFuncname>(loc);
    fname->SetFuncNameList(fnlist);

    auto block = std::make_shared<SyntaxTreeBlock>(loc);
    block->SetStmts(assign_stmts);

    auto fbody = std::make_shared<SyntaxTreeFuncbody>(loc);
    fbody->SetBlock(block);

    auto func = std::make_shared<SyntaxTreeFunction>(loc);
    func->SetFuncname(fname);
    func->SetFuncbody(fbody);

    return func;
}

void PreProcessor::PreprocessGlobalInitializers(const SyntaxTreeInterfacePtr &chunk) {
    LOG_INFO("start PreprocessGlobalInitializers");
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    const auto top_block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);

    std::vector<SyntaxTreeInterfacePtr> init_assign_stmts;
    std::vector<SyntaxTreeInterfacePtr> new_stmts;

    for (const auto &stmt : top_block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::LocalVar) {
            const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
            const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist());
            const auto explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(lv->Explist());

            if (namelist && explist) {
                auto &names = namelist->Names();
                auto &exps = explist->Exps();

                bool has_complex = false;
                for (const auto &exp : exps) {
                    if (IsComplexExp(exp)) {
                        has_complex = true;
                        break;
                    }
                }

                bool last_is_func = !exps.empty() && IsFunctionCallExp(exps.back());
                bool last_is_vararg = !exps.empty() && IsVarargExp(exps.back());
                if (names.size() != exps.size() && (last_is_func || last_is_vararg)) {
                    has_complex = true;
                }

                if (has_complex) {
                    if (names.size() == exps.size()) {
                        for (size_t i = 0; i < names.size(); ++i) {
                            if (IsComplexExp(exps[i])) {
                                auto assign = std::make_shared<SyntaxTreeAssign>(stmt->Loc());
                                auto varlist = std::make_shared<SyntaxTreeVarlist>(stmt->Loc());
                                auto var = std::make_shared<SyntaxTreeVar>(stmt->Loc());
                                var->SetName(names[i]);
                                var->SetVarKind(VarKind::kSimple);
                                varlist->AddVar(var);
                                assign->SetVarlist(varlist);

                                auto single_explist = std::make_shared<SyntaxTreeExplist>(stmt->Loc());
                                single_explist->AddExp(exps[i]);
                                assign->SetExplist(single_explist);

                                init_assign_stmts.push_back(assign);

                                auto nil_exp = std::make_shared<SyntaxTreeExp>(stmt->Loc());
                                nil_exp->SetExpKind(ExpKind::kNil);
                                exps[i] = nil_exp;
                            }
                        }
                    } else {
                        auto assign = std::make_shared<SyntaxTreeAssign>(stmt->Loc());
                        auto varlist = std::make_shared<SyntaxTreeVarlist>(stmt->Loc());
                        for (const auto &name : names) {
                            auto var = std::make_shared<SyntaxTreeVar>(stmt->Loc());
                            var->SetName(name);
                            var->SetVarKind(VarKind::kSimple);
                            varlist->AddVar(var);
                        }
                        assign->SetVarlist(varlist);
                        assign->SetExplist(explist);

                        init_assign_stmts.push_back(assign);

                        std::vector<SyntaxTreeInterfacePtr> nil_exps;
                        for (size_t i = 0; i < names.size(); ++i) {
                            auto nil_exp = std::make_shared<SyntaxTreeExp>(stmt->Loc());
                            nil_exp->SetExpKind(ExpKind::kNil);
                            nil_exps.push_back(nil_exp);
                        }
                        exps = nil_exps;
                    }
                }
            }
        }
        new_stmts.push_back(stmt);
    }

    if (!init_assign_stmts.empty()) {
        auto init_func = MakeInitFunction(chunk->Loc(), init_assign_stmts);
        new_stmts.push_back(init_func);
        LOG_INFO("PreprocessGlobalInitializers: generated __fakelua_init function with {} initializers", init_assign_stmts.size());
    }

    top_block->SetStmts(new_stmts);
    LOG_INFO("end PreprocessGlobalInitializers");
}

}// namespace fakelua
