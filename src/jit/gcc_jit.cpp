#include "gcc_jit.h"
#include "fakelua.h"
#include "state/State.h"
#include "util/common.h"
#include "util/exception.h"
#include "util/file_util.h"
#include "vm.h"

namespace fakelua {

GccJitter::~GccJitter() {
    if (gccjit_context_) {
        gccjit_context_->release();
        gccjit_context_ = nullptr;
    }
}

void GccJitter::PrepareCompile(const FakeluaStatePtr &sp, const CompileConfig &cfg, const std::string &file_name) {
    gcc_jit_handle_ = std::make_shared<GccJitHandle>();
    sp_ = sp;
    file_name_ = file_name;
    // init gccjit
    gccjit_context_ = std::make_shared<gccjit::context>(gccjit::context::acquire());
    // Set some options on the context.
    if (cfg.debug_mode) {
        gccjit_context_->set_bool_option(GCC_JIT_BOOL_OPTION_DUMP_EVERYTHING, 1);
        gccjit_context_->set_bool_option(GCC_JIT_BOOL_OPTION_KEEP_INTERMEDIATES, 1);
        gccjit_context_->set_bool_option(GCC_JIT_BOOL_OPTION_DEBUGINFO, 1);
        gccjit_context_->set_int_option(GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL, 0);
        auto logfilename = GenerateTmpFilename("fakelua_gccjit_", ".log");
        if (FILE *fp = fopen(logfilename.c_str(), "wb")) {
            gccjit_context_->set_logfile(fp, 0, 0);
            gcc_jit_handle_->SetLogFp(fp);
            LOG_INFO("{} gccjit log file: {}", file_name, logfilename);
        }
    } else {
        gccjit_context_->set_int_option(GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL, 3);
    }

#ifdef WIN32
    // for mingw, need add libfakelua.so to find the symbol
    gccjit_context_->add_driver_option("-L.");
    gccjit_context_->add_driver_option("-lfakelua");
#endif

    // get all the type we need
    void_ptr_type_ = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    bool_type_ = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);
    int64_type_ = gccjit_context_->get_type(GCC_JIT_TYPE_LONG);
    double_type_ = gccjit_context_->get_type(GCC_JIT_TYPE_DOUBLE);
    const_char_ptr_type_ = gccjit_context_->get_type(GCC_JIT_TYPE_CONST_CHAR_PTR);
    int_type_ = gccjit_context_->get_type(GCC_JIT_TYPE_INT);
    size_t_type_ = gccjit_context_->get_type(GCC_JIT_TYPE_SIZE_T);

    /* define var in gccjit
    struct var {
        int type_; // the type of the var, 0: nil, 1: bool, 2: int, 3: double, 4: string, 5: table
        int flag_; // the flag of the var, bit index 0: const, bit index 1: variadic
        union var_data {
            bool b;
            int64_t i;
            double f;
            VarString *s;
            VarTable *t;
        };
        var_data data_;
    }
    */
    var_type_field_ = gccjit_context_->new_field(int_type_, "type_");
    var_flag_field_ = gccjit_context_->new_field(int_type_, "flag_");

    var_data_b_field_ = gccjit_context_->new_field(bool_type_, "b");
    var_data_i_field_ = gccjit_context_->new_field(int64_type_, "i");
    var_data_f_field_ = gccjit_context_->new_field(double_type_, "f");
    var_data_s_field_ = gccjit_context_->new_field(void_ptr_type_, "s");
    var_data_t_field_ = gccjit_context_->new_field(void_ptr_type_, "t");
    gcc_jit_field *data_fields[] = {var_data_b_field_.get_inner_field(), var_data_i_field_.get_inner_field(),
                                    var_data_f_field_.get_inner_field(), var_data_s_field_.get_inner_field(),
                                    var_data_t_field_.get_inner_field()};
    var_data_type_ =
            gccjit::type(gcc_jit_context_new_union_type(gccjit_context_->get_inner_context(), nullptr, "var_data", 5, data_fields));

    var_data_field_ = gccjit_context_->new_field(var_data_type_, "data_");
    std::vector<gccjit::field> var_fields = {var_type_field_, var_flag_field_, var_data_field_};
    var_struct_ = gccjit_context_->new_struct_type("var", var_fields);

    // define global nil var
    global_const_null_var_ = gccjit_context_->new_global(GCC_JIT_GLOBAL_INTERNAL, var_struct_, "__fakelua_jit_const_null_var__");
    global_const_false_var_ = gccjit_context_->new_global(GCC_JIT_GLOBAL_INTERNAL, var_struct_, "__fakelua_jit_const_false_var__");
    global_const_true_var_ = gccjit_context_->new_global(GCC_JIT_GLOBAL_INTERNAL, var_struct_, "__fakelua_jit_const_true_var__");
}

void GccJitter::Compile(const FakeluaStatePtr &sp, const CompileConfig &cfg, const std::string &file_name,
                        const SyntaxTreeInterfacePtr &chunk) {
    LOG_INFO("start GccJitter::Compile {}", file_name);

    PrepareCompile(sp, cfg, file_name);

    // just walk through the chunk, and save the function declaration, and then we can call the function by name
    // first, check the global const definition; the const definition must be an assignment expression at the top level
    CompileConstDefines(chunk);

    // second, walk through the chunk, and save the function declaration
    CompileFunctions(chunk);

    // at last, compile the chunk
    const auto result = gccjit_context_->compile();
    DEBUG_ASSERT(result);
    gcc_jit_handle_->SetResult(result);

    // dump to file
    if (cfg.debug_mode) {
        auto dumpfile = GenerateTmpFilename("fakelua_gccjit_", ".c");
        gccjit_context_->dump_to_file(dumpfile, true);
        LOG_INFO("{} gccjit dump file: {}", file_name, dumpfile);
    }

    // register the function
    for (auto &[name, info]: function_infos_) {
        auto func = gcc_jit_result_get_code(result, name.c_str());
        DEBUG_ASSERT(func);
        std::dynamic_pointer_cast<State>(sp_)->get_vm().RegisterFunction(
                name, std::make_shared<VmFunction>(gcc_jit_handle_, func, info.params_count, info.IsVariadic));
        LOG_INFO("register function: {}", name);
    }

    // call the global const define init func
    CallGlobalInitFunc();

    gccjit_context_->release();
    gccjit_context_ = nullptr;

    LOG_INFO("end GccJitter::compile {}", file_name);
}

void GccJitter::CompileFunctions(const SyntaxTreeInterfacePtr &chunk) {
    // the chunk must be a block
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    // walk through the block
    for (const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk); auto &stmt: block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::Function) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
            auto name = CompileFuncname(func->Funcname());
            CompileFunction(name, func->Funcbody());
        } else if (stmt->Type() == SyntaxTreeType::LocalFunction) {
            const auto func = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt);
            CompileFunction(func->Name(), func->Funcbody());
        }
    }
}

std::string GccJitter::CompileFuncname(const SyntaxTreeInterfacePtr &ptr) {
    DEBUG_ASSERT(ptr->Type() == SyntaxTreeType::FuncName);

    std::string ret;

    const auto name = std::dynamic_pointer_cast<SyntaxTreeFuncname>(ptr);
    const auto funcnamelistptr = name->FuncNameList();

    std::vector<std::string> namelist;

    DEBUG_ASSERT(funcnamelistptr->Type() == SyntaxTreeType::FuncNameList);
    const auto funcnamelist = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(funcnamelistptr);
    const auto &names = funcnamelist->Funcnames();
    namelist.insert(namelist.end(), names.begin(), names.end());

    DEBUG_ASSERT(namelist.size() == 1);
    DEBUG_ASSERT(name->ColonName().empty());

    return namelist[0];
}

void GccJitter::CompileFunction(const std::string &name, const SyntaxTreeInterfacePtr &funcbody) {
    LOG_INFO("start compile function: {}", name);

    DEBUG_ASSERT(funcbody->Type() == SyntaxTreeType::FuncBody);
    const auto funcbody_ptr = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);

    // reset data
    cur_function_data_ = FunctionData();
    cur_function_data_.is_const = name == "__fakelua_global_init__";
    cur_function_data_.cur_function_name = name;

    // compile the func params, all func params is (var *args, size_t arg_size, var *rets, size_t ret_size)
    std::vector<gccjit::param> call_func_params;
    call_func_params.push_back(gccjit_context_->new_param(var_struct_.get_pointer(), "__fakelua_args__", NewLocation(funcbody_ptr)));
    call_func_params.push_back(gccjit_context_->new_param(size_t_type_, "__fakelua_args_size__", NewLocation(funcbody_ptr)));
    call_func_params.push_back(gccjit_context_->new_param(var_struct_.get_pointer(), "__fakelua_rets__", NewLocation(funcbody_ptr)));
    call_func_params.push_back(gccjit_context_->new_param(var_struct_.get_pointer(), "__fakelua_cur__", NewLocation(funcbody_ptr)));
    call_func_params.push_back(gccjit_context_->new_param(var_struct_.get_pointer(), "__fakelua_max__", NewLocation(funcbody_ptr)));
    auto func = gccjit_context_->new_function(GCC_JIT_FUNCTION_EXPORTED, var_struct_, name, call_func_params, 0, NewLocation(funcbody_ptr));
    cur_function_data_.cur_gccjit_func = func;

    // define the function body
    const auto block = funcbody_ptr->Block();
    const auto the_block = func.new_block(NewBlockName("block", block));
    cur_function_data_.cur_block = the_block;

    // compile the input params
    const auto parlist = funcbody_ptr->Parlist();
    int IsVariadic = 0;
    std::vector<std::pair<std::string, gccjit::lvalue>> func_params;
    if (parlist) {
        func_params = CompileParlist(func, parlist, IsVariadic);
    }
    const auto actual_params_count = func_params.size();
    if (IsVariadic) {
        cur_function_data_.cur_variadic_param_start_index = actual_params_count;
    }

    // add params to the new stack frame
    FunctionData::StackFrame sf;
    for (auto &[fst, snd]: func_params) {
        sf.local_vars[fst] = snd;
    }
    cur_function_data_.stack_frames.clear();
    cur_function_data_.stack_frames.push_back(sf);

    // init some const var if is const func
    if (cur_function_data_.is_const) {
        InitGlobalConstVar(func);
    }

    // compile the function body
    CompileStmtBlock(func, block);

    // check the return block, if the return block is not ended with return, we add a return nil
    CheckReturnBlock(func, funcbody_ptr);

    // save the function info
    function_infos_[name] = {static_cast<int>(actual_params_count), IsVariadic > 0, func};
}

void GccJitter::CheckReturnBlock(gccjit::function &func, const SyntaxTreeInterfacePtr &ptr) {
    if (cur_function_data_.ended_blocks.find(cur_function_data_.cur_block.get_inner_block()) == cur_function_data_.ended_blocks.end()) {
        // return nil
        auto stmt = std::make_shared<SyntaxTreeReturn>(ptr->Loc());
        CompileStmtReturn(func, stmt);
    }
}

bool GccJitter::IsBlockEnded() {
    if (cur_function_data_.ended_blocks.find(cur_function_data_.cur_block.get_inner_block()) != cur_function_data_.ended_blocks.end()) {
        return true;
    }
    return false;
}

