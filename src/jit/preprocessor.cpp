#include "preprocessor.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "util/exception.h"
#include "util/file_util.h"

namespace fakelua {

void PreProcessor::Process(const FakeluaStatePtr &sp, const CompileConfig &cfg, const std::string &file_name,
                           const SyntaxTreeInterfacePtr &chunk) {
    LOG_INFO("start PreProcessor::process {}", file_name);

    sp_ = sp;
    file_name_ = file_name;
    int debug_step = 0;

    // extracts all literal constants (e.g., integers, floats, strings),
    // adds them to the const definition, and replaces them with the corresponding const name.
    PreprocessExtractsLiteralConstants(chunk);
    if (cfg.debug_mode) {
        DumpDebugFile(chunk, debug_step++);
    }

    // make the const define 'a = 1' to 'a = nil', and add a new stmt 'a = 1' in function __fakelua_global_init__
    // because the const value is not supported function, so we need to make it to nil. and then assign.
    PreprocessConst(chunk);
    if (cfg.debug_mode) {
        DumpDebugFile(chunk, debug_step++);
    }

    // make the function name like 'function a.b.c()' to a temp name 'function __fakelua_global_1__()',
    // and add a new stmt 'xxx.yyy.zzz = "__fakelua_global_1__"' in function __fakelua_global_init__
    // also, we need to add 'self' in the front of the params if the function is a member function.
    PreprocessFunctionsName(chunk);
    if (cfg.debug_mode) {
        DumpDebugFile(chunk, debug_step++);
    }

    // now we have funtion __fakelua_global_init__, we need to add it to the chunk. we will insert more stmts to it later.
    SavePreprocessGlobalInit(chunk);
    if (cfg.debug_mode) {
        DumpDebugFile(chunk, debug_step++);
    }

    // change the table assign stmt like 'a.b.c = some_value' lvalue to temp name.
    // like 'local __fakelua_global_2__; __fakelua_global_2__ = some_value; __fakelua_set_table__(a.b, "c", __fakelua_global_2__)'
    // so we can easily always get the value of a.b.c as rvalue.
    PreprocessTableAssigns(chunk);
    if (cfg.debug_mode) {
        DumpDebugFile(chunk, debug_step++);
    }

    LOG_INFO("end PreProcessor::compile {}", file_name);
}

void PreProcessor::DumpDebugFile(const SyntaxTreeInterfacePtr &chunk, int step) {
    const auto dumpfile = GenerateTmpFilename("fakelua_gccjit_", std::format(".pre{}", step));
    std::ofstream file(dumpfile);
    DEBUG_ASSERT(file.is_open());
    file << chunk->Dump(0);
    file.close();
}

void PreProcessor::SavePreprocessGlobalInit(const SyntaxTreeInterfacePtr &chunk) {
    // add new function __fakelua_global_init__
    const auto funcname = std::make_shared<SyntaxTreeFuncname>(chunk->Loc());
    const auto funcnamelist = std::make_shared<SyntaxTreeFuncnamelist>(chunk->Loc());
    funcnamelist->AddName("__fakelua_global_init__");
    funcname->SetFuncNameList(funcnamelist);
    const auto funcbody = std::make_shared<SyntaxTreeFuncbody>(chunk->Loc());
    const auto block = std::make_shared<SyntaxTreeBlock>(chunk->Loc());
    for (auto &stmt: global_init_new_stmt_) {
        block->AddStmt(stmt);
    }
    funcbody->SetBlock(block);
    const auto func = std::make_shared<SyntaxTreeFunction>(chunk->Loc());
    func->SetFuncname(funcname);
    func->SetFuncbody(funcbody);
    const auto chunk_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
    chunk_ptr->AddStmt(func);
    global_init_new_stmt_.clear();
}

void PreProcessor::PreprocessConst(const SyntaxTreeInterfacePtr &chunk) {
    // the chunk must be a block
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    // walk through the block
    for (const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk); auto &stmt: block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::LocalVar) {
            PreprocessConstDefine(stmt);
        }
    }
}