void GccJitter::InitGlobalConstVar(gccjit::function &func) {
    // init global_const_null_var_
    cur_function_data_.cur_block.add_comment("init const_null_var");
    // set type
    cur_function_data_.cur_block.add_assignment(global_const_null_var_.access_field(var_type_field_),
                                                gccjit_context_->new_rvalue(int_type_, static_cast<int>(VarType::Nil)));

    // init global_const_false_var_
    cur_function_data_.cur_block.add_comment("init const_false_var");
    // set type
    cur_function_data_.cur_block.add_assignment(global_const_false_var_.access_field(var_type_field_),
                                                gccjit_context_->new_rvalue(int_type_, (int) static_cast<int>(VarType::Bool)));

    // set data.b = false
    cur_function_data_.cur_block.add_assignment(global_const_false_var_.access_field(var_data_field_).access_field(var_data_b_field_),
                                                gccjit_context_->new_rvalue(bool_type_, false));

    // init global_const_true_var_
    cur_function_data_.cur_block.add_comment("init const_true_var");
    // set type
    cur_function_data_.cur_block.add_assignment(global_const_true_var_.access_field(var_type_field_),
                                                gccjit_context_->new_rvalue(int_type_, (int) static_cast<int>(VarType::Bool)));

    // set data.b = true
    cur_function_data_.cur_block.add_assignment(global_const_true_var_.access_field(var_data_field_).access_field(var_data_b_field_),
                                                gccjit_context_->new_rvalue(bool_type_, true));
}

void GccJitter::CompileConstDefines(const SyntaxTreeInterfacePtr &chunk) {
    // the chunk must be a block
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);
    // walk through the block
    for (const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk); auto &stmt: block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::LocalVar) {
            CompileConstDefine(stmt);
        } else if (stmt->Type() == SyntaxTreeType::Function || stmt->Type() == SyntaxTreeType::LocalFunction) {
            // skip
        } else {
            ThrowError("the chunk top level only support const define and function define", stmt);
        }
    }
}

void GccJitter::CompileConstDefine(const SyntaxTreeInterfacePtr &stmt) {
    const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
    DEBUG_ASSERT(local_var->Namelist()->Type() == SyntaxTreeType::NameList);
    const auto keys = std::dynamic_pointer_cast<SyntaxTreeNamelist>(local_var->Namelist());
    const auto &names = keys->Names();
    DEBUG_ASSERT(local_var->Explist());
    DEBUG_ASSERT(local_var->Explist()->Type() == SyntaxTreeType::ExpList);
    const auto values = std::dynamic_pointer_cast<SyntaxTreeExplist>(local_var->Explist());
    const auto &values_exps = values->Exps();

    for (size_t i = 0; i < names.size(); ++i) {
        auto name = names[i];
        DEBUG_ASSERT(i < values_exps.size());
        LOG_INFO("compile const define: {}", name);
        auto dst = gccjit_context_->new_global(GCC_JIT_GLOBAL_INTERNAL, var_struct_, name.c_str(), NewLocation(keys));
        if (global_const_vars_.contains(name)) {
            ThrowError("the const define name is duplicated: " + name, values);
        }
        global_const_vars_[name] = std::make_pair(dst, values_exps[i]);
    }
}

gccjit::rvalue GccJitter::CompileExp(gccjit::function &func, const SyntaxTreeInterfacePtr &exp) {
    // the chunk must be an exp
    DEBUG_ASSERT(exp->Type() == SyntaxTreeType::Exp);
    // start to compile the expression
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto &ExpType = e->ExpType();
    const auto &value = e->ExpValue();

    const auto is_const = cur_function_data_.is_const;

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(void_ptr_type_, "s"));
    params.push_back(gccjit_context_->new_param(void_ptr_type_, "h"));
    params.push_back(gccjit_context_->new_param(bool_type_, "is_const"));

    const std::string func_name;

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(void_ptr_type_, (void *) sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(void_ptr_type_, (void *) gcc_jit_handle_.get()));
    args.push_back(gccjit_context_->new_rvalue(bool_type_, is_const));

    DEBUG_ASSERT(ExpType == "nil" || ExpType == "false" || ExpType == "true" || ExpType == "number" || ExpType == "string" ||
                 ExpType == "prefixexp" || ExpType == "VarParams" || ExpType == "tableconstructor" || ExpType == "binop" ||
                 ExpType == "unop")

    if (ExpType == "nil") {
        return global_const_null_var_;
    } else if (ExpType == "false") {
        return global_const_false_var_;
    } else if (ExpType == "true") {
        return global_const_true_var_;
    } else if (ExpType == "number") {
        if (IsInteger(value)) {
            cur_function_data_.cur_block.add_comment(std::format("new int var {}", value), NewLocation(e));
            auto tmp_int =
                    func.new_local(var_struct_, std::format("__fakelua_jit_tmp_int_{}__", cur_function_data_.tmp_index++), NewLocation(e));
            SetVarInt(tmp_int, ToInteger(value), is_const, e);
            return tmp_int;
        } else {
            cur_function_data_.cur_block.add_comment(std::format("new float var {}", value), NewLocation(e));
            auto tmp_float = func.new_local(var_struct_, std::format("__fakelua_jit_tmp_float_{}__", cur_function_data_.tmp_index++),
                                            NewLocation(e));
            SetVarFloat(tmp_float, ToFloat(value), is_const, e);
            return tmp_float;
        }
    } else if (ExpType == "string") {
        cur_function_data_.cur_block.add_comment(std::format("new string var {}", value), NewLocation(e));
        auto tmp_string =
                func.new_local(var_struct_, std::format("__fakelua_jit_tmp_string_{}__", cur_function_data_.tmp_index++), NewLocation(e));
        SetVarString(tmp_string, value, is_const, e);
        return tmp_string;
    } else if (ExpType == "prefixexp") {
        const auto pe = e->Right();
        return CompilePrefixexp(func, pe);
    } else if (ExpType == "VarParams") {
        if (is_const) {
            ThrowError("... can not be const", exp);
        }
        return FindLvalueByName("__fakelua_variadic__", e);
    } else if (ExpType == "tableconstructor") {
        const auto tc = e->Right();
        return CompileTableconstructor(func, tc);
    } else if (ExpType == "binop") {
        const auto left = e->Left();
        const auto right = e->Right();
        const auto op = e->Op();
        return CompileBinop(func, left, right, op);
    } else if (ExpType == "unop") {
        const auto right = e->Right();
        const auto op = e->Op();
        return CompileUnop(func, right, op);
    }

    DEBUG_ASSERT(!func_name.empty());

    gccjit::function new_var_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, var_struct_.get_pointer(), func_name, params, 0, NewLocation(e));
    auto ret = gccjit_context_->new_call(new_var_func, args, NewLocation(e));
    return ret;
}

std::vector<std::pair<std::string, gccjit::lvalue>> GccJitter::CompileParlist(gccjit::function &func, SyntaxTreeInterfacePtr parlist,
                                                                              int &IsVariadic) {
    DEBUG_ASSERT(parlist->Type() == SyntaxTreeType::ParList);
    const auto parlist_ptr = std::dynamic_pointer_cast<SyntaxTreeParlist>(parlist);

    std::vector<std::pair<std::string, gccjit::lvalue>> ret;

    if (const auto namelist = parlist_ptr->Namelist()) {
        DEBUG_ASSERT(namelist->Type() == SyntaxTreeType::NameList);
        const auto namelist_ptr = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist);
        auto &param_names = namelist_ptr->Names();

        // check duplicated
        std::set<std::string> param_names_set;
        // insert global names
        for (const auto &key: global_const_vars_ | std::views::keys) {
            param_names_set.insert(key);
        }
        for (auto &name: param_names) {
            if (param_names_set.contains(name)) {
                ThrowError("the param name is duplicated: " + name, namelist_ptr);
            }
            param_names_set.insert(name);
        }

        const auto args = func.get_param(0);
        const auto args_size = func.get_param(1);

        for (size_t i = 0; i < param_names.size(); ++i) {
            auto name = param_names[i];
            // just copy from args: var name; if (i < __fakelua_args_size__) { name = __fakelua_args__[i]; }
            DEBUG_ASSERT(!name.empty());

            // compile: var name
            cur_function_data_.cur_block.add_comment(std::format("new param var {}", name), NewLocation(namelist_ptr));
            auto param = func.new_local(var_struct_, name, NewLocation(namelist_ptr));

            // compile: if (i < __fakelua_args_size__)
            const gccjit::block cond_block = func.new_block(NewBlockName("if cond", namelist_ptr));
            const gccjit::block body_block = func.new_block(NewBlockName("if body", namelist_ptr));
            const gccjit::block else_block = func.new_block(NewBlockName("if else", namelist_ptr));
            const gccjit::block end_block = func.new_block(NewBlockName("if end", namelist_ptr));

            DEBUG_ASSERT(!IsBlockEnded());
            cur_function_data_.cur_block.end_with_jump(cond_block, NewLocation(namelist_ptr));
            cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
            cur_function_data_.cur_block = cond_block;

            const auto i_var = gccjit_context_->new_rvalue(int_type_, static_cast<int>(i));
            const auto test_ret = gccjit_context_->new_comparison(GCC_JIT_COMPARISON_LT, i_var, args_size, NewLocation(namelist_ptr));

            DEBUG_ASSERT(!IsBlockEnded());
            cur_function_data_.cur_block.end_with_conditional(test_ret, body_block, else_block, NewLocation(namelist_ptr));
            cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
            cur_function_data_.cur_block = body_block;

            // compile: name = __fakelua_args__[i];
            const auto args_exp = gccjit_context_->new_array_access(args, i_var, NewLocation(namelist_ptr));
            cur_function_data_.cur_block.add_assignment(param, args_exp, NewLocation(namelist_ptr));

            DEBUG_ASSERT(!IsBlockEnded());
            cur_function_data_.cur_block.end_with_jump(end_block, NewLocation(namelist_ptr));
            cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
            cur_function_data_.cur_block = else_block;

            DEBUG_ASSERT(!IsBlockEnded());
            cur_function_data_.cur_block.end_with_jump(end_block, NewLocation(namelist_ptr));
            cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
            cur_function_data_.cur_block = end_block;

            ret.emplace_back(name, param);
        }
    }

    if (parlist_ptr->VarParams()) {
        IsVariadic = 1;
    }

    return ret;
}

gccjit::location GccJitter::NewLocation(const SyntaxTreeInterfacePtr &ptr) {
    return gccjit_context_->new_location(file_name_, ptr->Loc().begin.line, ptr->Loc().begin.column);
}

std::string GccJitter::NewBlockName(const std::string &name, const SyntaxTreeInterfacePtr &ptr) {
    return std::format("{} {}:{}:{}", name, file_name_, ptr ? ptr->Loc().begin.line : 0, ptr ? ptr->Loc().begin.column : 0);
}

void GccJitter::CompileStmt(gccjit::function &func, const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(!IsBlockEnded());

    cur_function_data_.cur_block.add_comment(stmt->Dump(0), NewLocation(stmt));

    switch (stmt->Type()) {
        case SyntaxTreeType::Return: {
            CompileStmtReturn(func, stmt);
            break;
        }
        case SyntaxTreeType::LocalVar: {
            CompileStmtLocalVar(func, stmt);
            break;
        }
        case SyntaxTreeType::Assign: {
            CompileStmtAssign(func, stmt);
            break;
        }
        case SyntaxTreeType::FunctionCall: {
            CompileStmtFunctioncall(func, stmt);
            break;
        }
        case SyntaxTreeType::Label: {
            CompileStmtLabel(func, stmt);
            break;
        }
        case SyntaxTreeType::Block: {
            CompileStmtBlock(func, stmt);
            break;
        }
        case SyntaxTreeType::While: {
            CompileStmtWhile(func, stmt);
            break;
        }
        case SyntaxTreeType::Repeat: {
            CompileStmtRepeat(func, stmt);
            break;
        }
        case SyntaxTreeType::If: {
            CompileStmtIf(func, stmt);
            break;
        }
        case SyntaxTreeType::Break: {
            CompileStmtBreak(func, stmt);
            break;
        }
        case SyntaxTreeType::ForLoop: {
            CompileStmtForLoop(func, stmt);
            break;
        }
        case SyntaxTreeType::ForIn: {
            CompileStmtForIn(func, stmt);
            break;
        }
        default: {
            ThrowError(std::format("not support stmt type: {}", SyntaxTreeTypeToString(stmt->Type())), stmt);
        }
    }
}

void GccJitter::CompileStmtReturn(gccjit::function &func, const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::Return);
    const auto return_stmt = std::dynamic_pointer_cast<SyntaxTreeReturn>(stmt);

    /*
     * eg: 'return var_a, FuncB(b), FuncC(c)'
     * begin compile return, make tmp var:
     *    size_t tmp_ret_count = 0;
     *    cvar * tmp_ret_start = __fakelua_cur__;
     * first, compile the var_a, it is simple return, just
     *    *__fakelua_cur__ = var_a;
     *    __fakelua_cur__++;
     *    tmp_ret_count++;
     * second, compile the FuncB(), it is function call return, need call function first
     *    call FuncB, get the return var list and return count
     *    *__fakelua_cur__ = ret_var_list[1] from FuncB
     *    __fakelua_cur__++;
     *    tmp_ret_count++;
     * then, compile the FuncC(), same as FuncB(), but because it is the last return, need check the max rets
     *    call FuncC, get the return var list and return count
     *    for i = 1 to ret_count from FuncC
     *        *__fakelua_cur__ = ret_var_list[i] from FuncC
     *        __fakelua_cur__++;
     *        tmp_ret_count++;
     * finally, copy the tmp_ret_count to the __fakelua_rets__ list, the __fakelua_rets__ is the place we should return to the caller
     * it may overlap with __fakelua_cur__, so we need copy one by one
     *    for i = 1 to tmp_ret_count
     *        __fakelua_rets__[i] = tmp_ret_start[i]
     *    return tmp_ret_count;
    */
    auto explist = return_stmt->Explist();
    if (!explist) {
        // return nil
        explist = std::make_shared<SyntaxTreeExplist>(return_stmt->Loc());
        const auto exp = std::make_shared<SyntaxTreeExp>(return_stmt->Loc());
        exp->SetType("nil");
        std::dynamic_pointer_cast<SyntaxTreeExplist>(explist)->AddExp(exp);
    }

    // check if is simple return. e.g.: return 1
    const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
    DEBUG_ASSERT(!explist_ptr->Exps().empty());
    if (explist_ptr->Exps().size() == 1) {
        const auto exp = explist_ptr->Exps()[0];
        const auto ret = CompileExp(func, exp);

        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.end_with_return(ret, NewLocation(return_stmt));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
        return;
    }

    const auto explist_ret = CompileExplist(func, explist);

    const auto is_const = cur_function_data_.is_const;

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(void_ptr_type_, "s"));
    params.push_back(gccjit_context_->new_param(bool_type_, "is_const"));
    params.push_back(gccjit_context_->new_param(int_type_, "n"));

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(void_ptr_type_, (void *) sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(bool_type_, is_const));
    args.push_back(gccjit_context_->new_rvalue(int_type_, static_cast<int>(explist_ret.size())));

    for (auto &exp_ret: explist_ret) {
        args.push_back(exp_ret);
    }

    const gccjit::function wrap_return_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, var_struct_, "WrapReturnVar", params, 1, NewLocation(explist));
    const auto ret = gccjit_context_->new_call(wrap_return_func, args, NewLocation(explist));

    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.end_with_return(ret, NewLocation(return_stmt));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
}

std::vector<gccjit::rvalue> GccJitter::CompileExplist(gccjit::function &func, const SyntaxTreeInterfacePtr &explist) {
    DEBUG_ASSERT(explist->Type() == SyntaxTreeType::ExpList);
    auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);

    std::vector<gccjit::rvalue> ret;
    auto &exps = explist_ptr->Exps();
    for (auto &exp: exps) {
        auto exp_ret = CompileExp(func, exp);
        ret.push_back(exp_ret);
    }

    return ret;
}

void GccJitter::CallGlobalInitFunc() {
    if (global_const_vars_.empty()) {
        return;
    }
    const auto init_func = reinterpret_cast<Var (*)()>(gcc_jit_result_get_code(gcc_jit_handle_->get_result(), "__fakelua_global_init__"));
    DEBUG_ASSERT(init_func);
    init_func();
}

gccjit::rvalue GccJitter::CompilePrefixexp(gccjit::function &func, const SyntaxTreeInterfacePtr &pe) {
    DEBUG_ASSERT(pe->Type() == SyntaxTreeType::PrefixExp);
    auto pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(pe);

    const auto &pe_type = pe_ptr->GetType();
    auto value = pe_ptr->GetValue();

    auto is_const = cur_function_data_.is_const;

    DEBUG_ASSERT(pe_type == "var" || pe_type == "functioncall" || pe_type == "exp");

    if (pe_type == "var") {
        return CompileVar(func, value);
    } else if (pe_type == "functioncall") {
        if (is_const) {
            ThrowError("functioncall can not be const", pe);
        }
        return CompileFunctioncall(func, value);
    } else /*if (pe_type == "exp")*/ {
        return CompileExp(func, value);
    }
}

std::string GccJitter::LocationStr(const SyntaxTreeInterfacePtr &ptr) {
    return std::format("{}:{}:{}", file_name_, ptr->Loc().begin.line, ptr->Loc().begin.column);
}

gccjit::rvalue GccJitter::CompileVar(gccjit::function &func, const SyntaxTreeInterfacePtr &v) {
    DEBUG_ASSERT(v->Type() == SyntaxTreeType::Var);
    auto v_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(v);

    DEBUG_ASSERT(v_ptr->GetType() == "simple" || v_ptr->GetType() == "square" || v_ptr->GetType() == "dot");

    const auto &type = v_ptr->GetType();
    if (type == "simple") {
        const auto &name = v_ptr->GetName();
        return FindLvalueByName(name, v_ptr);
    } else if (type == "square") {
        auto pe = v_ptr->GetPrefixexp();
        auto exp = v_ptr->GetExp();
        auto pe_ret = CompilePrefixexp(func, pe);
        auto exp_ret = CompileExp(func, exp);

        auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
        auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

        auto is_const = cur_function_data_.is_const;

        std::vector<gccjit::param> params;
        params.push_back(gccjit_context_->new_param(the_var_type, "s"));
        params.push_back(gccjit_context_->new_param(the_var_type, "h"));
        params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
        params.push_back(gccjit_context_->new_param(the_var_type, "t"));
        params.push_back(gccjit_context_->new_param(the_var_type, "k"));

        std::vector<gccjit::rvalue> args;
        args.push_back(gccjit_context_->new_rvalue(the_var_type, sp_.get()));
        args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
        args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
        args.push_back(pe_ret);
        args.push_back(exp_ret);

        gccjit::function table_index_func =
                gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, "TableIndexByVar", params, 0, NewLocation(v_ptr));
        auto ret = gccjit_context_->new_call(table_index_func, args, NewLocation(v_ptr));

        return ret;
    } else /*if (type == "dot")*/ {
        auto pe = v_ptr->GetPrefixexp();
        auto name = v_ptr->GetName();
        auto pe_ret = CompilePrefixexp(func, pe);

        auto name_exp = std::make_shared<SyntaxTreeExp>(v_ptr->Loc());
        name_exp->SetType("string");
        name_exp->SetValue(name);
        auto exp_ret = CompileExp(func, name_exp);

        auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
        auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

        auto is_const = cur_function_data_.is_const;

        std::vector<gccjit::param> params;
        params.push_back(gccjit_context_->new_param(the_var_type, "s"));
        params.push_back(gccjit_context_->new_param(the_var_type, "h"));
        params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
        params.push_back(gccjit_context_->new_param(the_var_type, "t"));
        params.push_back(gccjit_context_->new_param(the_var_type, "k"));

        std::vector<gccjit::rvalue> args;
        args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
        args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
        args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
        args.push_back(pe_ret);
        args.push_back(exp_ret);

        gccjit::function table_index_func =
                gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, "TableIndexByVar", params, 0, NewLocation(v_ptr));
        auto ret = gccjit_context_->new_call(table_index_func, args, NewLocation(v_ptr));

        return ret;
    }
}

gccjit::lvalue GccJitter::CompileVarLvalue(gccjit::function &func, const SyntaxTreeInterfacePtr &v) {
    DEBUG_ASSERT(v->Type() == SyntaxTreeType::Var);
    auto v_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(v);

    // we already change the code a.b = 1 to _pre = 1; SetTable(_pre, a, b)
    // so here no "square" "dot", just "simple"
    DEBUG_ASSERT(v_ptr->GetType() == "simple");

    const auto &type = v_ptr->GetType();
    /*if (type == "simple")*/ {
        const auto &name = v_ptr->GetName();
        return FindLvalueByName(name, v_ptr);
    }
}

gccjit::lvalue GccJitter::FindLvalueByName(const std::string &name, const SyntaxTreeInterfacePtr &ptr) {
    auto ret = TryFindLvalueByName(name, ptr);
    if (!ret) {
        ThrowError("can not find var: " + name, ptr);
    }
    return ret.value();
}

std::optional<gccjit::lvalue> GccJitter::TryFindLvalueByName(const std::string &name, const SyntaxTreeInterfacePtr &ptr) {
    // find in local vars in stack_frames_ by reverse
    for (auto iter = cur_function_data_.stack_frames.rbegin(); iter != cur_function_data_.stack_frames.rend(); ++iter) {
        auto &local_vars = iter->local_vars;
        auto iter2 = local_vars.find(name);
        if (iter2 != local_vars.end()) {
            return iter2->second;
        }
    }

    // find in global const vars
    auto iter = global_const_vars_.find(name);
    if (iter != global_const_vars_.end()) {
        return iter->second.first;
    }

    return std::nullopt;
}