void PreProcessor::PreprocessConstDefine(const SyntaxTreeInterfacePtr &stmt) {
    const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
    DEBUG_ASSERT(local_var->Namelist()->Type() == SyntaxTreeType::NameList);
    const auto keys = std::dynamic_pointer_cast<SyntaxTreeNamelist>(local_var->Namelist());
    auto &names = keys->Names();
    if (!local_var->Explist()) {
        ThrowError("the const define must have a value, but the value is null, it's useless", local_var);
    }
    DEBUG_ASSERT(local_var->Explist()->Type() == SyntaxTreeType::ExpList);
    const auto values = std::dynamic_pointer_cast<SyntaxTreeExplist>(local_var->Explist());
    auto &values_exps = values->Exps();

    for (size_t i = 0; i < names.size(); ++i) {
        auto name = names[i];
        if (i >= values_exps.size()) {
            ThrowError("the const define not match, the value is not enough", values);
        }

        const auto assign_stmt = std::make_shared<SyntaxTreeAssign>(stmt->Loc());
        const auto varlist = std::make_shared<SyntaxTreeVarlist>(stmt->Loc());
        const auto var = std::make_shared<SyntaxTreeVar>(stmt->Loc());
        var->SetType("simple");
        var->SetName(name);
        varlist->AddVar(var);
        assign_stmt->SetVarlist(varlist);
        const auto explist = std::make_shared<SyntaxTreeExplist>(stmt->Loc());
        explist->AddExp(values_exps[i]);
        assign_stmt->SetExplist(explist);

        // now replace the value to nil
        const auto nil_exp = std::make_shared<SyntaxTreeExp>(stmt->Loc());
        nil_exp->SetType("nil");
        values_exps[i] = nil_exp;

        global_init_new_stmt_.push_back(assign_stmt);
    }
}

void PreProcessor::PreprocessFunctionsName(const SyntaxTreeInterfacePtr &chunk) {
    // the chunk must be a block
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    // walk through the block
    for (const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk); auto &stmt: block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::Function) {
            auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            PreprocessFunctionName(func);
        }
    }
}

void PreProcessor::PreprocessFunctionName(const SyntaxTreeInterfacePtr &func) {
    DEBUG_ASSERT(func->Type() == SyntaxTreeType::Function);
    const auto func_ptr = std::dynamic_pointer_cast<SyntaxTreeFunction>(func);

    const auto funcname = std::dynamic_pointer_cast<SyntaxTreeFuncname>(func_ptr->Funcname());
    const auto funcnamelistptr = funcname->FuncNameList();

    const auto newfuncnamelistptr = std::make_shared<SyntaxTreeFuncnamelist>(funcnamelistptr->Loc());
    std::vector<std::string> newnamelist;

    DEBUG_ASSERT(funcnamelistptr->Type() == SyntaxTreeType::FuncNameList);
    const auto funcnamelist = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(funcnamelistptr);
    const auto &names = funcnamelist->Funcnames();
    newnamelist.insert(newnamelist.end(), names.begin(), names.end());

    if (!funcname->ColonName().empty()) {
        newnamelist.push_back(funcname->ColonName());
        // insert self in the front of params
        const auto funcbody = func_ptr->Funcbody();
        DEBUG_ASSERT(funcbody->Type() == SyntaxTreeType::FuncBody);
        const auto funcbody_ptr = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
        auto parlist = funcbody_ptr->Parlist();
        const auto new_parlist = std::make_shared<SyntaxTreeParlist>(funcbody_ptr->Loc());
        const auto namelist = std::make_shared<SyntaxTreeNamelist>(funcbody_ptr->Loc());
        namelist->AddName("self");
        if (parlist) {
            for (const auto old_namelist = std::dynamic_pointer_cast<SyntaxTreeParlist>(parlist)->Namelist();
                 auto &name: std::dynamic_pointer_cast<SyntaxTreeNamelist>(old_namelist)->Names()) {
                namelist->AddName(name);
            }
        }
        std::dynamic_pointer_cast<SyntaxTreeParlist>(new_parlist)->SetNamelist(namelist);
        parlist = new_parlist;
        funcbody_ptr->SetParlist(parlist);

        funcname->SetColonName("");
    }

    DEBUG_ASSERT(!newnamelist.empty());

    if (newnamelist.size() == 1) {
        // global function
        newfuncnamelistptr->AddName(newnamelist[0]);
        funcname->SetFuncNameList(newfuncnamelistptr);
    } else {
        // member function
        // xxx.yyy(), we need alloc special function name, and set the name to xxx.yyy
        const auto name = std::dynamic_pointer_cast<State>(sp_)->get_vm().AllocGlobalName();
        newfuncnamelistptr->AddName(name);
        funcname->SetFuncNameList(newfuncnamelistptr);

        // set the name to xxx.yyy.zzz = name in preprocess_trunk_new_stmt_
        const auto assign_stmt = std::make_shared<SyntaxTreeAssign>(funcname->Loc());
        const auto varlist = std::make_shared<SyntaxTreeVarlist>(funcname->Loc());

        const auto prefixexp = std::make_shared<SyntaxTreePrefixexp>(funcname->Loc());
        const auto first_var = std::make_shared<SyntaxTreeVar>(funcname->Loc());
        first_var->SetType("simple");
        first_var->SetName(newnamelist[0]);
        prefixexp->SetType("var");
        prefixexp->SetValue(first_var);

        SyntaxTreeInterfacePtr last_prefixexp = prefixexp;
        SyntaxTreeInterfacePtr last_var = first_var;
        for (size_t i = 1; i < newnamelist.size(); ++i) {
            const auto var = std::make_shared<SyntaxTreeVar>(funcname->Loc());
            var->SetType("dot");
            var->SetName(newnamelist[i]);
            var->SetPrefixexp(last_prefixexp);

            const auto cur_prefixexp = std::make_shared<SyntaxTreePrefixexp>(funcname->Loc());
            cur_prefixexp->SetType("var");
            cur_prefixexp->SetValue(var);

            last_var = var;
            last_prefixexp = cur_prefixexp;
        }

        varlist->AddVar(last_var);
        assign_stmt->SetVarlist(varlist);

        const auto explist = std::make_shared<SyntaxTreeExplist>(funcname->Loc());
        const auto exp = std::make_shared<SyntaxTreeExp>(funcname->Loc());
        exp->SetType("string");
        exp->SetValue(name);
        explist->AddExp(exp);

        assign_stmt->SetExplist(explist);

        global_init_new_stmt_.push_back(assign_stmt);
    }
}

[[noreturn]] void PreProcessor::ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr) {
    ThrowFakeluaException(std::format("{} at {}", msg, LocationStr(ptr)));
}

std::string PreProcessor::LocationStr(const SyntaxTreeInterfacePtr &ptr) {
    return std::format("{}:{}:{}", file_name_, ptr->Loc().begin.line, ptr->Loc().begin.column);
}

void PreProcessor::PreprocessTableAssigns(const SyntaxTreeInterfacePtr &chunk) {
    // the chunk must be a block
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    // walk through the block
    for (const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk); auto &stmt: block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            PreprocessTableAssign(func->Funcbody());
        } else if (stmt->Type() == SyntaxTreeType::LocalFunction) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt);
            PreprocessTableAssign(func->Funcbody());
        }
    }
}