void GccJitter::SaveStackLvalueByName(const std::string &name, const gccjit::lvalue &value, const SyntaxTreeInterfacePtr &ptr) {
    // check global dumplicated
    if (global_const_vars_.find(name) != global_const_vars_.end()) {
        ThrowError("the const define name is duplicated: " + name, ptr);
    }
    // check local dumplicated
    for (auto iter = cur_function_data_.stack_frames.rbegin(); iter != cur_function_data_.stack_frames.rend(); ++iter) {
        auto &local_vars = iter->local_vars;
        auto iter2 = local_vars.find(name);
        if (iter2 != local_vars.end()) {
            ThrowError("the local var name is duplicated: " + name, ptr);
        }
    }
    cur_function_data_.stack_frames.back().local_vars[name] = value;
}

[[noreturn]] void GccJitter::ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr) {
    ThrowFakeluaException(std::format("{} at {}", msg, LocationStr(ptr)));
}

void GccJitter::CompileStmtLocalVar(gccjit::function &function, const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::LocalVar);
    auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);

    auto keys = std::dynamic_pointer_cast<SyntaxTreeNamelist>(local_var->Namelist());
    auto &names = keys->Names();

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);

    // first register the local vars
    for (size_t i = 0; i < names.size(); ++i) {
        auto name = names[i];
        LOG_INFO("compile local var: {}", name);

        auto dst = function.new_local(the_var_type, name, NewLocation(keys));

        // make it nil
        auto nil_exp = std::make_shared<SyntaxTreeExp>(keys->Loc());
        nil_exp->SetType("nil");
        auto exp_ret = CompileExp(function, nil_exp);

        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.add_assignment(dst, exp_ret, NewLocation(nil_exp));

        // add to local vars
        SaveStackLvalueByName(name, dst, keys);
    }

    // then make the assign
    if (!local_var->Explist()) {
        // no value, just return
        return;
    }
    auto assign_stmt = std::make_shared<SyntaxTreeAssign>(stmt->Loc());
    auto varlist = std::make_shared<SyntaxTreeVarlist>(stmt->Loc());
    for (auto &name: names) {
        auto var = std::make_shared<SyntaxTreeVar>(stmt->Loc());
        var->SetType("simple");
        var->SetName(name);
        varlist->AddVar(var);
    }
    assign_stmt->SetVarlist(varlist);
    assign_stmt->SetExplist(local_var->Explist());

    CompileStmtAssign(function, assign_stmt);
}

std::vector<gccjit::lvalue> GccJitter::CompileVarlistLvalue(gccjit::function &func, const SyntaxTreeInterfacePtr &varlist) {
    DEBUG_ASSERT(varlist->Type() == SyntaxTreeType::VarList);
    auto varlist_ptr = std::dynamic_pointer_cast<SyntaxTreeVarlist>(varlist);

    std::vector<gccjit::lvalue> ret;
    auto &vars = varlist_ptr->Vars();
    for (auto &var: vars) {
        auto lvalue = CompileVarLvalue(func, var);
        ret.push_back(lvalue);
    }

    return ret;
}

bool GccJitter::IsSimpleAssign(const SyntaxTreeInterfacePtr &vars, const SyntaxTreeInterfacePtr &exps) {
    DEBUG_ASSERT(vars->Type() == SyntaxTreeType::VarList);
    DEBUG_ASSERT(exps->Type() == SyntaxTreeType::ExpList);
    auto vars_ptr = std::dynamic_pointer_cast<SyntaxTreeVarlist>(vars);
    auto exps_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(exps);

    auto &vars_vec = vars_ptr->Vars();
    auto &exps_vec = exps_ptr->Exps();

    for (size_t i = 0; i < vars_vec.size(); ++i) {
        auto &var = vars_vec[i];
        DEBUG_ASSERT(var->Type() == SyntaxTreeType::Var);
        auto var_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(var);
        DEBUG_ASSERT(var_ptr->GetType() == "simple");
    }

    for (size_t i = 0; i < exps_vec.size(); ++i) {
        auto &exp = exps_vec[i];
        DEBUG_ASSERT(exp->Type() == SyntaxTreeType::Exp);
        auto exp_ptr = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
        if (!IsSimpleExp(exp_ptr)) {
            return false;
        }
    }

    return true;
}

bool GccJitter::IsSimpleArgs(const SyntaxTreeInterfacePtr &args) {
    DEBUG_ASSERT(args->Type() == SyntaxTreeType::Args);
    auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(args);

    auto type = args_ptr->GetType();

    DEBUG_ASSERT(type == "explist" || type == "tableconstructor" || type == "string" || type == "empty");

    if (type == "explist") {
        auto explist = args_ptr->Explist();
        return IsSimpleExplist(explist);
    } else if (type == "tableconstructor") {
        auto tc = args_ptr->Tableconstructor();
        return IsSimpleTableconstructor(tc);
    } else /*if (type == "string")*/ {
        return true;
    }
}

bool GccJitter::IsSimpleExplist(const SyntaxTreeInterfacePtr &explist) {
    DEBUG_ASSERT(explist->Type() == SyntaxTreeType::ExpList);
    auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);

    std::vector<gccjit::rvalue> ret;
    auto &exps = explist_ptr->Exps();
    for (auto &exp: exps) {
        if (!IsSimpleExp(exp)) {
            return false;
        }
    }

    return true;
}

bool GccJitter::IsSimpleExp(const SyntaxTreeInterfacePtr &exp) {
    DEBUG_ASSERT(exp->Type() == SyntaxTreeType::Exp);
    auto exp_ptr = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);

    const auto &ExpType = exp_ptr->ExpType();
    const auto &value = exp_ptr->ExpValue();

    if (ExpType == "nil" || ExpType == "false" || ExpType == "true" || ExpType == "number" || ExpType == "string") {
        return true;
    } else if (ExpType == "prefixexp") {
        auto pe = exp_ptr->Right();
        return IsSimplePrefixexp(pe);
    } else if (ExpType == "tableconstructor") {
        auto tc = exp_ptr->Right();
        return IsSimpleTableconstructor(tc);
    } else if (ExpType == "binop") {
        auto left = exp_ptr->Left();
        auto right = exp_ptr->Right();
        auto op = exp_ptr->Op();
        return IsSimpleExp(left) && IsSimpleExp(right);
    } else if (ExpType == "unop") {
        auto right = exp_ptr->Right();
        auto op = exp_ptr->Op();
        return IsSimpleExp(right);
    } else {
        return false;
    }
}

bool GccJitter::IsSimplePrefixexp(const SyntaxTreeInterfacePtr &pe) {
    DEBUG_ASSERT(pe->Type() == SyntaxTreeType::PrefixExp);
    auto pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(pe);

    const auto &pe_type = pe_ptr->GetType();
    auto value = pe_ptr->GetValue();

    if (pe_type == "var") {
        return true;
    } else if (pe_type == "functioncall") {
        return false;
    } else if (pe_type == "exp") {
        return IsSimpleExp(value);
    } else {
        return false;
    }
}

bool GccJitter::IsSimpleTableconstructor(const SyntaxTreeInterfacePtr &tc) {
    DEBUG_ASSERT(tc->Type() == SyntaxTreeType::TableConstructor);
    auto tc_ptr = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(tc);

    auto fieldlist = tc_ptr->Fieldlist();
    if (!fieldlist) {
        return true;
    }
    DEBUG_ASSERT(fieldlist->Type() == SyntaxTreeType::FieldList);
    auto fieldlist_ptr = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(fieldlist);

    auto &fields = fieldlist_ptr->Fields();
    for (auto &field: fields) {
        if (!IsSimpleField(field)) {
            return false;
        }
    }

    return true;
}

bool GccJitter::IsSimpleField(const SyntaxTreeInterfacePtr &field) {
    DEBUG_ASSERT(field->Type() == SyntaxTreeType::Field);
    auto field_ptr = std::dynamic_pointer_cast<SyntaxTreeField>(field);

    auto field_type = field_ptr->GetType();
    if (field_type == "object") {
        return IsSimpleExp(field_ptr->Value());
    } else if (field_type == "array") {
        if (field_ptr->Key()) {
            return IsSimpleExp(field_ptr->Key()) && IsSimpleExp(field_ptr->Value());
        } else {
            return IsSimpleExp(field_ptr->Value());
        }
    } else {
        return false;
    }
}

void GccJitter::CompileStmtAssign(gccjit::function &function, const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::Assign);
    auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);

    auto vars = assign->Varlist();
    auto varlist = CompileVarlistLvalue(function, vars);

    auto exps = assign->Explist();
    auto explist = CompileExplist(function, exps);

    if (IsSimpleAssign(vars, exps)) {
        // eg: a = 1
        for (size_t i = 0; i < varlist.size() && i < explist.size(); ++i) {
            auto &var = varlist[i];
            auto &exp = explist[i];
            DEBUG_ASSERT(!IsBlockEnded());
            cur_function_data_.cur_block.add_assignment(var, exp, NewLocation(stmt));
        }
        return;
    }

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

    auto is_const = cur_function_data_.is_const;

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));
    params.push_back(gccjit_context_->new_param(the_var_type, "h"));
    params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
    params.push_back(gccjit_context_->new_param(gccjit_context_->get_type(GCC_JIT_TYPE_INT), "src_n"));
    params.push_back(gccjit_context_->new_param(gccjit_context_->get_type(GCC_JIT_TYPE_INT), "dst_n"));

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
    args.push_back(gccjit_context_->new_rvalue(gccjit_context_->get_type(GCC_JIT_TYPE_INT), (int) varlist.size()));
    args.push_back(gccjit_context_->new_rvalue(gccjit_context_->get_type(GCC_JIT_TYPE_INT), (int) explist.size()));
    for (auto &var: varlist) {
        auto var_addr = var.get_address(NewLocation(stmt));
        args.push_back(var_addr);
    }
    for (auto &exp_ret: explist) {
        args.push_back(exp_ret);
    }

    gccjit::function assign_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, "AssignVar", params, 1, NewLocation(stmt));
    auto ret = gccjit_context_->new_call(assign_func, args, NewLocation(stmt));
    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.add_eval(ret, NewLocation(stmt));
}

gccjit::rvalue GccJitter::CompileTableconstructor(gccjit::function &func, const SyntaxTreeInterfacePtr &tc) {
    DEBUG_ASSERT(tc->Type() == SyntaxTreeType::TableConstructor);
    auto tc_ptr = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(tc);

    auto is_const = cur_function_data_.is_const;

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

    std::vector<gccjit::rvalue> kvs;
    auto fieldlist = tc_ptr->Fieldlist();
    if (fieldlist) {
        kvs = CompileFieldlist(func, fieldlist);
    }

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));
    params.push_back(gccjit_context_->new_param(the_var_type, "h"));
    params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
    params.push_back(gccjit_context_->new_param(gccjit_context_->get_type(GCC_JIT_TYPE_INT), "n"));

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
    args.push_back(gccjit_context_->new_rvalue(gccjit_context_->get_type(GCC_JIT_TYPE_INT), (int) kvs.size()));
    for (auto &kv: kvs) {
        args.push_back(kv);
    }

    gccjit::function new_table_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, "NewVarTable", params, 1, NewLocation(tc));
    auto ret = gccjit_context_->new_call(new_table_func, args, NewLocation(tc));
    return ret;
}

std::vector<gccjit::rvalue> GccJitter::CompileFieldlist(gccjit::function &func, const SyntaxTreeInterfacePtr &fieldlist) {
    DEBUG_ASSERT(fieldlist->Type() == SyntaxTreeType::FieldList);
    auto fieldlist_ptr = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(fieldlist);

    std::vector<gccjit::rvalue> ret;
    auto &fields = fieldlist_ptr->Fields();
    for (auto &field: fields) {
        auto field_ret = CompileField(func, field);
        ret.push_back(field_ret.first);
        ret.push_back(field_ret.second);
    }

    return ret;
}

std::pair<gccjit::rvalue, gccjit::rvalue> GccJitter::CompileField(gccjit::function &func, const SyntaxTreeInterfacePtr &field) {
    DEBUG_ASSERT(field->Type() == SyntaxTreeType::Field);
    auto field_ptr = std::dynamic_pointer_cast<SyntaxTreeField>(field);

    std::pair<gccjit::rvalue, gccjit::rvalue> ret;

    auto field_type = field_ptr->GetType();
    DEBUG_ASSERT(field_type == "object" || field_type == "array");

    if (field_type == "object") {
        auto name = field_ptr->Name();
        auto name_exp = std::make_shared<SyntaxTreeExp>(field->Loc());
        name_exp->SetType("string");
        name_exp->SetValue(name);
        auto k = CompileExp(func, name_exp);

        auto exp = field_ptr->Value();
        auto exp_ret = CompileExp(func, exp);

        ret.first = k;
        ret.second = exp_ret;
    } else if (field_type == "array") {
        auto key = field_ptr->Key();
        if (key) {
            auto k_ret = CompileExp(func, key);
            ret.first = k_ret;
        } else {
            // use nullptr key, means use the auto increment key
            auto k_ret = gccjit_context_->new_rvalue(gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR), nullptr);
            ret.first = k_ret;
        }

        auto exp = field_ptr->Value();
        auto exp_ret = CompileExp(func, exp);

        ret.second = exp_ret;
    }

    return ret;
}

gccjit::rvalue GccJitter::CompileBinop(gccjit::function &func, const SyntaxTreeInterfacePtr &left, const SyntaxTreeInterfacePtr &right,
                                       const SyntaxTreeInterfacePtr &op) {
    DEBUG_ASSERT(op->Type() == SyntaxTreeType::Binop);
    auto op_ptr = std::dynamic_pointer_cast<SyntaxTreeBinop>(op);
    auto opstr = op_ptr->GetOp();

    auto is_const = cur_function_data_.is_const;

    DEBUG_ASSERT(opstr == "PLUS" || opstr == "MINUS" || opstr == "STAR" || opstr == "SLASH" || opstr == "DOUBLE_SLASH" || opstr == "POW" ||
                 opstr == "XOR" || opstr == "MOD" || opstr == "BITAND" || opstr == "BITOR" || opstr == "RIGHT_SHIFT" ||
                 opstr == "LEFT_SHIFT" || opstr == "CONCAT" || opstr == "LESS" || opstr == "LESS_EQUAL" || opstr == "MORE" ||
                 opstr == "MORE_EQUAL" || opstr == "EQUAL" || opstr == "NOT_EQUAL" || opstr == "AND" || opstr == "OR");

    if (opstr == "AND" || opstr == "OR") {
        // left and right ==> var* pre=left; if (pre) pre=right;
        // left or right ==> var* pre=left; if (!pre) pre=right;
        auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
        auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

        auto pre_name = std::format("__fakelua_jit_pre_{}__", cur_function_data_.tmp_index++);
        auto pre = func.new_local(the_var_type, pre_name, NewLocation(op));

        // var* pre=left
        auto left_ret = CompileExp(func, left);
        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.add_assignment(pre, left_ret, NewLocation(op));

        // if (pre)
        std::vector<gccjit::param> params;
        params.push_back(gccjit_context_->new_param(the_var_type, "s"));
        params.push_back(gccjit_context_->new_param(the_var_type, "h"));
        params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
        params.push_back(gccjit_context_->new_param(the_var_type, "v"));

        std::vector<gccjit::rvalue> args;
        args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
        args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
        args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
        args.push_back(pre);

        std::string func_name;
        if (opstr == "AND") {
            func_name = "TestVar";
        } else if (opstr == "OR") {
            func_name = "TestNotVar";
        }

        gccjit::function test_func = gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, gccjit_context_->get_type(GCC_JIT_TYPE_BOOL),
                                                                   func_name, params, 0, NewLocation(op));
        auto test_ret = gccjit_context_->new_call(test_func, args, NewLocation(op));

        auto then_block = func.new_block(NewBlockName("then", op));
        auto else_block = func.new_block(NewBlockName("else", op));
        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.end_with_conditional(test_ret, then_block, else_block, NewLocation(op));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());

        auto new_block = func.new_block(NewBlockName("and end", op));

        // {pre=right;}
        cur_function_data_.cur_block = then_block;
        auto right_ret = CompileExp(func, right);
        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.add_assignment(pre, right_ret, NewLocation(op));
        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.end_with_jump(new_block, NewLocation(op));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());

        // {}
        cur_function_data_.cur_block = else_block;
        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.end_with_jump(new_block, NewLocation(op));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());

        cur_function_data_.cur_block = new_block;

        return pre;
    }

    auto left_ret = CompileExp(func, left);
    auto right_ret = CompileExp(func, right);

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));
    params.push_back(gccjit_context_->new_param(the_var_type, "h"));
    params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
    params.push_back(gccjit_context_->new_param(the_var_type, "left"));
    params.push_back(gccjit_context_->new_param(the_var_type, "right"));

    std::string func_name;

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
    args.push_back(left_ret);
    args.push_back(right_ret);

    if (opstr == "PLUS") {
        func_name = "BinopPlus";
    } else if (opstr == "MINUS") {
        func_name = "BinopMinus";
    } else if (opstr == "STAR") {
        func_name = "BinopStar";
    } else if (opstr == "SLASH") {
        func_name = "BinopSlash";
    } else if (opstr == "DOUBLE_SLASH") {
        func_name = "BinopDoubleSlash";
    } else if (opstr == "POW") {
        func_name = "BinopPow";
    } else if (opstr == "XOR") {
        func_name = "BinopXor";
    } else if (opstr == "MOD") {
        func_name = "BinopMod";
    } else if (opstr == "BITAND") {
        func_name = "BinopBitand";
    } else if (opstr == "BITOR") {
        func_name = "BinopBitor";
    } else if (opstr == "RIGHT_SHIFT") {
        func_name = "BinopRightShift";
    } else if (opstr == "LEFT_SHIFT") {
        func_name = "BinopLeftShift";
    } else if (opstr == "CONCAT") {
        func_name = "BinopConcat";
    } else if (opstr == "LESS") {
        func_name = "BinopLess";
    } else if (opstr == "LESS_EQUAL") {
        func_name = "BinopLessEqual";
    } else if (opstr == "MORE") {
        func_name = "BinopMore";
    } else if (opstr == "MORE_EQUAL") {
        func_name = "BinopMoreEqual";
    } else if (opstr == "EQUAL") {
        func_name = "BinopEqual";
    } else if (opstr == "NOT_EQUAL") {
        func_name = "BinopNotEqual";
    }

    gccjit::function binop_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, func_name, params, 0, NewLocation(op));
    auto ret = gccjit_context_->new_call(binop_func, args, NewLocation(op));

    return ret;
}

gccjit::rvalue GccJitter::CompileUnop(gccjit::function &func, const SyntaxTreeInterfacePtr &right, const SyntaxTreeInterfacePtr &op) {
    DEBUG_ASSERT(op->Type() == SyntaxTreeType::Unop);
    auto op_ptr = std::dynamic_pointer_cast<SyntaxTreeUnop>(op);
    auto opstr = op_ptr->GetOp();

    DEBUG_ASSERT(opstr == "MINUS" || opstr == "NOT" || opstr == "NUMBER_SIGN" || opstr == "BITNOT");

    auto is_const = cur_function_data_.is_const;

    auto right_ret = CompileExp(func, right);

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));
    params.push_back(gccjit_context_->new_param(the_var_type, "h"));
    params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
    params.push_back(gccjit_context_->new_param(the_var_type, "right"));

    std::string func_name;

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
    args.push_back(right_ret);

    if (opstr == "MINUS") {
        func_name = "UnopMinus";
    } else if (opstr == "NOT") {
        func_name = "UnopNot";
    } else if (opstr == "NUMBER_SIGN") {
        func_name = "UnopNumberSign";
    } else if (opstr == "BITNOT") {
        func_name = "UnopBitnot";
    }

    gccjit::function unop_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, func_name, params, 0, NewLocation(op));
    auto ret = gccjit_context_->new_call(unop_func, args, NewLocation(op));

    return ret;
}

gccjit::rvalue GccJitter::CompileFunctioncall(gccjit::function &func, const SyntaxTreeInterfacePtr &functioncall) {
    DEBUG_ASSERT(functioncall->Type() == SyntaxTreeType::FunctionCall);
    auto functioncall_ptr = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(functioncall);

    auto prefixexp = functioncall_ptr->prefixexp();

    gccjit::rvalue prefixexp_ret;

    // simple way, just call the function directly
    auto simple_name = GetSimplePrefixexpName(prefixexp);
    if (!simple_name.empty()) {
        // check if is var call
        auto find_ret = TryFindLvalueByName(simple_name, prefixexp);
        if (find_ret) {
            prefixexp_ret = find_ret.value();
        } else {
            // check if is jit builtin function
            if (IsJitBuiltinFunction(simple_name)) {
                // note: here maybe const function call
                auto is_const = cur_function_data_.is_const;

                auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
                auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

                auto args = functioncall_ptr->Args();
                auto args_ret = CompileArgs(func, args);

                std::vector<gccjit::param> params;
                params.push_back(gccjit_context_->new_param(the_var_type, "s"));
                params.push_back(gccjit_context_->new_param(the_var_type, "h"));
                params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
                for (size_t i = 0; i < args_ret.size(); ++i) {
                    params.push_back(gccjit_context_->new_param(the_var_type, std::format("arg{}", i)));
                }

                std::vector<gccjit::rvalue> args2;
                args2.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
                args2.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
                args2.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
                for (auto &arg_ret: args_ret) {
                    args2.push_back(arg_ret);
                }

                std::string func_name = GetJitBuiltinFunctionVmName(simple_name);
                gccjit::function call_func = gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, func_name, params, 0,
                                                                           NewLocation(functioncall));
                auto ret = gccjit_context_->new_call(call_func, args2, NewLocation(functioncall));
                return ret;
            }

            // check if is local function call, special case: cur function call itself
            gccjit::function local_call_func;
            auto it = function_infos_.find(simple_name);
            if (it != function_infos_.end()) {
                local_call_func = it->second.func;
            } else if (cur_function_data_.cur_function_name == simple_name) {
                local_call_func = cur_function_data_.cur_gccjit_func;
            }
            if (local_call_func.get_inner_function()) {
                auto args = functioncall_ptr->Args();
                auto args_ret = CompileArgs(func, args);

                // check all args is simple assign, just call it directly
                // if args not match, it will compile failed
                if (IsSimpleArgs(args)) {
                    std::vector<gccjit::rvalue> args2;
                    for (auto &arg_ret: args_ret) {
                        args2.push_back(arg_ret);
                    }

                    gccjit::function call_func = local_call_func;
                    auto ret = gccjit_context_->new_call(call_func, args2, NewLocation(functioncall));
                    return ret;
                } else {
                    // same as global function call
                }
            }

            // is global function call, make it as a var
            auto name_exp = std::make_shared<SyntaxTreeExp>(functioncall->Loc());
            name_exp->SetType("string");
            name_exp->SetValue(simple_name);
            prefixexp_ret = CompileExp(func, name_exp);
        }
    } else {
        // is var call, eg: a="test"; a();
        prefixexp_ret = CompilePrefixexp(func, prefixexp);
    }

    // call with col, eg: a:b()
    gccjit::rvalue col_key;
    if (!functioncall_ptr->Name().empty()) {
        DEBUG_ASSERT(functioncall_ptr->Name().starts_with("__fakelua_pp_pre_"));
        col_key = FindLvalueByName(functioncall_ptr->Name(), functioncall_ptr);
    } else {
        col_key = gccjit_context_->new_rvalue(gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR), nullptr);
    }

    // complex way, call the function by CallVar
    auto args = functioncall_ptr->Args();
    auto args_ret = CompileArgs(func, args);

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

    auto is_const = cur_function_data_.is_const;

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));
    params.push_back(gccjit_context_->new_param(the_var_type, "h"));
    params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
    params.push_back(gccjit_context_->new_param(the_var_type, "func"));
    params.push_back(gccjit_context_->new_param(the_var_type, "col_key"));
    params.push_back(gccjit_context_->new_param(gccjit_context_->get_type(GCC_JIT_TYPE_INT), "n"));

    std::vector<gccjit::rvalue> args2;
    args2.push_back(gccjit_context_->new_rvalue(the_var_type, sp_.get()));
    args2.push_back(gccjit_context_->new_rvalue(the_var_type, gcc_jit_handle_.get()));
    args2.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
    args2.push_back(prefixexp_ret);
    args2.push_back(col_key);
    args2.push_back(gccjit_context_->new_rvalue(gccjit_context_->get_type(GCC_JIT_TYPE_INT), (int) args_ret.size()));
    for (auto &arg_ret: args_ret) {
        args2.push_back(arg_ret);
    }

    gccjit::function call_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, "CallVar", params, 1, NewLocation(functioncall));
    auto ret = gccjit_context_->new_call(call_func, args2, NewLocation(functioncall));
    return ret;
}