void PreProcessor::PreprocessTableAssign(const SyntaxTreeInterfacePtr &funcbody) {
    DEBUG_ASSERT(funcbody->Type() == SyntaxTreeType::FuncBody);
    const auto funcbody_ptr = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);

    const auto block = funcbody_ptr->Block();
    DEBUG_ASSERT(block->Type() == SyntaxTreeType::Block);
    auto block_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(block);

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

            std::vector<SyntaxTreeInterfacePtr> pre;
            std::vector<SyntaxTreeInterfacePtr> post;

            for (auto &var: vars) {
                DEBUG_ASSERT(var->Type() == SyntaxTreeType::Var);
                if (const auto var_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(var);
                    var_ptr->GetType() == "square" || var_ptr->GetType() == "dot") {
                    const auto name = std::format("__fakelua_pp_pre_{}__", pre_index_++);

                    // add new stmt 'local _pre;'
                    {
                        const auto local_var = std::make_shared<SyntaxTreeLocalVar>(stmt->Loc());
                        const auto namelist = std::make_shared<SyntaxTreeNamelist>(stmt->Loc());
                        namelist->AddName(name);
                        local_var->SetNamelist(namelist);
                        pre.push_back(local_var);
                        LOG_INFO("PreprocessTableAssigns add new pre stmt {}", local_var->Dump(0));
                    }

                    // set stmt '_pre = some_value;'
                    {
                        const auto new_var = std::make_shared<SyntaxTreeVar>(var_ptr->Loc());
                        new_var->SetType("simple");
                        new_var->SetName(name);
                        var = new_var;
                        LOG_INFO("PreprocessTableAssigns change new var {}", new_var->Dump(0));
                    }

                    // add new stmt '__fakelua_set_table__(a.b, "c", _pre)'
                    {
                        const auto func_call = std::make_shared<SyntaxTreeFunctioncall>(stmt->Loc());
                        const auto prefixexp = std::make_shared<SyntaxTreePrefixexp>(stmt->Loc());
                        const auto CallVar = std::make_shared<SyntaxTreeVar>(stmt->Loc());
                        CallVar->SetType("simple");
                        CallVar->SetName("__fakelua_set_table__");
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

                        // _pre
                        {
                            const auto args_exp = std::make_shared<SyntaxTreeExp>(stmt->Loc());
                            const auto args_exp_prefixexp = std::make_shared<SyntaxTreePrefixexp>(stmt->Loc());
                            const auto args_exp_var = std::make_shared<SyntaxTreeVar>(stmt->Loc());
                            args_exp_var->SetType("simple");
                            args_exp_var->SetName(name);
                            args_exp_prefixexp->SetType("var");
                            args_exp_prefixexp->SetValue(args_exp_var);
                            args_exp->SetType("prefixexp");
                            args_exp->SetRight(args_exp_prefixexp);
                            args_explist->AddExp(args_exp);
                        }

                        args->SetExplist(args_explist);
                        args->SetType("explist");
                        func_call->SetArgs(args);

                        post.push_back(func_call);
                        LOG_INFO("PreprocessTableAssigns add new post stmt {}", func_call->Dump(0));
                    }
                }
            }

            if (!pre.empty()) {
                new_stmts.insert(new_stmts.end(), pre.begin(), pre.end());
            }
            new_stmts.push_back(stmt);
            if (!post.empty()) {
                new_stmts.insert(new_stmts.end(), post.begin(), post.end());
            }
        } else {
            new_stmts.push_back(stmt);
        }
    }
    block_ptr->SetStmts(new_stmts);
}