std::vector<gccjit::rvalue> GccJitter::CompileArgs(gccjit::function &func, const SyntaxTreeInterfacePtr &args) {
    DEBUG_ASSERT(args->Type() == SyntaxTreeType::Args);
    auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(args);

    auto type = args_ptr->GetType();

    DEBUG_ASSERT(type == "explist" || type == "tableconstructor" || type == "string" || type == "empty");

    if (type == "explist") {
        auto explist = args_ptr->Explist();
        return CompileExplist(func, explist);
    } else if (type == "tableconstructor") {
        auto tc = args_ptr->Tableconstructor();
        auto tc_ret = CompileTableconstructor(func, tc);
        return {tc_ret};
    } else if (type == "string") {
        auto str = args_ptr->String();
        auto str_ret = CompileExp(func, str);
        return {str_ret};
    }

    return {};
}

void GccJitter::CompileStmtFunctioncall(gccjit::function &function, const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::FunctionCall);
    auto functioncall = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(stmt);
    if (cur_function_data_.is_const) {
        // only can call __fakelua_set_table__ in const init function
        DEBUG_ASSERT(GetSimplePrefixexpName(functioncall->prefixexp()) == "__fakelua_set_table__");
    }
    auto ret = CompileFunctioncall(function, functioncall);
    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.add_eval(ret, NewLocation(stmt));
}

std::string GccJitter::GetSimplePrefixexpName(const SyntaxTreeInterfacePtr &pe) {
    DEBUG_ASSERT(pe->Type() == SyntaxTreeType::PrefixExp);
    auto pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(pe);

    auto pe_type = pe_ptr->GetType();
    auto value = pe_ptr->GetValue();

    if (pe_type == "var") {
        return GetSimpleVarName(value);
    } else {
        return "";
    }
}

std::string GccJitter::GetSimpleVarName(const SyntaxTreeInterfacePtr &v) {
    DEBUG_ASSERT(v->Type() == SyntaxTreeType::Var);
    auto v_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(v);

    auto v_type = v_ptr->GetType();
    auto name = v_ptr->GetName();

    if (v_type == "simple") {
        return name;
    } else {
        return "";
    }
}

bool GccJitter::IsJitBuiltinFunction(const std::string &name) {
    return name == "__fakelua_set_table__";
}

std::string GccJitter::GetJitBuiltinFunctionVmName(const std::string &name) {
    DEBUG_ASSERT(name == "__fakelua_set_table__");
    /*if (name == "__fakelua_set_table__")*/ { return "TableSet"; }
}

void GccJitter::CompileStmtLabel(gccjit::function &func, const fakelua::SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::Label);
    ThrowError("not support label", stmt);
}

void GccJitter::CompileStmtBlock(gccjit::function &func, const fakelua::SyntaxTreeInterfacePtr &block) {
    DEBUG_ASSERT(block->Type() == SyntaxTreeType::Block);
    const auto block_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(block);

    // alloc new stack frame
    cur_function_data_.stack_frames.push_back(FunctionData::StackFrame());

    for (const auto &stmts = block_ptr->Stmts(); auto &stmt: stmts) {
        CompileStmt(func, stmt);
    }

    // free stack frame
    cur_function_data_.stack_frames.pop_back();
}

void GccJitter::CompileStmtWhile(gccjit::function &func, const fakelua::SyntaxTreeInterfacePtr &wh) {
    DEBUG_ASSERT(wh->Type() == SyntaxTreeType::While);
    auto while_ptr = std::dynamic_pointer_cast<SyntaxTreeWhile>(wh);

    gccjit::block cond_block = func.new_block(NewBlockName("loop cond", wh));
    gccjit::block body_block = func.new_block(NewBlockName("loop body", wh));
    gccjit::block after_block = func.new_block(NewBlockName("after loop", wh));

    // use to break jump
    cur_function_data_.stack_end_blocks.emplace_back(after_block);

    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.end_with_jump(cond_block, NewLocation(wh));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = cond_block;

    // while exp do block end
    auto exp = while_ptr->Exp();
    auto block = while_ptr->Block();

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

    // make the exp
    auto cond_ret = CompileExp(func, exp);

    auto is_const = cur_function_data_.is_const;

    // check exp
    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));
    params.push_back(gccjit_context_->new_param(the_var_type, "h"));
    params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
    params.push_back(gccjit_context_->new_param(the_var_type, "v"));

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
    args.push_back(cond_ret);
    gccjit::function test_func = gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, gccjit_context_->get_type(GCC_JIT_TYPE_BOOL),
                                                               "TestVar", params, 0, NewLocation(exp));
    auto test_ret = gccjit_context_->new_call(test_func, args, NewLocation(exp));

    DEBUG_ASSERT(!IsBlockEnded());
    cond_block.end_with_conditional(test_ret, body_block, after_block, NewLocation(exp));

    cur_function_data_.cur_block = body_block;
    CompileStmtBlock(func, block);

    cur_function_data_.stack_end_blocks.pop_back();

    if (!IsBlockEnded()) {
        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.end_with_jump(cond_block, NewLocation(wh));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    }
    cur_function_data_.cur_block = after_block;
}

void GccJitter::CompileStmtRepeat(gccjit::function &func, const SyntaxTreeInterfacePtr &re) {
    DEBUG_ASSERT(re->Type() == SyntaxTreeType::Repeat);
    auto repeat_ptr = std::dynamic_pointer_cast<SyntaxTreeRepeat>(re);

    gccjit::block cond_block;// maybe body just return, so cond and after maybe not exist
    gccjit::block body_block = func.new_block(NewBlockName("loop body", re));
    gccjit::block after_block;

    // use to break jump
    cur_function_data_.stack_end_blocks.emplace_back(after_block);

    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.end_with_jump(body_block, NewLocation(re));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = body_block;

    // repeat block until cond
    auto exp = repeat_ptr->Exp();
    auto block = repeat_ptr->Block();

    CompileStmtBlock(func, block);

    after_block = cur_function_data_.stack_end_blocks.back();
    cur_function_data_.stack_end_blocks.pop_back();

    if (!IsBlockEnded()) {
        cond_block = func.new_block(NewBlockName("loop cond", re));
        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.end_with_jump(cond_block, NewLocation(re));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    } else {
        if (!after_block.get_inner_block()) {
            // not init, mean all finish
            return;
        }
        // we need continue to after block
        cur_function_data_.cur_block = after_block;
        return;
    }

    cur_function_data_.cur_block = cond_block;

    // make the exp
    auto cond_ret = CompileExp(func, exp);

    auto is_const = cur_function_data_.is_const;

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

    // check exp
    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));
    params.push_back(gccjit_context_->new_param(the_var_type, "h"));
    params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
    params.push_back(gccjit_context_->new_param(the_var_type, "v"));

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
    args.push_back(cond_ret);
    gccjit::function test_func = gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, gccjit_context_->get_type(GCC_JIT_TYPE_BOOL),
                                                               "TestVar", params, 0, NewLocation(exp));
    auto test_ret = gccjit_context_->new_call(test_func, args, NewLocation(exp));

    if (!after_block.get_inner_block()) {
        after_block = func.new_block(NewBlockName("after loop", re));
    }
    DEBUG_ASSERT(!IsBlockEnded());
    cond_block.end_with_conditional(test_ret, after_block, body_block, NewLocation(exp));

    cur_function_data_.cur_block = after_block;
}

void GccJitter::CompileStmtIf(gccjit::function &func, const SyntaxTreeInterfacePtr &is) {
    DEBUG_ASSERT(is->Type() == SyntaxTreeType::If);
    auto if_ptr = std::dynamic_pointer_cast<SyntaxTreeIf>(is);

    auto exp = if_ptr->Exp();
    auto block = if_ptr->Block();
    auto elseifs = if_ptr->ElseIfs();
    DEBUG_ASSERT(elseifs->Type() == SyntaxTreeType::ElseIfList);
    auto elseifs_ptr = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseifs);
    auto else_ptr = if_ptr->ElseBlock();// maybe nullptr

    gccjit::block cond_block = func.new_block(NewBlockName("if cond", is));
    gccjit::block body_block = func.new_block(NewBlockName("if body", is));
    gccjit::block else_block = func.new_block(NewBlockName("if else", is));
    gccjit::block end_block;// maybe if and else all returned, so no need end_block. when use it, init it.

    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.end_with_jump(cond_block, NewLocation(exp));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = cond_block;

    // make the exp
    auto cond_ret = CompileExp(func, exp);

    auto is_const = cur_function_data_.is_const;

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

    // check exp
    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));
    params.push_back(gccjit_context_->new_param(the_var_type, "h"));
    params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
    params.push_back(gccjit_context_->new_param(the_var_type, "v"));

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
    args.push_back(cond_ret);
    gccjit::function test_func = gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, gccjit_context_->get_type(GCC_JIT_TYPE_BOOL),
                                                               "TestVar", params, 0, NewLocation(exp));
    auto test_ret = gccjit_context_->new_call(test_func, args, NewLocation(exp));

    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.end_with_conditional(test_ret, body_block, else_block, NewLocation(exp));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = body_block;

    CompileStmtBlock(func, block);

    if (!IsBlockEnded()) {
        if (!end_block.get_inner_block()) {
            end_block = func.new_block(NewBlockName("if end", is));
        }
        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.end_with_jump(end_block, NewLocation(is));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    }

    cur_function_data_.cur_block = else_block;
    for (size_t i = 0; i < elseifs_ptr->ElseifSize(); i++) {
        auto elseifs_exp = elseifs_ptr->ElseifExp(i);
        auto elseifs_block = elseifs_ptr->ElseifBlock(i);

        auto elseifs_body_block = func.new_block(NewBlockName("elseif body", is));
        auto next_else_block = func.new_block(NewBlockName("elseif else", is));

        // make the exp
        auto elseifs_cond_ret = CompileExp(func, elseifs_exp);

        // check exp
        std::vector<gccjit::param> elseifs_params;
        elseifs_params.push_back(gccjit_context_->new_param(the_var_type, "s"));
        elseifs_params.push_back(gccjit_context_->new_param(the_var_type, "h"));
        elseifs_params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
        elseifs_params.push_back(gccjit_context_->new_param(the_var_type, "v"));

        std::vector<gccjit::rvalue> elseifs_args;
        elseifs_args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
        elseifs_args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
        elseifs_args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
        elseifs_args.push_back(elseifs_cond_ret);

        gccjit::function elseifs_test_func =
                gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, gccjit_context_->get_type(GCC_JIT_TYPE_BOOL), "TestVar",
                                              elseifs_params, 0, NewLocation(elseifs_exp));
        auto elseifs_test_ret = gccjit_context_->new_call(elseifs_test_func, elseifs_args, NewLocation(elseifs_exp));

        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.end_with_conditional(elseifs_test_ret, elseifs_body_block, next_else_block, NewLocation(elseifs_exp));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());

        cur_function_data_.cur_block = elseifs_body_block;
        CompileStmtBlock(func, elseifs_block);

        if (!IsBlockEnded()) {
            if (!end_block.get_inner_block()) {
                end_block = func.new_block(NewBlockName("if end", is));
            }
            DEBUG_ASSERT(!IsBlockEnded());
            cur_function_data_.cur_block.end_with_jump(end_block, NewLocation(is));
            cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
        }

        cur_function_data_.cur_block = next_else_block;
    }

    if (else_ptr) {
        auto else_body_block = func.new_block(NewBlockName("else body", is));

        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.end_with_jump(else_body_block, NewLocation(is));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
        cur_function_data_.cur_block = else_body_block;

        CompileStmtBlock(func, else_ptr);
    }

    if (!IsBlockEnded()) {
        if (!end_block.get_inner_block()) {
            end_block = func.new_block(NewBlockName("if end", is));
        }
        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.end_with_jump(end_block, NewLocation(is));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    }

    if (end_block.get_inner_block()) {
        cur_function_data_.cur_block = end_block;
    }
}

void GccJitter::CompileStmtBreak(gccjit::function &func, const SyntaxTreeInterfacePtr &bs) {
    DEBUG_ASSERT(bs->Type() == SyntaxTreeType::Break);
    auto break_ptr = std::dynamic_pointer_cast<SyntaxTreeBreak>(bs);

    if (cur_function_data_.stack_end_blocks.empty()) {
        ThrowError("break must in loop", bs);
    }

    auto &block = cur_function_data_.stack_end_blocks.back();
    if (!block.get_inner_block()) {
        block = func.new_block(NewBlockName("break", bs));
    }
    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.end_with_jump(block, NewLocation(bs));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
}

void GccJitter::CompileStmtForLoop(gccjit::function &func, const SyntaxTreeInterfacePtr &fs) {
    DEBUG_ASSERT(fs->Type() == SyntaxTreeType::ForLoop);
    auto for_loop_ptr = std::dynamic_pointer_cast<SyntaxTreeForLoop>(fs);

    // for a = 1, 10, 1 do ... end
    auto name = for_loop_ptr->Name();
    auto for_block_ptr = for_loop_ptr->Block();
    auto ExpBegin = for_loop_ptr->ExpBegin();
    auto ExpEnd = for_loop_ptr->ExpEnd();
    auto ExpStep = for_loop_ptr->ExpStep();// maybe nullptr
    if (!ExpStep) {
        // default is 1
        auto one_exp = std::make_shared<SyntaxTreeExp>(ExpEnd->Loc());
        one_exp->SetType("number");
        one_exp->SetValue("1");
        ExpStep = one_exp;
    }

    gccjit::block init_block = func.new_block(NewBlockName("for loop init", fs));
    gccjit::block cond_block = func.new_block(NewBlockName("for loop cond", fs));
    gccjit::block for_block = func.new_block(NewBlockName("for loop body", fs));
    gccjit::block step_block;// maybe return in body block, so when use it, init it
    gccjit::block after_block = func.new_block(NewBlockName("for loop after", fs));

    // use to break jump
    cur_function_data_.stack_end_blocks.emplace_back(after_block);

    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.end_with_jump(init_block, NewLocation(fs));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = init_block;

    auto init_ret = CompileExp(func, ExpBegin);
    auto end_ret = CompileExp(func, ExpEnd);
    auto step_ret = CompileExp(func, ExpStep);

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);
    auto is_const = cur_function_data_.is_const;

    // init the iterator var, eg: a = 1
    auto iter = func.new_local(the_var_type, name, NewLocation(ExpBegin));
    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.add_assignment(iter, init_ret, NewLocation(ExpBegin));
    // add to local vars
    SaveStackLvalueByName(name, iter, ExpBegin);

    // init the end var, eg: pre = 10
    auto end_pre_name = std::format("__fakelua_jit_pre_{}__", cur_function_data_.tmp_index++);
    auto end_pre = func.new_local(the_var_type, end_pre_name, NewLocation(ExpEnd));
    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.add_assignment(end_pre, end_ret, NewLocation(ExpEnd));
    // add to local vars
    SaveStackLvalueByName(end_pre_name, end_pre, ExpEnd);

    // init the step var, eg: pre = 1
    auto step_pre_name = std::format("__fakelua_jit_pre_{}__", cur_function_data_.tmp_index++);
    auto step_pre = func.new_local(the_var_type, step_pre_name, NewLocation(ExpStep));
    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.add_assignment(step_pre, step_ret, NewLocation(ExpStep));
    // add to local vars
    SaveStackLvalueByName(step_pre_name, step_pre, ExpStep);

    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.end_with_jump(cond_block, NewLocation(fs));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = cond_block;

    // check the iterator var, eg: a <= 10
    // make the cond exp
    auto cond_exp = std::make_shared<SyntaxTreeExp>(ExpEnd->Loc());
    cond_exp->SetType("binop");
    auto cond_op = std::make_shared<SyntaxTreeBinop>(ExpEnd->Loc());
    cond_op->SetOp("LESS_EQUAL");
    cond_exp->SetOp(cond_op);
    auto cond_left_var = std::make_shared<SyntaxTreeVar>(ExpEnd->Loc());
    cond_left_var->SetType("simple");
    cond_left_var->SetName(name);
    auto cond_left_prefix = std::make_shared<SyntaxTreePrefixexp>(ExpEnd->Loc());
    cond_left_prefix->SetType("var");
    cond_left_prefix->SetValue(cond_left_var);
    auto cond_left_exp = std::make_shared<SyntaxTreeExp>(ExpEnd->Loc());
    cond_left_exp->SetType("prefixexp");
    cond_left_exp->SetRight(cond_left_prefix);
    cond_exp->SetLeft(cond_left_exp);
    auto cond_right_var = std::make_shared<SyntaxTreeVar>(ExpEnd->Loc());
    cond_right_var->SetType("simple");
    cond_right_var->SetName(end_pre_name);
    auto cond_right_prefix = std::make_shared<SyntaxTreePrefixexp>(ExpEnd->Loc());
    cond_right_prefix->SetType("var");
    cond_right_prefix->SetValue(cond_right_var);
    auto cond_right_exp = std::make_shared<SyntaxTreeExp>(ExpEnd->Loc());
    cond_right_exp->SetType("prefixexp");
    cond_right_exp->SetRight(cond_right_prefix);
    cond_exp->SetRight(cond_right_exp);

    auto cond_ret = CompileExp(func, cond_exp);

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));
    params.push_back(gccjit_context_->new_param(the_var_type, "h"));
    params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
    params.push_back(gccjit_context_->new_param(the_var_type, "v"));

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
    args.push_back(cond_ret);
    gccjit::function test_func = gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, gccjit_context_->get_type(GCC_JIT_TYPE_BOOL),
                                                               "TestVar", params, 0, NewLocation(cond_exp));
    auto test_ret = gccjit_context_->new_call(test_func, args, NewLocation(cond_exp));

    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.end_with_conditional(test_ret, for_block, after_block, NewLocation(ExpEnd));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = for_block;

    CompileStmtBlock(func, for_block_ptr);

    cur_function_data_.stack_end_blocks.pop_back();

    if (!IsBlockEnded()) {
        step_block = func.new_block(NewBlockName("for loop step", fs));
        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.end_with_jump(step_block, NewLocation(fs));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
        cur_function_data_.cur_block = step_block;

        // step the iterator var, eg: a = a + 1
        // make an assign stmt
        auto assign_stmt = std::make_shared<SyntaxTreeAssign>(ExpStep->Loc());
        auto assign_stmt_varlist = std::make_shared<SyntaxTreeVarlist>(ExpStep->Loc());
        auto assign_stmt_var = std::make_shared<SyntaxTreeVar>(ExpStep->Loc());
        assign_stmt_var->SetType("simple");
        assign_stmt_var->SetName(name);
        assign_stmt_varlist->AddVar(assign_stmt_var);
        assign_stmt->SetVarlist(assign_stmt_varlist);
        auto assign_stmt_explist = std::make_shared<SyntaxTreeExplist>(ExpStep->Loc());
        auto assign_stmt_exp = std::make_shared<SyntaxTreeExp>(ExpStep->Loc());
        assign_stmt_exp->SetType("binop");
        auto assign_stmt_op = std::make_shared<SyntaxTreeBinop>(ExpStep->Loc());
        assign_stmt_op->SetOp("PLUS");
        assign_stmt_exp->SetOp(assign_stmt_op);
        auto assign_stmt_left_var = std::make_shared<SyntaxTreeVar>(ExpStep->Loc());
        assign_stmt_left_var->SetType("simple");
        assign_stmt_left_var->SetName(name);
        auto assign_stmt_left_prefix = std::make_shared<SyntaxTreePrefixexp>(ExpStep->Loc());
        assign_stmt_left_prefix->SetType("var");
        assign_stmt_left_prefix->SetValue(assign_stmt_left_var);
        auto assign_stmt_left_exp = std::make_shared<SyntaxTreeExp>(ExpStep->Loc());
        assign_stmt_left_exp->SetType("prefixexp");
        assign_stmt_left_exp->SetRight(assign_stmt_left_prefix);
        assign_stmt_exp->SetLeft(assign_stmt_left_exp);
        auto assign_stmt_right_var = std::make_shared<SyntaxTreeVar>(ExpStep->Loc());
        assign_stmt_right_var->SetType("simple");
        assign_stmt_right_var->SetName(step_pre_name);
        auto assign_stmt_right_prefix = std::make_shared<SyntaxTreePrefixexp>(ExpStep->Loc());
        assign_stmt_right_prefix->SetType("var");
        assign_stmt_right_prefix->SetValue(assign_stmt_right_var);
        auto assign_stmt_right_exp = std::make_shared<SyntaxTreeExp>(ExpStep->Loc());
        assign_stmt_right_exp->SetType("prefixexp");
        assign_stmt_right_exp->SetRight(assign_stmt_right_prefix);
        assign_stmt_exp->SetRight(assign_stmt_right_exp);
        assign_stmt_explist->AddExp(assign_stmt_exp);
        assign_stmt->SetExplist(assign_stmt_explist);

        CompileStmt(func, assign_stmt);

        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.end_with_jump(cond_block, NewLocation(fs));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    }

    cur_function_data_.cur_block = after_block;
}