void PreProcessor::PreprocessExtractsLiteralConstants(const SyntaxTreeInterfacePtr &chunk) {
    // the chunk must be a block
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);

    // walk tree and find all literal
    std::map<std::string, std::string> integer_map;
    std::map<std::string, std::string> float_map;
    std::map<std::string, std::string> string_map;

    auto walk_func = [&](const SyntaxTreeInterfacePtr &ptr) {
        if (ptr->Type() == SyntaxTreeType::Exp) {
            const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(ptr);
            const auto ExpType = exp->ExpType();
            const auto value = exp->ExpValue();
            if (ExpType == "number") {
                if (IsInteger(value)) {
                    if (!integer_map.contains(value)) {
                        integer_map[value] = std::format("__fakelua_jit_const_integer_var_{}__", integer_map.size());
                    }
                } else {
                    if (!float_map.contains(value)) {
                        float_map[value] = std::format("__fakelua_jit_const_float_var_{}__", float_map.size());
                    }
                }
            } else if (ExpType == "string") {
                if (!string_map.contains(value)) {
                    string_map[value] = std::format("__fakelua_jit_const_string_var_{}__", string_map.size());
                }
            }
        } else if (ptr->Type() == SyntaxTreeType::Var) {
            // a.b.c = 1, the "b" and "c" is common string
            // change to a[__global_string_1__][__global_string_2__] = 1
            if (const auto v_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(ptr); v_ptr->GetType() == "dot") {
                if (const auto name = v_ptr->GetName(); !string_map.contains(name)) {
                    string_map[name] = std::format("__fakelua_jit_const_string_var_{}__", string_map.size());
                }
            }
        } else if (ptr->Type() == SyntaxTreeType::Field) {
            // { a = 1 }, the "a" is a common string
            // change to { [__global_string_1__] = 1 }
            if (const auto field_ptr = std::dynamic_pointer_cast<SyntaxTreeField>(ptr); field_ptr->GetType() == "object") {
                if (const auto name = field_ptr->Name(); !string_map.contains(name)) {
                    string_map[name] = std::format("__fakelua_jit_const_string_var_{}__", string_map.size());
                }
            }
        } else if (ptr->Type() == SyntaxTreeType::FunctionCall) {
            // a:b(), the "b" is common string
            // change to a:__global_string_1__(), and then replace it to var in the Compiler
            if (const auto functioncall_ptr = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(ptr); !functioncall_ptr->Name().empty()) {
                if (const auto name = functioncall_ptr->Name(); !string_map.contains(name)) {
                    string_map[name] = std::format("__fakelua_jit_const_string_var_{}__", string_map.size());
                }
            }
        }
    };

    // walk through the block function
    const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk);
    for (auto &stmt: block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            WalkSyntaxTree(func, walk_func);
        }
    }

    auto cur_stmts = block->Stmts();

    // now we have all literal constants, we need to add them to chunk front
    for (const auto &[value, name]: integer_map) {
        const auto local_var = std::make_shared<SyntaxTreeLocalVar>(chunk->Loc());
        const auto namelist = std::make_shared<SyntaxTreeNamelist>(chunk->Loc());
        namelist->AddName(name);
        local_var->SetNamelist(namelist);
        const auto explist = std::make_shared<SyntaxTreeExplist>(chunk->Loc());
        const auto exp = std::make_shared<SyntaxTreeExp>(chunk->Loc());
        exp->SetType("number");
        exp->SetValue(value);
        explist->AddExp(exp);
        local_var->SetExplist(explist);
        cur_stmts.insert(cur_stmts.begin(), local_var);
    }
    for (const auto &[value, name]: float_map) {
        const auto local_var = std::make_shared<SyntaxTreeLocalVar>(chunk->Loc());
        const auto namelist = std::make_shared<SyntaxTreeNamelist>(chunk->Loc());
        namelist->AddName(name);
        local_var->SetNamelist(namelist);
        const auto explist = std::make_shared<SyntaxTreeExplist>(chunk->Loc());
        const auto exp = std::make_shared<SyntaxTreeExp>(chunk->Loc());
        exp->SetType("number");
        exp->SetValue(value);
        explist->AddExp(exp);
        local_var->SetExplist(explist);
        cur_stmts.insert(cur_stmts.begin(), local_var);
    }
    for (const auto &[value, name]: string_map) {
        const auto local_var = std::make_shared<SyntaxTreeLocalVar>(chunk->Loc());
        const auto namelist = std::make_shared<SyntaxTreeNamelist>(chunk->Loc());
        namelist->AddName(name);
        local_var->SetNamelist(namelist);
        const auto explist = std::make_shared<SyntaxTreeExplist>(chunk->Loc());
        const auto exp = std::make_shared<SyntaxTreeExp>(chunk->Loc());
        exp->SetType("string");
        exp->SetValue(value);
        explist->AddExp(exp);
        local_var->SetExplist(explist);
        cur_stmts.insert(cur_stmts.begin(), local_var);
    }

    block->SetStmts(cur_stmts);

    // now replace all the literal to const name
    const auto replace_func = [&](const SyntaxTreeInterfacePtr &ptr) {
        if (ptr->Type() == SyntaxTreeType::Exp) {
            const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(ptr);
            const auto ExpType = exp->ExpType();
            const auto value = exp->ExpValue();
            if (ExpType == "number") {
                if (IsInteger(value)) {
                    DEBUG_ASSERT(integer_map.contains(value));
                    const auto new_var_name = integer_map[value];
                    const auto prefix = std::make_shared<SyntaxTreePrefixexp>(exp->Loc());
                    const auto var = std::make_shared<SyntaxTreeVar>(exp->Loc());
                    var->SetType("simple");
                    var->SetName(new_var_name);
                    prefix->SetType("var");
                    prefix->SetValue(var);
                    exp->SetType("prefixexp");
                    exp->SetRight(prefix);
                } else {
                    DEBUG_ASSERT(float_map.contains(value));
                    const auto new_var_name = float_map[value];
                    const auto prefix = std::make_shared<SyntaxTreePrefixexp>(exp->Loc());
                    const auto var = std::make_shared<SyntaxTreeVar>(exp->Loc());
                    var->SetType("simple");
                    var->SetName(new_var_name);
                    prefix->SetType("var");
                    prefix->SetValue(var);
                    exp->SetType("prefixexp");
                    exp->SetRight(prefix);
                }
            } else if (ExpType == "string") {
                DEBUG_ASSERT(string_map.contains(value));
                const auto new_var_name = string_map[value];
                const auto prefix = std::make_shared<SyntaxTreePrefixexp>(exp->Loc());
                const auto var = std::make_shared<SyntaxTreeVar>(exp->Loc());
                var->SetType("simple");
                var->SetName(new_var_name);
                prefix->SetType("var");
                prefix->SetValue(var);
                exp->SetType("prefixexp");
                exp->SetRight(prefix);
            }
        } else if (ptr->Type() == SyntaxTreeType::Var) {
            if (const auto v_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(ptr); v_ptr->GetType() == "dot") {
                const auto name = v_ptr->GetName();
                DEBUG_ASSERT(string_map.contains(name));
                const auto new_var_name = string_map[name];
                const auto exp = std::make_shared<SyntaxTreeExp>(ptr->Loc());
                const auto prefix = std::make_shared<SyntaxTreePrefixexp>(ptr->Loc());
                const auto var = std::make_shared<SyntaxTreeVar>(ptr->Loc());
                var->SetType("simple");
                var->SetName(new_var_name);
                prefix->SetType("var");
                prefix->SetValue(var);
                exp->SetType("prefixexp");
                exp->SetRight(prefix);
                v_ptr->SetType("square");
                v_ptr->SetExp(exp);
            }
        } else if (ptr->Type() == SyntaxTreeType::Field) {
            if (const auto field_ptr = std::dynamic_pointer_cast<SyntaxTreeField>(ptr); field_ptr->GetType() == "object") {
                const auto name = field_ptr->Name();
                DEBUG_ASSERT(string_map.contains(name));
                const auto new_var_name = string_map[name];
                const auto exp = std::make_shared<SyntaxTreeExp>(ptr->Loc());
                const auto prefix = std::make_shared<SyntaxTreePrefixexp>(ptr->Loc());
                const auto var = std::make_shared<SyntaxTreeVar>(ptr->Loc());
                var->SetType("simple");
                var->SetName(new_var_name);
                prefix->SetType("var");
                prefix->SetValue(var);
                exp->SetType("prefixexp");
                exp->SetRight(prefix);
                field_ptr->SetType("array");
                field_ptr->SetKey(exp);
            }
        } else if (ptr->Type() == SyntaxTreeType::FunctionCall) {
            if (const auto functioncall_ptr = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(ptr); !functioncall_ptr->Name().empty()) {
                const auto name = functioncall_ptr->Name();
                DEBUG_ASSERT(string_map.contains(name));
                const auto new_var_name = string_map[name];
                functioncall_ptr->SetName(new_var_name);
            }
        }
    };

    // walk through the block function
    for (const auto &stmt: block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            WalkSyntaxTree(func, replace_func);
        }
    }
}

}// namespace fakelua