void GccJitter::CompileStmtForIn(gccjit::function &func, const SyntaxTreeInterfacePtr &fs) {
    DEBUG_ASSERT(fs->Type() == SyntaxTreeType::ForIn);
    auto for_in_ptr = std::dynamic_pointer_cast<SyntaxTreeForIn>(fs);

    // for k, v in pairs(t) do ... end
    auto namelist = for_in_ptr->Namelist();
    auto explist = for_in_ptr->Explist();
    auto block_ptr = for_in_ptr->Block();

    std::string key_name;
    std::string value_name;
    SyntaxTreeInterfacePtr for_args_ptr;

    // get k, v name from namelist
    auto namelist_ptr = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist);
    auto names = namelist_ptr->Names();
    if (names.size() != 1 && names.size() != 2) {
        ThrowError(std::format("for in namelist size must be 1 or 2, but got {}", names.size()), namelist);
    }
    key_name = names[0];
    if (names.size() == 2) {
        value_name = names[1];
    }

    // check explist only have one exp
    auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
    auto exps = explist_ptr->Exps();
    if (exps.size() != 1) {
        ThrowError(std::format("for in explist size must be 1, but got {}", exps.size()), explist);
    }
    auto exp = exps[0];
    // check exp must be ipairs() or pairs()
    DEBUG_ASSERT(exp->Type() == SyntaxTreeType::Exp);
    auto exp_ptr = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    if (exp_ptr->ExpType() != "prefixexp") {
        ThrowError("for in exp (expect prefixexp) must be ipairs() or pairs()", exp);
    }
    auto prefixexp = exp_ptr->Right();
    DEBUG_ASSERT(prefixexp->Type() == SyntaxTreeType::PrefixExp);
    auto prefixexp_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(prefixexp);
    if (prefixexp_ptr->GetType() != "functioncall") {
        ThrowError("for in exp (expect functioncall) must be ipairs() or pairs()", exp);
    }
    auto functioncall = prefixexp_ptr->GetValue();
    DEBUG_ASSERT(functioncall->Type() == SyntaxTreeType::FunctionCall);
    auto functioncall_ptr = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(functioncall);
    auto call_name = GetSimplePrefixexpName(functioncall_ptr->prefixexp());
    if (call_name != "ipairs" && call_name != "pairs") {
        ThrowError("for in exp (expect ipairs/pairs) must be ipairs() or pairs()", exp);
    }
    auto for_args = functioncall_ptr->Args();
    DEBUG_ASSERT(for_args);
    DEBUG_ASSERT(for_args->Type() == SyntaxTreeType::Args);
    for_args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(for_args);

    // for in block, just like for in
    gccjit::block init_block = func.new_block(NewBlockName("for in init", fs));
    gccjit::block cond_block = func.new_block(NewBlockName("for in cond", fs));
    gccjit::block for_block = func.new_block(NewBlockName("for in body", fs));
    gccjit::block step_block;// maybe return in body block, so when use it, init it
    gccjit::block after_block = func.new_block(NewBlockName("for in after", fs));

    // use to break jump
    cur_function_data_.stack_end_blocks.emplace_back(after_block);

    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.end_with_jump(init_block, NewLocation(fs));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = init_block;

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);
    auto the_size_t_type = gccjit_context_->get_type(GCC_JIT_TYPE_SIZE_T);
    auto is_const = cur_function_data_.is_const;

    // init the iterator var, eg: size_t pre0 = 0
    auto iter_name = std::format("__fakelua_jit_pre_{}__", cur_function_data_.tmp_index++);
    auto iter = func.new_local(the_size_t_type, iter_name, NewLocation(exp));
    auto iter_init_value = gccjit_context_->zero(the_size_t_type);
    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.add_assignment(iter, iter_init_value, NewLocation(exp));

    // make the iterator dst
    auto args_ret = CompileArgs(func, for_args_ptr);
    if (args_ret.size() != 1) {
        ThrowError(std::format("for in ipairs() or pairs() args size must be 1, but got {}", args_ret.size()), for_args);
    }
    // store in local, eg: pre1 = table
    auto iter_dst_name = std::format("__fakelua_jit_pre_{}__", cur_function_data_.tmp_index++);
    auto iter_dst = func.new_local(gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR), iter_dst_name, NewLocation(for_args_ptr));
    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.add_assignment(iter_dst, args_ret[0], NewLocation(for_args_ptr));

    // init the iterator end var, eg: size_t pre2 = size(pre1)
    auto iter_end_name = std::format("__fakelua_jit_pre_{}__", cur_function_data_.tmp_index++);
    auto iter_end = func.new_local(the_size_t_type, iter_end_name, NewLocation(for_args_ptr));

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));
    params.push_back(gccjit_context_->new_param(the_var_type, "h"));
    params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
    params.push_back(gccjit_context_->new_param(the_var_type, "v"));

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
    args.push_back(iter_dst);
    gccjit::function size_func = gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, gccjit_context_->get_type(GCC_JIT_TYPE_SIZE_T),
                                                               "TableSize", params, 0, NewLocation(for_args_ptr));
    auto size_ret = gccjit_context_->new_call(size_func, args, NewLocation(for_args_ptr));

    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.add_assignment(iter_end, size_ret, NewLocation(for_args_ptr));

    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.end_with_jump(cond_block, NewLocation(fs));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = cond_block;

    // check cond
    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.end_with_conditional(iter < iter_end, for_block, after_block, NewLocation(exp));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = for_block;

    // start init cur loop local var, eg: k, v = pre1[pre0]
    auto key = func.new_local(the_var_type, key_name, NewLocation(namelist_ptr));

    params.clear();
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));
    params.push_back(gccjit_context_->new_param(the_var_type, "h"));
    params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
    params.push_back(gccjit_context_->new_param(the_var_type, "v"));
    params.push_back(gccjit_context_->new_param(the_size_t_type, "pos"));

    args.clear();
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
    args.push_back(iter_dst);
    args.push_back(iter);
    gccjit::function key_at_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR), "TableKeyByPos",
                                          params, 0, NewLocation(namelist_ptr));
    auto key_ret = gccjit_context_->new_call(key_at_func, args, NewLocation(namelist_ptr));

    DEBUG_ASSERT(!IsBlockEnded());
    cur_function_data_.cur_block.add_assignment(key, key_ret, NewLocation(namelist_ptr));
    SaveStackLvalueByName(key_name, key, namelist_ptr);

    if (!value_name.empty()) {
        auto value = func.new_local(the_var_type, value_name, NewLocation(namelist_ptr));

        params.clear();
        params.push_back(gccjit_context_->new_param(the_var_type, "s"));
        params.push_back(gccjit_context_->new_param(the_var_type, "h"));
        params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
        params.push_back(gccjit_context_->new_param(the_var_type, "v"));
        params.push_back(gccjit_context_->new_param(the_size_t_type, "pos"));

        args.clear();
        args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
        args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
        args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
        args.push_back(iter_dst);
        args.push_back(iter);
        gccjit::function value_at_func =
                gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR),
                                              "TableValueByPos", params, 0, NewLocation(namelist_ptr));
        auto value_ret = gccjit_context_->new_call(value_at_func, args, NewLocation(namelist_ptr));

        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.add_assignment(value, value_ret, NewLocation(namelist_ptr));
        SaveStackLvalueByName(value_name, value, namelist_ptr);
    }

    CompileStmtBlock(func, block_ptr);

    cur_function_data_.stack_end_blocks.pop_back();

    if (!IsBlockEnded()) {
        step_block = func.new_block(NewBlockName("for in step", fs));

        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.end_with_jump(step_block, NewLocation(fs));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
        cur_function_data_.cur_block = step_block;

        // step the iterator var, eg: pre0 = pre0 + 1
        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.add_assignment_op(iter, GCC_JIT_BINARY_OP_PLUS, gccjit_context_->one(the_size_t_type),
                                                       NewLocation(fs));

        DEBUG_ASSERT(!IsBlockEnded());
        cur_function_data_.cur_block.end_with_jump(cond_block, NewLocation(fs));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    }

    cur_function_data_.cur_block = after_block;
}

void GccJitter::SetVarInt(gccjit::lvalue &var, int64_t v, bool is_const, const SyntaxTreeInterfacePtr &p) {
    // set type
    cur_function_data_.cur_block.add_assignment(var.access_field(var_type_field_),
                                                gccjit_context_->new_rvalue(int_type_, (int) static_cast<int>(VarType::Int)),
                                                NewLocation(p));

    // set data.i = v
    cur_function_data_.cur_block.add_assignment(var.access_field(var_data_field_).access_field(var_data_i_field_),
                                                gccjit_context_->new_rvalue(int_type_, static_cast<long>(v)), NewLocation(p));
}

void GccJitter::SetVarFloat(gccjit::lvalue &var, double v, bool is_const, const SyntaxTreeInterfacePtr &p) {
    // set type
    cur_function_data_.cur_block.add_assignment(var.access_field(var_type_field_),
                                                gccjit_context_->new_rvalue(int_type_, (int) static_cast<int>(VarType::Float)),
                                                NewLocation(p));

    // set data.f = v
    cur_function_data_.cur_block.add_assignment(var.access_field(var_data_field_).access_field(var_data_f_field_),
                                                gccjit_context_->new_rvalue(double_type_, v), NewLocation(p));
}

void GccJitter::SetVarString(gccjit::lvalue &var, const std::string &v, bool is_const, const SyntaxTreeInterfacePtr &p) {
    auto container_str = std::dynamic_pointer_cast<State>(sp_)->get_var_string_heap().alloc(v);
    // set type
    cur_function_data_.cur_block.add_assignment(var.access_field(var_type_field_),
                                                gccjit_context_->new_rvalue(int_type_, (int) static_cast<int>(VarType::String)),
                                                NewLocation(p));

    // set data.s = container_str
    cur_function_data_.cur_block.add_assignment(var.access_field(var_data_field_).access_field(var_data_s_field_),
                                                gccjit_context_->new_rvalue(void_ptr_type_, (void *) container_str), NewLocation(p));
}

}// namespace fakelua
