#include "gcc_jit.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "util/exception.h"
#include "util/file_util.h"
#include "vm.h"

namespace fakelua {

gcc_jitter::~gcc_jitter() {
    if (gccjit_context_) {
        gccjit_context_->release();
        gccjit_context_ = nullptr;
    }
}

void gcc_jitter::prepare_compile(const fakelua_state_ptr &sp, const compile_config &cfg, const std::string &file_name) {
    gcc_jit_handle_ = std::make_shared<gcc_jit_handle>();
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
        auto logfilename = generate_tmp_filename("fakelua_gccjit_", ".log");
        if (FILE *fp = fopen(logfilename.c_str(), "wb")) {
            gccjit_context_->set_logfile(fp, 0, 0);
            gcc_jit_handle_->set_log_fp(fp);
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
            var_string *s;
            var_table *t;
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

void gcc_jitter::compile(const fakelua_state_ptr &sp, const compile_config &cfg, const std::string &file_name,
                         const syntax_tree_interface_ptr &chunk) {
    LOG_INFO("start gcc_jitter::compile {}", file_name);

    prepare_compile(sp, cfg, file_name);

    // just walk through the chunk, and save the function declaration, and then we can call the function by name
    // first, check the global const definition; the const definition must be an assignment expression at the top level
    compile_const_defines(chunk);

    // second, walk through the chunk, and save the function declaration
    compile_functions(chunk);

    // at last, compile the chunk
    const auto result = gccjit_context_->compile();
    DEBUG_ASSERT(result);
    gcc_jit_handle_->set_result(result);

    // dump to file
    if (cfg.debug_mode) {
        auto dumpfile = generate_tmp_filename("fakelua_gccjit_", ".c");
        gccjit_context_->dump_to_file(dumpfile, true);
        LOG_INFO("{} gccjit dump file: {}", file_name, dumpfile);
    }

    gccjit_context_->release();
    gccjit_context_ = nullptr;

    // call the global const define init func
    call_global_init_func();

    // register the function
    for (auto &[name, info]: function_infos_) {
        auto func = gcc_jit_result_get_code(result, name.c_str());
        DEBUG_ASSERT(func);
        std::dynamic_pointer_cast<state>(sp_)->get_vm().register_function(
                name, std::make_shared<vm_function>(gcc_jit_handle_, func, info.params_count, info.is_variadic));
        LOG_INFO("register function: {}", name);
    }

    LOG_INFO("end gcc_jitter::compile {}", file_name);
}

void gcc_jitter::compile_functions(const syntax_tree_interface_ptr &chunk) {
    // the chunk must be a block
    DEBUG_ASSERT(chunk->type() == syntax_tree_type::syntax_tree_type_block);
    // walk through the block
    for (const auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk); auto &stmt: block->stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_function) {
            const auto func = std::dynamic_pointer_cast<syntax_tree_function>(stmt);
            auto name = compile_funcname(func->funcname());
            compile_function(name, func->funcbody());
        } else if (stmt->type() == syntax_tree_type::syntax_tree_type_local_function) {
            const auto func = std::dynamic_pointer_cast<syntax_tree_local_function>(stmt);
            compile_function(func->name(), func->funcbody());
        }
    }
}

std::string gcc_jitter::compile_funcname(const syntax_tree_interface_ptr &ptr) {
    DEBUG_ASSERT(ptr->type() == syntax_tree_type::syntax_tree_type_funcname);

    std::string ret;

    const auto name = std::dynamic_pointer_cast<syntax_tree_funcname>(ptr);
    const auto funcnamelistptr = name->funcnamelist();

    std::vector<std::string> namelist;

    DEBUG_ASSERT(funcnamelistptr->type() == syntax_tree_type::syntax_tree_type_funcnamelist);
    const auto funcnamelist = std::dynamic_pointer_cast<syntax_tree_funcnamelist>(funcnamelistptr);
    const auto &names = funcnamelist->funcnames();
    namelist.insert(namelist.end(), names.begin(), names.end());

    DEBUG_ASSERT(namelist.size() == 1);
    DEBUG_ASSERT(name->colon_name().empty());

    return namelist[0];
}

void gcc_jitter::compile_function(const std::string &name, const syntax_tree_interface_ptr &funcbody) {
    LOG_INFO("start compile function: {}", name);

    DEBUG_ASSERT(funcbody->type() == syntax_tree_type::syntax_tree_type_funcbody);
    const auto funcbody_ptr = std::dynamic_pointer_cast<syntax_tree_funcbody>(funcbody);

    // reset data
    cur_function_data_ = function_data();
    cur_function_data_.is_const = name == "__fakelua_global_init__";
    cur_function_data_.cur_function_name = name;

    // compile the input params
    const auto parlist = funcbody_ptr->parlist();
    int is_variadic = 0;
    std::vector<std::pair<std::string, gccjit::param>> func_params;
    if (parlist) {
        func_params = compile_parlist(parlist, is_variadic);
    }
    const auto actual_params_count = func_params.size();
    if (is_variadic) {
        // insert variadic in the front of params
        func_params.insert(func_params.begin(), std::make_pair("__fakelua_variadic__",
                                                               gccjit_context_->new_param(gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR),
                                                                                          "__fakelua_variadic__")));
    }

    // add params to the new stack frame
    function_data::stack_frame sf;
    for (auto &[fst, snd]: func_params) {
        sf.local_vars[fst] = snd;
    }
    cur_function_data_.stack_frames.clear();
    cur_function_data_.stack_frames.push_back(sf);

    // get every second in func_params
    std::vector<gccjit::param> call_func_params;
    std::ranges::transform(func_params, std::back_inserter(call_func_params), [](const auto &pair) { return pair.second; });
    auto func = gccjit_context_->new_function(GCC_JIT_FUNCTION_EXPORTED, var_struct_, name.c_str(), call_func_params, is_variadic,
                                              new_location(funcbody_ptr));
    cur_function_data_.cur_gccjit_func = func;

    // compile the function body
    const auto block = funcbody_ptr->block();
    const auto the_block = func.new_block(new_block_name("block", block));
    cur_function_data_.cur_block = the_block;

    // init some const var if is const func
    if (cur_function_data_.is_const) {
        init_global_const_var(func);
    }

    compile_stmt_block(func, block);

    // check the return block, if the return block is not ended with return, we add a return nil
    check_return_block(func, funcbody_ptr);

    // save the function info
    function_infos_[name] = {static_cast<int>(actual_params_count), is_variadic > 0, func};
}

void gcc_jitter::check_return_block(gccjit::function &func, const syntax_tree_interface_ptr &ptr) {
    if (cur_function_data_.ended_blocks.find(cur_function_data_.cur_block.get_inner_block()) == cur_function_data_.ended_blocks.end()) {
        // return nil
        auto stmt = std::make_shared<syntax_tree_return>(ptr->loc());
        compile_stmt_return(func, stmt);
    }
}

bool gcc_jitter::is_block_ended() {
    if (cur_function_data_.ended_blocks.find(cur_function_data_.cur_block.get_inner_block()) != cur_function_data_.ended_blocks.end()) {
        return true;
    }
    return false;
}

void gcc_jitter::init_global_const_var(gccjit::function &func) {
    // init global_const_null_var_
    cur_function_data_.cur_block.add_comment("init const_null_var");
    // set type
    cur_function_data_.cur_block.add_assignment(global_const_null_var_.access_field(var_type_field_),
                                                gccjit_context_->new_rvalue(int_type_, static_cast<int>(var_type::VAR_NIL)));
    // flag |= 1
    cur_function_data_.cur_block.add_assignment_op(global_const_null_var_.access_field(var_flag_field_), GCC_JIT_BINARY_OP_BITWISE_OR,
                                                   gccjit_context_->new_rvalue(int_type_, (int) 1 << VAR_FLAG_CONST_IDX));

    // init global_const_false_var_
    cur_function_data_.cur_block.add_comment("init const_false_var");
    // set type
    cur_function_data_.cur_block.add_assignment(global_const_false_var_.access_field(var_type_field_),
                                                gccjit_context_->new_rvalue(int_type_, (int) static_cast<int>(var_type::VAR_BOOL)));
    // flag |= 1
    cur_function_data_.cur_block.add_assignment_op(global_const_false_var_.access_field(var_flag_field_), GCC_JIT_BINARY_OP_BITWISE_OR,
                                                   gccjit_context_->new_rvalue(int_type_, (int) 1 << VAR_FLAG_CONST_IDX));
    // set data.b = false
    cur_function_data_.cur_block.add_assignment(global_const_false_var_.access_field(var_data_field_).access_field(var_data_b_field_),
                                                gccjit_context_->new_rvalue(bool_type_, false));

    // init global_const_true_var_
    cur_function_data_.cur_block.add_comment("init const_true_var");
    // set type
    cur_function_data_.cur_block.add_assignment(global_const_true_var_.access_field(var_type_field_),
                                                gccjit_context_->new_rvalue(int_type_, (int) static_cast<int>(var_type::VAR_BOOL)));
    // flag |= 1
    cur_function_data_.cur_block.add_assignment_op(global_const_true_var_.access_field(var_flag_field_), GCC_JIT_BINARY_OP_BITWISE_OR,
                                                   gccjit_context_->new_rvalue(int_type_, (int) 1 << VAR_FLAG_CONST_IDX));
    // set data.b = true
    cur_function_data_.cur_block.add_assignment(global_const_true_var_.access_field(var_data_field_).access_field(var_data_b_field_),
                                                gccjit_context_->new_rvalue(bool_type_, true));
}

void gcc_jitter::compile_const_defines(const syntax_tree_interface_ptr &chunk) {
    // the chunk must be a block
    DEBUG_ASSERT(chunk->type() == syntax_tree_type::syntax_tree_type_block);
    // walk through the block
    for (const auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk); auto &stmt: block->stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_local_var) {
            compile_const_define(stmt);
        } else if (stmt->type() == syntax_tree_type::syntax_tree_type_function ||
                   stmt->type() == syntax_tree_type::syntax_tree_type_local_function) {
            // skip
        } else {
            throw_error("the chunk top level only support const define and function define", stmt);
        }
    }
}

void gcc_jitter::compile_const_define(const syntax_tree_interface_ptr &stmt) {
    const auto local_var = std::dynamic_pointer_cast<syntax_tree_local_var>(stmt);
    DEBUG_ASSERT(local_var->namelist()->type() == syntax_tree_type::syntax_tree_type_namelist);
    const auto keys = std::dynamic_pointer_cast<syntax_tree_namelist>(local_var->namelist());
    const auto &names = keys->names();
    DEBUG_ASSERT(local_var->explist());
    DEBUG_ASSERT(local_var->explist()->type() == syntax_tree_type::syntax_tree_type_explist);
    const auto values = std::dynamic_pointer_cast<syntax_tree_explist>(local_var->explist());
    const auto &values_exps = values->exps();

    for (size_t i = 0; i < names.size(); ++i) {
        auto name = names[i];
        DEBUG_ASSERT(i < values_exps.size());
        LOG_INFO("compile const define: {}", name);
        auto dst = gccjit_context_->new_global(GCC_JIT_GLOBAL_INTERNAL, var_struct_, name.c_str(), new_location(keys));
        if (global_const_vars_.contains(name)) {
            throw_error("the const define name is duplicated: " + name, values);
        }
        global_const_vars_[name] = std::make_pair(dst, values_exps[i]);
    }
}

gccjit::rvalue gcc_jitter::compile_exp(gccjit::function &func, const syntax_tree_interface_ptr &exp) {
    // the chunk must be an exp
    DEBUG_ASSERT(exp->type() == syntax_tree_type::syntax_tree_type_exp);
    // start to compile the expression
    const auto e = std::dynamic_pointer_cast<syntax_tree_exp>(exp);
    const auto &exp_type = e->exp_type();
    const auto &value = e->exp_value();

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

    DEBUG_ASSERT(exp_type == "nil" || exp_type == "false" || exp_type == "true" || exp_type == "number" || exp_type == "string" ||
                 exp_type == "prefixexp" || exp_type == "var_params" || exp_type == "tableconstructor" || exp_type == "binop" ||
                 exp_type == "unop")

    if (exp_type == "nil") {
        return global_const_null_var_;
    } else if (exp_type == "false") {
        return global_const_false_var_;
    } else if (exp_type == "true") {
        return global_const_true_var_;
    } else if (exp_type == "number") {
        if (is_integer(value)) {
            cur_function_data_.cur_block.add_comment(std::format("new int var {}", value), new_location(e));
            auto tmp_int =
                    func.new_local(var_struct_, std::format("__fakelua_jit_tmp_int_{}__", cur_function_data_.tmp_index++), new_location(e));
            set_var_int(tmp_int, to_integer(value), is_const, e);
            return tmp_int;
        } else {
            cur_function_data_.cur_block.add_comment(std::format("new float var {}", value), new_location(e));
            auto tmp_float = func.new_local(var_struct_, std::format("__fakelua_jit_tmp_float_{}__", cur_function_data_.tmp_index++),
                                            new_location(e));
            set_var_float(tmp_float, to_float(value), is_const, e);
            return tmp_float;
        }
    } else if (exp_type == "string") {
        cur_function_data_.cur_block.add_comment(std::format("new string var {}", value), new_location(e));
        auto tmp_string =
                func.new_local(var_struct_, std::format("__fakelua_jit_tmp_string_{}__", cur_function_data_.tmp_index++), new_location(e));
        set_var_string(tmp_string, value, is_const, e);
        return tmp_string;
    } else if (exp_type == "prefixexp") {
        const auto pe = e->right();
        return compile_prefixexp(func, pe);
    } else if (exp_type == "var_params") {
        if (is_const) {
            throw_error("... can not be const", exp);
        }
        return find_lvalue_by_name("__fakelua_variadic__", e);
    } else if (exp_type == "tableconstructor") {
        const auto tc = e->right();
        return compile_tableconstructor(func, tc);
    } else if (exp_type == "binop") {
        const auto left = e->left();
        const auto right = e->right();
        const auto op = e->op();
        return compile_binop(func, left, right, op);
    } else if (exp_type == "unop") {
        const auto right = e->right();
        const auto op = e->op();
        return compile_unop(func, right, op);
    }

    DEBUG_ASSERT(!func_name.empty());

    gccjit::function new_var_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, var_struct_.get_pointer(), func_name, params, 0, new_location(e));
    auto ret = gccjit_context_->new_call(new_var_func, args, new_location(e));
    return ret;
}

std::vector<std::pair<std::string, gccjit::param>> gcc_jitter::compile_parlist(syntax_tree_interface_ptr parlist, int &is_variadic) {
    DEBUG_ASSERT(parlist->type() == syntax_tree_type::syntax_tree_type_parlist);
    const auto parlist_ptr = std::dynamic_pointer_cast<syntax_tree_parlist>(parlist);

    std::vector<std::pair<std::string, gccjit::param>> ret;

    if (const auto namelist = parlist_ptr->namelist()) {
        DEBUG_ASSERT(namelist->type() == syntax_tree_type::syntax_tree_type_namelist);
        const auto namelist_ptr = std::dynamic_pointer_cast<syntax_tree_namelist>(namelist);
        auto &param_names = namelist_ptr->names();

        // check duplicated
        std::set<std::string> param_names_set;
        // insert global names
        for (const auto &key: global_const_vars_ | std::views::keys) {
            param_names_set.insert(key);
        }
        for (auto &name: param_names) {
            if (param_names_set.contains(name)) {
                throw_error("the param name is duplicated: " + name, namelist_ptr);
            }
            param_names_set.insert(name);
        }

        for (auto &name: param_names) {
            auto param = gccjit_context_->new_param(var_struct_, name, new_location(namelist_ptr));
            ret.push_back(std::make_pair(name, param));
        }
    }

    if (parlist_ptr->var_params()) {
        is_variadic = 1;
    }

    return ret;
}

gccjit::location gcc_jitter::new_location(const syntax_tree_interface_ptr &ptr) {
    return gccjit_context_->new_location(file_name_, ptr->loc().begin.line, ptr->loc().begin.column);
}

std::string gcc_jitter::new_block_name(const std::string &name, const syntax_tree_interface_ptr &ptr) {
    return std::format("{} {}:{}:{}", name, file_name_, ptr ? ptr->loc().begin.line : 0, ptr ? ptr->loc().begin.column : 0);
}

void gcc_jitter::compile_stmt(gccjit::function &func, const syntax_tree_interface_ptr &stmt) {
    DEBUG_ASSERT(!is_block_ended());

    cur_function_data_.cur_block.add_comment(stmt->dump(0), new_location(stmt));

    switch (stmt->type()) {
        case syntax_tree_type::syntax_tree_type_return: {
            compile_stmt_return(func, stmt);
            break;
        }
        case syntax_tree_type::syntax_tree_type_local_var: {
            compile_stmt_local_var(func, stmt);
            break;
        }
        case syntax_tree_type::syntax_tree_type_assign: {
            compile_stmt_assign(func, stmt);
            break;
        }
        case syntax_tree_type::syntax_tree_type_functioncall: {
            compile_stmt_functioncall(func, stmt);
            break;
        }
        case syntax_tree_type::syntax_tree_type_label: {
            compile_stmt_label(func, stmt);
            break;
        }
        case syntax_tree_type::syntax_tree_type_block: {
            compile_stmt_block(func, stmt);
            break;
        }
        case syntax_tree_type::syntax_tree_type_while: {
            compile_stmt_while(func, stmt);
            break;
        }
        case syntax_tree_type::syntax_tree_type_repeat: {
            compile_stmt_repeat(func, stmt);
            break;
        }
        case syntax_tree_type::syntax_tree_type_if: {
            compile_stmt_if(func, stmt);
            break;
        }
        case syntax_tree_type::syntax_tree_type_break: {
            compile_stmt_break(func, stmt);
            break;
        }
        case syntax_tree_type::syntax_tree_type_for_loop: {
            compile_stmt_for_loop(func, stmt);
            break;
        }
        case syntax_tree_type::syntax_tree_type_for_in: {
            compile_stmt_for_in(func, stmt);
            break;
        }
        default: {
            throw_error(std::format("not support stmt type: {}", magic_enum::enum_name(stmt->type())), stmt);
        }
    }
}

void gcc_jitter::compile_stmt_return(gccjit::function &func, const syntax_tree_interface_ptr &stmt) {
    DEBUG_ASSERT(stmt->type() == syntax_tree_type::syntax_tree_type_return);
    const auto return_stmt = std::dynamic_pointer_cast<syntax_tree_return>(stmt);

    auto explist = return_stmt->explist();
    if (!explist) {
        // return nil
        explist = std::make_shared<syntax_tree_explist>(return_stmt->loc());
        const auto exp = std::make_shared<syntax_tree_exp>(return_stmt->loc());
        exp->set_type("nil");
        std::dynamic_pointer_cast<syntax_tree_explist>(explist)->add_exp(exp);
    }

    // check if is simple return. e.g.: return 1
    const auto explist_ptr = std::dynamic_pointer_cast<syntax_tree_explist>(explist);
    DEBUG_ASSERT(!explist_ptr->exps().empty());
    if (explist_ptr->exps().size() == 1) {
        const auto exp = explist_ptr->exps()[0];
        const auto ret = compile_exp(func, exp);

        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.end_with_return(ret, new_location(return_stmt));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
        return;
    }

    auto explist_ret = compile_explist(func, explist);

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

    auto is_const = cur_function_data_.is_const;

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));
    params.push_back(gccjit_context_->new_param(the_var_type, "h"));
    params.push_back(gccjit_context_->new_param(the_bool_type, "is_const"));
    params.push_back(gccjit_context_->new_param(gccjit_context_->get_type(GCC_JIT_TYPE_INT), "n"));

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) gcc_jit_handle_.get()));
    args.push_back(gccjit_context_->new_rvalue(the_bool_type, is_const));
    args.push_back(gccjit_context_->new_rvalue(gccjit_context_->get_type(GCC_JIT_TYPE_INT), (int) explist_ret.size()));

    for (auto &exp_ret: explist_ret) {
        args.push_back(exp_ret);
    }

    gccjit::function wrap_return_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, "wrap_return_var", params, 1, new_location(explist));
    auto ret = gccjit_context_->new_call(wrap_return_func, args, new_location(explist));

    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.end_with_return(ret, new_location(return_stmt));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
}

std::vector<gccjit::rvalue> gcc_jitter::compile_explist(gccjit::function &func, const syntax_tree_interface_ptr &explist) {
    DEBUG_ASSERT(explist->type() == syntax_tree_type::syntax_tree_type_explist);
    auto explist_ptr = std::dynamic_pointer_cast<syntax_tree_explist>(explist);

    std::vector<gccjit::rvalue> ret;
    auto &exps = explist_ptr->exps();
    for (auto &exp: exps) {
        auto exp_ret = compile_exp(func, exp);
        ret.push_back(exp_ret);
    }

    return ret;
}

void gcc_jitter::call_global_init_func() {
    if (global_const_vars_.empty()) {
        return;
    }
    auto init_func = (void (*)()) gcc_jit_result_get_code(gcc_jit_handle_->get_result(), "__fakelua_global_init__");
    DEBUG_ASSERT(init_func);
    init_func();
}

gccjit::rvalue gcc_jitter::compile_prefixexp(gccjit::function &func, const syntax_tree_interface_ptr &pe) {
    DEBUG_ASSERT(pe->type() == syntax_tree_type::syntax_tree_type_prefixexp);
    auto pe_ptr = std::dynamic_pointer_cast<syntax_tree_prefixexp>(pe);

    const auto &pe_type = pe_ptr->get_type();
    auto value = pe_ptr->get_value();

    auto is_const = cur_function_data_.is_const;

    DEBUG_ASSERT(pe_type == "var" || pe_type == "functioncall" || pe_type == "exp");

    if (pe_type == "var") {
        return compile_var(func, value);
    } else if (pe_type == "functioncall") {
        if (is_const) {
            throw_error("functioncall can not be const", pe);
        }
        return compile_functioncall(func, value);
    } else /*if (pe_type == "exp")*/ {
        return compile_exp(func, value);
    }
}

std::string gcc_jitter::location_str(const syntax_tree_interface_ptr &ptr) {
    return std::format("{}:{}:{}", file_name_, ptr->loc().begin.line, ptr->loc().begin.column);
}

gccjit::rvalue gcc_jitter::compile_var(gccjit::function &func, const syntax_tree_interface_ptr &v) {
    DEBUG_ASSERT(v->type() == syntax_tree_type::syntax_tree_type_var);
    auto v_ptr = std::dynamic_pointer_cast<syntax_tree_var>(v);

    DEBUG_ASSERT(v_ptr->get_type() == "simple" || v_ptr->get_type() == "square" || v_ptr->get_type() == "dot");

    const auto &type = v_ptr->get_type();
    if (type == "simple") {
        const auto &name = v_ptr->get_name();
        return find_lvalue_by_name(name, v_ptr);
    } else if (type == "square") {
        auto pe = v_ptr->get_prefixexp();
        auto exp = v_ptr->get_exp();
        auto pe_ret = compile_prefixexp(func, pe);
        auto exp_ret = compile_exp(func, exp);

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

        gccjit::function table_index_func = gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, "table_index_by_var",
                                                                          params, 0, new_location(v_ptr));
        auto ret = gccjit_context_->new_call(table_index_func, args, new_location(v_ptr));

        return ret;
    } else /*if (type == "dot")*/ {
        auto pe = v_ptr->get_prefixexp();
        auto name = v_ptr->get_name();
        auto pe_ret = compile_prefixexp(func, pe);

        auto name_exp = std::make_shared<syntax_tree_exp>(v_ptr->loc());
        name_exp->set_type("string");
        name_exp->set_value(name);
        auto exp_ret = compile_exp(func, name_exp);

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

        gccjit::function table_index_func = gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, "table_index_by_var",
                                                                          params, 0, new_location(v_ptr));
        auto ret = gccjit_context_->new_call(table_index_func, args, new_location(v_ptr));

        return ret;
    }
}

gccjit::lvalue gcc_jitter::compile_var_lvalue(gccjit::function &func, const syntax_tree_interface_ptr &v) {
    DEBUG_ASSERT(v->type() == syntax_tree_type::syntax_tree_type_var);
    auto v_ptr = std::dynamic_pointer_cast<syntax_tree_var>(v);

    // we already change the code a.b = 1 to _pre = 1; set_table(_pre, a, b)
    // so here no "square" "dot", just "simple"
    DEBUG_ASSERT(v_ptr->get_type() == "simple");

    const auto &type = v_ptr->get_type();
    /*if (type == "simple")*/ {
        const auto &name = v_ptr->get_name();
        return find_lvalue_by_name(name, v_ptr);
    }
}

gccjit::lvalue gcc_jitter::find_lvalue_by_name(const std::string &name, const syntax_tree_interface_ptr &ptr) {
    auto ret = try_find_lvalue_by_name(name, ptr);
    if (!ret) {
        throw_error("can not find var: " + name, ptr);
    }
    return ret.value();
}

std::optional<gccjit::lvalue> gcc_jitter::try_find_lvalue_by_name(const std::string &name, const syntax_tree_interface_ptr &ptr) {
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

void gcc_jitter::save_stack_lvalue_by_name(const std::string &name, const gccjit::lvalue &value, const syntax_tree_interface_ptr &ptr) {
    // check global dumplicated
    if (global_const_vars_.find(name) != global_const_vars_.end()) {
        throw_error("the const define name is duplicated: " + name, ptr);
    }
    // check local dumplicated
    for (auto iter = cur_function_data_.stack_frames.rbegin(); iter != cur_function_data_.stack_frames.rend(); ++iter) {
        auto &local_vars = iter->local_vars;
        auto iter2 = local_vars.find(name);
        if (iter2 != local_vars.end()) {
            throw_error("the local var name is duplicated: " + name, ptr);
        }
    }
    cur_function_data_.stack_frames.back().local_vars[name] = value;
}

[[noreturn]] void gcc_jitter::throw_error(const std::string &msg, const syntax_tree_interface_ptr &ptr) {
    throw_fakelua_exception(std::format("{} at {}", msg, location_str(ptr)));
}

void gcc_jitter::compile_stmt_local_var(gccjit::function &function, const syntax_tree_interface_ptr &stmt) {
    DEBUG_ASSERT(stmt->type() == syntax_tree_type::syntax_tree_type_local_var);
    auto local_var = std::dynamic_pointer_cast<syntax_tree_local_var>(stmt);

    auto keys = std::dynamic_pointer_cast<syntax_tree_namelist>(local_var->namelist());
    auto &names = keys->names();

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);

    // first register the local vars
    for (size_t i = 0; i < names.size(); ++i) {
        auto name = names[i];
        LOG_INFO("compile local var: {}", name);

        auto dst = function.new_local(the_var_type, name, new_location(keys));

        // make it nil
        auto nil_exp = std::make_shared<syntax_tree_exp>(keys->loc());
        nil_exp->set_type("nil");
        auto exp_ret = compile_exp(function, nil_exp);

        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.add_assignment(dst, exp_ret, new_location(nil_exp));

        // add to local vars
        save_stack_lvalue_by_name(name, dst, keys);
    }

    // then make the assign
    if (!local_var->explist()) {
        // no value, just return
        return;
    }
    auto assign_stmt = std::make_shared<syntax_tree_assign>(stmt->loc());
    auto varlist = std::make_shared<syntax_tree_varlist>(stmt->loc());
    for (auto &name: names) {
        auto var = std::make_shared<syntax_tree_var>(stmt->loc());
        var->set_type("simple");
        var->set_name(name);
        varlist->add_var(var);
    }
    assign_stmt->set_varlist(varlist);
    assign_stmt->set_explist(local_var->explist());

    compile_stmt_assign(function, assign_stmt);
}

std::vector<gccjit::lvalue> gcc_jitter::compile_varlist_lvalue(gccjit::function &func, const syntax_tree_interface_ptr &varlist) {
    DEBUG_ASSERT(varlist->type() == syntax_tree_type::syntax_tree_type_varlist);
    auto varlist_ptr = std::dynamic_pointer_cast<syntax_tree_varlist>(varlist);

    std::vector<gccjit::lvalue> ret;
    auto &vars = varlist_ptr->vars();
    for (auto &var: vars) {
        auto lvalue = compile_var_lvalue(func, var);
        ret.push_back(lvalue);
    }

    return ret;
}

bool gcc_jitter::is_simple_assign(const syntax_tree_interface_ptr &vars, const syntax_tree_interface_ptr &exps) {
    DEBUG_ASSERT(vars->type() == syntax_tree_type::syntax_tree_type_varlist);
    DEBUG_ASSERT(exps->type() == syntax_tree_type::syntax_tree_type_explist);
    auto vars_ptr = std::dynamic_pointer_cast<syntax_tree_varlist>(vars);
    auto exps_ptr = std::dynamic_pointer_cast<syntax_tree_explist>(exps);

    auto &vars_vec = vars_ptr->vars();
    auto &exps_vec = exps_ptr->exps();

    for (size_t i = 0; i < vars_vec.size(); ++i) {
        auto &var = vars_vec[i];
        DEBUG_ASSERT(var->type() == syntax_tree_type::syntax_tree_type_var);
        auto var_ptr = std::dynamic_pointer_cast<syntax_tree_var>(var);
        DEBUG_ASSERT(var_ptr->get_type() == "simple");
    }

    for (size_t i = 0; i < exps_vec.size(); ++i) {
        auto &exp = exps_vec[i];
        DEBUG_ASSERT(exp->type() == syntax_tree_type::syntax_tree_type_exp);
        auto exp_ptr = std::dynamic_pointer_cast<syntax_tree_exp>(exp);
        if (!is_simple_exp(exp_ptr)) {
            return false;
        }
    }

    return true;
}

bool gcc_jitter::is_simple_args(const syntax_tree_interface_ptr &args) {
    DEBUG_ASSERT(args->type() == syntax_tree_type::syntax_tree_type_args);
    auto args_ptr = std::dynamic_pointer_cast<syntax_tree_args>(args);

    auto type = args_ptr->get_type();

    DEBUG_ASSERT(type == "explist" || type == "tableconstructor" || type == "string" || type == "empty");

    if (type == "explist") {
        auto explist = args_ptr->explist();
        return is_simple_explist(explist);
    } else if (type == "tableconstructor") {
        auto tc = args_ptr->tableconstructor();
        return is_simple_tableconstructor(tc);
    } else /*if (type == "string")*/ {
        return true;
    }
}

bool gcc_jitter::is_simple_explist(const syntax_tree_interface_ptr &explist) {
    DEBUG_ASSERT(explist->type() == syntax_tree_type::syntax_tree_type_explist);
    auto explist_ptr = std::dynamic_pointer_cast<syntax_tree_explist>(explist);

    std::vector<gccjit::rvalue> ret;
    auto &exps = explist_ptr->exps();
    for (auto &exp: exps) {
        if (!is_simple_exp(exp)) {
            return false;
        }
    }

    return true;
}

bool gcc_jitter::is_simple_exp(const syntax_tree_interface_ptr &exp) {
    DEBUG_ASSERT(exp->type() == syntax_tree_type::syntax_tree_type_exp);
    auto exp_ptr = std::dynamic_pointer_cast<syntax_tree_exp>(exp);

    const auto &exp_type = exp_ptr->exp_type();
    const auto &value = exp_ptr->exp_value();

    if (exp_type == "nil" || exp_type == "false" || exp_type == "true" || exp_type == "number" || exp_type == "string") {
        return true;
    } else if (exp_type == "prefixexp") {
        auto pe = exp_ptr->right();
        return is_simple_prefixexp(pe);
    } else if (exp_type == "tableconstructor") {
        auto tc = exp_ptr->right();
        return is_simple_tableconstructor(tc);
    } else if (exp_type == "binop") {
        auto left = exp_ptr->left();
        auto right = exp_ptr->right();
        auto op = exp_ptr->op();
        return is_simple_exp(left) && is_simple_exp(right);
    } else if (exp_type == "unop") {
        auto right = exp_ptr->right();
        auto op = exp_ptr->op();
        return is_simple_exp(right);
    } else {
        return false;
    }
}

bool gcc_jitter::is_simple_prefixexp(const syntax_tree_interface_ptr &pe) {
    DEBUG_ASSERT(pe->type() == syntax_tree_type::syntax_tree_type_prefixexp);
    auto pe_ptr = std::dynamic_pointer_cast<syntax_tree_prefixexp>(pe);

    const auto &pe_type = pe_ptr->get_type();
    auto value = pe_ptr->get_value();

    if (pe_type == "var") {
        return true;
    } else if (pe_type == "functioncall") {
        return false;
    } else if (pe_type == "exp") {
        return is_simple_exp(value);
    } else {
        return false;
    }
}

bool gcc_jitter::is_simple_tableconstructor(const syntax_tree_interface_ptr &tc) {
    DEBUG_ASSERT(tc->type() == syntax_tree_type::syntax_tree_type_tableconstructor);
    auto tc_ptr = std::dynamic_pointer_cast<syntax_tree_tableconstructor>(tc);

    auto fieldlist = tc_ptr->fieldlist();
    if (!fieldlist) {
        return true;
    }
    DEBUG_ASSERT(fieldlist->type() == syntax_tree_type::syntax_tree_type_fieldlist);
    auto fieldlist_ptr = std::dynamic_pointer_cast<syntax_tree_fieldlist>(fieldlist);

    auto &fields = fieldlist_ptr->fields();
    for (auto &field: fields) {
        if (!is_simple_field(field)) {
            return false;
        }
    }

    return true;
}

bool gcc_jitter::is_simple_field(const syntax_tree_interface_ptr &field) {
    DEBUG_ASSERT(field->type() == syntax_tree_type::syntax_tree_type_field);
    auto field_ptr = std::dynamic_pointer_cast<syntax_tree_field>(field);

    auto field_type = field_ptr->get_type();
    if (field_type == "object") {
        return is_simple_exp(field_ptr->value());
    } else if (field_type == "array") {
        if (field_ptr->key()) {
            return is_simple_exp(field_ptr->key()) && is_simple_exp(field_ptr->value());
        } else {
            return is_simple_exp(field_ptr->value());
        }
    } else {
        return false;
    }
}

void gcc_jitter::compile_stmt_assign(gccjit::function &function, const syntax_tree_interface_ptr &stmt) {
    DEBUG_ASSERT(stmt->type() == syntax_tree_type::syntax_tree_type_assign);
    auto assign = std::dynamic_pointer_cast<syntax_tree_assign>(stmt);

    auto vars = assign->varlist();
    auto varlist = compile_varlist_lvalue(function, vars);

    auto exps = assign->explist();
    auto explist = compile_explist(function, exps);

    if (is_simple_assign(vars, exps)) {
        // eg: a = 1
        for (size_t i = 0; i < varlist.size() && i < explist.size(); ++i) {
            auto &var = varlist[i];
            auto &exp = explist[i];
            DEBUG_ASSERT(!is_block_ended());
            cur_function_data_.cur_block.add_assignment(var, exp, new_location(stmt));
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
        auto var_addr = var.get_address(new_location(stmt));
        args.push_back(var_addr);
    }
    for (auto &exp_ret: explist) {
        args.push_back(exp_ret);
    }

    gccjit::function assign_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, "assign_var", params, 1, new_location(stmt));
    auto ret = gccjit_context_->new_call(assign_func, args, new_location(stmt));
    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.add_eval(ret, new_location(stmt));
}

gccjit::rvalue gcc_jitter::compile_tableconstructor(gccjit::function &func, const syntax_tree_interface_ptr &tc) {
    DEBUG_ASSERT(tc->type() == syntax_tree_type::syntax_tree_type_tableconstructor);
    auto tc_ptr = std::dynamic_pointer_cast<syntax_tree_tableconstructor>(tc);

    auto is_const = cur_function_data_.is_const;

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

    std::vector<gccjit::rvalue> kvs;
    auto fieldlist = tc_ptr->fieldlist();
    if (fieldlist) {
        kvs = compile_fieldlist(func, fieldlist);
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
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, "new_var_table", params, 1, new_location(tc));
    auto ret = gccjit_context_->new_call(new_table_func, args, new_location(tc));
    return ret;
}

std::vector<gccjit::rvalue> gcc_jitter::compile_fieldlist(gccjit::function &func, const syntax_tree_interface_ptr &fieldlist) {
    DEBUG_ASSERT(fieldlist->type() == syntax_tree_type::syntax_tree_type_fieldlist);
    auto fieldlist_ptr = std::dynamic_pointer_cast<syntax_tree_fieldlist>(fieldlist);

    std::vector<gccjit::rvalue> ret;
    auto &fields = fieldlist_ptr->fields();
    for (auto &field: fields) {
        auto field_ret = compile_field(func, field);
        ret.push_back(field_ret.first);
        ret.push_back(field_ret.second);
    }

    return ret;
}

std::pair<gccjit::rvalue, gccjit::rvalue> gcc_jitter::compile_field(gccjit::function &func, const syntax_tree_interface_ptr &field) {
    DEBUG_ASSERT(field->type() == syntax_tree_type::syntax_tree_type_field);
    auto field_ptr = std::dynamic_pointer_cast<syntax_tree_field>(field);

    std::pair<gccjit::rvalue, gccjit::rvalue> ret;

    auto field_type = field_ptr->get_type();
    DEBUG_ASSERT(field_type == "object" || field_type == "array");

    if (field_type == "object") {
        auto name = field_ptr->name();
        auto name_exp = std::make_shared<syntax_tree_exp>(field->loc());
        name_exp->set_type("string");
        name_exp->set_value(name);
        auto k = compile_exp(func, name_exp);

        auto exp = field_ptr->value();
        auto exp_ret = compile_exp(func, exp);

        ret.first = k;
        ret.second = exp_ret;
    } else if (field_type == "array") {
        auto key = field_ptr->key();
        if (key) {
            auto k_ret = compile_exp(func, key);
            ret.first = k_ret;
        } else {
            // use nullptr key, means use the auto increment key
            auto k_ret = gccjit_context_->new_rvalue(gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR), nullptr);
            ret.first = k_ret;
        }

        auto exp = field_ptr->value();
        auto exp_ret = compile_exp(func, exp);

        ret.second = exp_ret;
    }

    return ret;
}

gccjit::rvalue gcc_jitter::compile_binop(gccjit::function &func, const syntax_tree_interface_ptr &left,
                                         const syntax_tree_interface_ptr &right, const syntax_tree_interface_ptr &op) {
    DEBUG_ASSERT(op->type() == syntax_tree_type::syntax_tree_type_binop);
    auto op_ptr = std::dynamic_pointer_cast<syntax_tree_binop>(op);
    auto opstr = op_ptr->get_op();

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
        auto pre = func.new_local(the_var_type, pre_name, new_location(op));

        // var* pre=left
        auto left_ret = compile_exp(func, left);
        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.add_assignment(pre, left_ret, new_location(op));

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
            func_name = "test_var";
        } else if (opstr == "OR") {
            func_name = "test_not_var";
        }

        gccjit::function test_func = gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, gccjit_context_->get_type(GCC_JIT_TYPE_BOOL),
                                                                   func_name, params, 0, new_location(op));
        auto test_ret = gccjit_context_->new_call(test_func, args, new_location(op));

        auto then_block = func.new_block(new_block_name("then", op));
        auto else_block = func.new_block(new_block_name("else", op));
        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.end_with_conditional(test_ret, then_block, else_block, new_location(op));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());

        auto new_block = func.new_block(new_block_name("and end", op));

        // {pre=right;}
        cur_function_data_.cur_block = then_block;
        auto right_ret = compile_exp(func, right);
        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.add_assignment(pre, right_ret, new_location(op));
        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.end_with_jump(new_block, new_location(op));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());

        // {}
        cur_function_data_.cur_block = else_block;
        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.end_with_jump(new_block, new_location(op));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());

        cur_function_data_.cur_block = new_block;

        return pre;
    }

    auto left_ret = compile_exp(func, left);
    auto right_ret = compile_exp(func, right);

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
        func_name = "binop_plus";
    } else if (opstr == "MINUS") {
        func_name = "binop_minus";
    } else if (opstr == "STAR") {
        func_name = "binop_star";
    } else if (opstr == "SLASH") {
        func_name = "binop_slash";
    } else if (opstr == "DOUBLE_SLASH") {
        func_name = "binop_double_slash";
    } else if (opstr == "POW") {
        func_name = "binop_pow";
    } else if (opstr == "XOR") {
        func_name = "binop_xor";
    } else if (opstr == "MOD") {
        func_name = "binop_mod";
    } else if (opstr == "BITAND") {
        func_name = "binop_bitand";
    } else if (opstr == "BITOR") {
        func_name = "binop_bitor";
    } else if (opstr == "RIGHT_SHIFT") {
        func_name = "binop_right_shift";
    } else if (opstr == "LEFT_SHIFT") {
        func_name = "binop_left_shift";
    } else if (opstr == "CONCAT") {
        func_name = "binop_concat";
    } else if (opstr == "LESS") {
        func_name = "binop_less";
    } else if (opstr == "LESS_EQUAL") {
        func_name = "binop_less_equal";
    } else if (opstr == "MORE") {
        func_name = "binop_more";
    } else if (opstr == "MORE_EQUAL") {
        func_name = "binop_more_equal";
    } else if (opstr == "EQUAL") {
        func_name = "binop_equal";
    } else if (opstr == "NOT_EQUAL") {
        func_name = "binop_not_equal";
    }

    gccjit::function binop_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, func_name, params, 0, new_location(op));
    auto ret = gccjit_context_->new_call(binop_func, args, new_location(op));

    return ret;
}

gccjit::rvalue gcc_jitter::compile_unop(gccjit::function &func, const syntax_tree_interface_ptr &right,
                                        const syntax_tree_interface_ptr &op) {
    DEBUG_ASSERT(op->type() == syntax_tree_type::syntax_tree_type_unop);
    auto op_ptr = std::dynamic_pointer_cast<syntax_tree_unop>(op);
    auto opstr = op_ptr->get_op();

    DEBUG_ASSERT(opstr == "MINUS" || opstr == "NOT" || opstr == "NUMBER_SIGN" || opstr == "BITNOT");

    auto is_const = cur_function_data_.is_const;

    auto right_ret = compile_exp(func, right);

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
        func_name = "unop_minus";
    } else if (opstr == "NOT") {
        func_name = "unop_not";
    } else if (opstr == "NUMBER_SIGN") {
        func_name = "unop_number_sign";
    } else if (opstr == "BITNOT") {
        func_name = "unop_bitnot";
    }

    gccjit::function unop_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, func_name, params, 0, new_location(op));
    auto ret = gccjit_context_->new_call(unop_func, args, new_location(op));

    return ret;
}

gccjit::rvalue gcc_jitter::compile_functioncall(gccjit::function &func, const syntax_tree_interface_ptr &functioncall) {
    DEBUG_ASSERT(functioncall->type() == syntax_tree_type::syntax_tree_type_functioncall);
    auto functioncall_ptr = std::dynamic_pointer_cast<syntax_tree_functioncall>(functioncall);

    auto prefixexp = functioncall_ptr->prefixexp();

    gccjit::rvalue prefixexp_ret;

    // simple way, just call the function directly
    auto simple_name = get_simple_prefixexp_name(prefixexp);
    if (!simple_name.empty()) {
        // check if is var call
        auto find_ret = try_find_lvalue_by_name(simple_name, prefixexp);
        if (find_ret) {
            prefixexp_ret = find_ret.value();
        } else {
            // check if is jit builtin function
            if (is_jit_builtin_function(simple_name)) {
                // note: here maybe const function call
                auto is_const = cur_function_data_.is_const;

                auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
                auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

                auto args = functioncall_ptr->args();
                auto args_ret = compile_args(func, args);

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

                std::string func_name = get_jit_builtin_function_vm_name(simple_name);
                gccjit::function call_func = gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, func_name, params, 0,
                                                                           new_location(functioncall));
                auto ret = gccjit_context_->new_call(call_func, args2, new_location(functioncall));
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
                auto args = functioncall_ptr->args();
                auto args_ret = compile_args(func, args);

                // check all args is simple assign, just call it directly
                // if args not match, it will compile failed
                if (is_simple_args(args)) {
                    std::vector<gccjit::rvalue> args2;
                    for (auto &arg_ret: args_ret) {
                        args2.push_back(arg_ret);
                    }

                    gccjit::function call_func = local_call_func;
                    auto ret = gccjit_context_->new_call(call_func, args2, new_location(functioncall));
                    return ret;
                } else {
                    // same as global function call
                }
            }

            // is global function call, make it as a var
            auto name_exp = std::make_shared<syntax_tree_exp>(functioncall->loc());
            name_exp->set_type("string");
            name_exp->set_value(simple_name);
            prefixexp_ret = compile_exp(func, name_exp);
        }
    } else {
        // is var call, eg: a="test"; a();
        prefixexp_ret = compile_prefixexp(func, prefixexp);
    }

    // call with col, eg: a:b()
    gccjit::rvalue col_key;
    if (!functioncall_ptr->name().empty()) {
        DEBUG_ASSERT(functioncall_ptr->name().starts_with("__fakelua_pp_pre_"));
        col_key = find_lvalue_by_name(functioncall_ptr->name(), functioncall_ptr);
    } else {
        col_key = gccjit_context_->new_rvalue(gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR), nullptr);
    }

    // complex way, call the function by call_var
    auto args = functioncall_ptr->args();
    auto args_ret = compile_args(func, args);

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
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, "call_var", params, 1, new_location(functioncall));
    auto ret = gccjit_context_->new_call(call_func, args2, new_location(functioncall));
    return ret;
}

std::vector<gccjit::rvalue> gcc_jitter::compile_args(gccjit::function &func, const syntax_tree_interface_ptr &args) {
    DEBUG_ASSERT(args->type() == syntax_tree_type::syntax_tree_type_args);
    auto args_ptr = std::dynamic_pointer_cast<syntax_tree_args>(args);

    auto type = args_ptr->get_type();

    DEBUG_ASSERT(type == "explist" || type == "tableconstructor" || type == "string" || type == "empty");

    if (type == "explist") {
        auto explist = args_ptr->explist();
        return compile_explist(func, explist);
    } else if (type == "tableconstructor") {
        auto tc = args_ptr->tableconstructor();
        auto tc_ret = compile_tableconstructor(func, tc);
        return {tc_ret};
    } else if (type == "string") {
        auto str = args_ptr->string();
        auto str_ret = compile_exp(func, str);
        return {str_ret};
    }

    return {};
}

void gcc_jitter::compile_stmt_functioncall(gccjit::function &function, const syntax_tree_interface_ptr &stmt) {
    DEBUG_ASSERT(stmt->type() == syntax_tree_type::syntax_tree_type_functioncall);
    auto functioncall = std::dynamic_pointer_cast<syntax_tree_functioncall>(stmt);
    if (cur_function_data_.is_const) {
        // only can call __fakelua_set_table__ in const init function
        DEBUG_ASSERT(get_simple_prefixexp_name(functioncall->prefixexp()) == "__fakelua_set_table__");
    }
    auto ret = compile_functioncall(function, functioncall);
    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.add_eval(ret, new_location(stmt));
}

std::string gcc_jitter::get_simple_prefixexp_name(const syntax_tree_interface_ptr &pe) {
    DEBUG_ASSERT(pe->type() == syntax_tree_type::syntax_tree_type_prefixexp);
    auto pe_ptr = std::dynamic_pointer_cast<syntax_tree_prefixexp>(pe);

    auto pe_type = pe_ptr->get_type();
    auto value = pe_ptr->get_value();

    if (pe_type == "var") {
        return get_simple_var_name(value);
    } else {
        return "";
    }
}

std::string gcc_jitter::get_simple_var_name(const syntax_tree_interface_ptr &v) {
    DEBUG_ASSERT(v->type() == syntax_tree_type::syntax_tree_type_var);
    auto v_ptr = std::dynamic_pointer_cast<syntax_tree_var>(v);

    auto v_type = v_ptr->get_type();
    auto name = v_ptr->get_name();

    if (v_type == "simple") {
        return name;
    } else {
        return "";
    }
}

bool gcc_jitter::is_jit_builtin_function(const std::string &name) {
    return name == "__fakelua_set_table__";
}

std::string gcc_jitter::get_jit_builtin_function_vm_name(const std::string &name) {
    DEBUG_ASSERT(name == "__fakelua_set_table__");
    /*if (name == "__fakelua_set_table__")*/ { return "table_set"; }
}

void gcc_jitter::compile_stmt_label(gccjit::function &func, const fakelua::syntax_tree_interface_ptr &stmt) {
    DEBUG_ASSERT(stmt->type() == syntax_tree_type::syntax_tree_type_label);
    throw_error("not support label", stmt);
}

void gcc_jitter::compile_stmt_block(gccjit::function &func, const fakelua::syntax_tree_interface_ptr &block) {
    DEBUG_ASSERT(block->type() == syntax_tree_type::syntax_tree_type_block);
    const auto block_ptr = std::dynamic_pointer_cast<syntax_tree_block>(block);

    // alloc new stack frame
    cur_function_data_.stack_frames.push_back(function_data::stack_frame());

    for (const auto &stmts = block_ptr->stmts(); auto &stmt: stmts) {
        compile_stmt(func, stmt);
    }

    // free stack frame
    cur_function_data_.stack_frames.pop_back();
}

void gcc_jitter::compile_stmt_while(gccjit::function &func, const fakelua::syntax_tree_interface_ptr &wh) {
    DEBUG_ASSERT(wh->type() == syntax_tree_type::syntax_tree_type_while);
    auto while_ptr = std::dynamic_pointer_cast<syntax_tree_while>(wh);

    gccjit::block cond_block = func.new_block(new_block_name("loop cond", wh));
    gccjit::block body_block = func.new_block(new_block_name("loop body", wh));
    gccjit::block after_block = func.new_block(new_block_name("after loop", wh));

    // use to break jump
    cur_function_data_.stack_end_blocks.emplace_back(after_block);

    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.end_with_jump(cond_block, new_location(wh));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = cond_block;

    // while exp do block end
    auto exp = while_ptr->exp();
    auto block = while_ptr->block();

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);

    // make the exp
    auto cond_ret = compile_exp(func, exp);

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
                                                               "test_var", params, 0, new_location(exp));
    auto test_ret = gccjit_context_->new_call(test_func, args, new_location(exp));

    DEBUG_ASSERT(!is_block_ended());
    cond_block.end_with_conditional(test_ret, body_block, after_block, new_location(exp));

    cur_function_data_.cur_block = body_block;
    compile_stmt_block(func, block);

    cur_function_data_.stack_end_blocks.pop_back();

    if (!is_block_ended()) {
        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.end_with_jump(cond_block, new_location(wh));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    }
    cur_function_data_.cur_block = after_block;
}

void gcc_jitter::compile_stmt_repeat(gccjit::function &func, const syntax_tree_interface_ptr &re) {
    DEBUG_ASSERT(re->type() == syntax_tree_type::syntax_tree_type_repeat);
    auto repeat_ptr = std::dynamic_pointer_cast<syntax_tree_repeat>(re);

    gccjit::block cond_block;// maybe body just return, so cond and after maybe not exist
    gccjit::block body_block = func.new_block(new_block_name("loop body", re));
    gccjit::block after_block;

    // use to break jump
    cur_function_data_.stack_end_blocks.emplace_back(after_block);

    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.end_with_jump(body_block, new_location(re));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = body_block;

    // repeat block until cond
    auto exp = repeat_ptr->exp();
    auto block = repeat_ptr->block();

    compile_stmt_block(func, block);

    after_block = cur_function_data_.stack_end_blocks.back();
    cur_function_data_.stack_end_blocks.pop_back();

    if (!is_block_ended()) {
        cond_block = func.new_block(new_block_name("loop cond", re));
        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.end_with_jump(cond_block, new_location(re));
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
    auto cond_ret = compile_exp(func, exp);

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
                                                               "test_var", params, 0, new_location(exp));
    auto test_ret = gccjit_context_->new_call(test_func, args, new_location(exp));

    if (!after_block.get_inner_block()) {
        after_block = func.new_block(new_block_name("after loop", re));
    }
    DEBUG_ASSERT(!is_block_ended());
    cond_block.end_with_conditional(test_ret, after_block, body_block, new_location(exp));

    cur_function_data_.cur_block = after_block;
}

void gcc_jitter::compile_stmt_if(gccjit::function &func, const syntax_tree_interface_ptr &is) {
    DEBUG_ASSERT(is->type() == syntax_tree_type::syntax_tree_type_if);
    auto if_ptr = std::dynamic_pointer_cast<syntax_tree_if>(is);

    auto exp = if_ptr->exp();
    auto block = if_ptr->block();
    auto elseifs = if_ptr->elseifs();
    DEBUG_ASSERT(elseifs->type() == syntax_tree_type::syntax_tree_type_elseiflist);
    auto elseifs_ptr = std::dynamic_pointer_cast<syntax_tree_elseiflist>(elseifs);
    auto else_ptr = if_ptr->elseblock();// maybe nullptr

    gccjit::block cond_block = func.new_block(new_block_name("if cond", is));
    gccjit::block body_block = func.new_block(new_block_name("if body", is));
    gccjit::block else_block = func.new_block(new_block_name("if else", is));
    gccjit::block end_block;// maybe if and else all returned, so no need end_block. when use it, init it.

    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.end_with_jump(cond_block, new_location(exp));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = cond_block;

    // make the exp
    auto cond_ret = compile_exp(func, exp);

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
                                                               "test_var", params, 0, new_location(exp));
    auto test_ret = gccjit_context_->new_call(test_func, args, new_location(exp));

    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.end_with_conditional(test_ret, body_block, else_block, new_location(exp));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = body_block;

    compile_stmt_block(func, block);

    if (!is_block_ended()) {
        if (!end_block.get_inner_block()) {
            end_block = func.new_block(new_block_name("if end", is));
        }
        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.end_with_jump(end_block, new_location(is));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    }

    cur_function_data_.cur_block = else_block;
    for (size_t i = 0; i < elseifs_ptr->elseif_size(); i++) {
        auto elseifs_exp = elseifs_ptr->elseif_exp(i);
        auto elseifs_block = elseifs_ptr->elseif_block(i);

        auto elseifs_body_block = func.new_block(new_block_name("elseif body", is));
        auto next_else_block = func.new_block(new_block_name("elseif else", is));

        // make the exp
        auto elseifs_cond_ret = compile_exp(func, elseifs_exp);

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
                gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, gccjit_context_->get_type(GCC_JIT_TYPE_BOOL), "test_var",
                                              elseifs_params, 0, new_location(elseifs_exp));
        auto elseifs_test_ret = gccjit_context_->new_call(elseifs_test_func, elseifs_args, new_location(elseifs_exp));

        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.end_with_conditional(elseifs_test_ret, elseifs_body_block, next_else_block, new_location(elseifs_exp));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());

        cur_function_data_.cur_block = elseifs_body_block;
        compile_stmt_block(func, elseifs_block);

        if (!is_block_ended()) {
            if (!end_block.get_inner_block()) {
                end_block = func.new_block(new_block_name("if end", is));
            }
            DEBUG_ASSERT(!is_block_ended());
            cur_function_data_.cur_block.end_with_jump(end_block, new_location(is));
            cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
        }

        cur_function_data_.cur_block = next_else_block;
    }

    if (else_ptr) {
        auto else_body_block = func.new_block(new_block_name("else body", is));

        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.end_with_jump(else_body_block, new_location(is));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
        cur_function_data_.cur_block = else_body_block;

        compile_stmt_block(func, else_ptr);
    }

    if (!is_block_ended()) {
        if (!end_block.get_inner_block()) {
            end_block = func.new_block(new_block_name("if end", is));
        }
        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.end_with_jump(end_block, new_location(is));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    }

    if (end_block.get_inner_block()) {
        cur_function_data_.cur_block = end_block;
    }
}

void gcc_jitter::compile_stmt_break(gccjit::function &func, const syntax_tree_interface_ptr &bs) {
    DEBUG_ASSERT(bs->type() == syntax_tree_type::syntax_tree_type_break);
    auto break_ptr = std::dynamic_pointer_cast<syntax_tree_break>(bs);

    if (cur_function_data_.stack_end_blocks.empty()) {
        throw_error("break must in loop", bs);
    }

    auto &block = cur_function_data_.stack_end_blocks.back();
    if (!block.get_inner_block()) {
        block = func.new_block(new_block_name("break", bs));
    }
    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.end_with_jump(block, new_location(bs));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
}

void gcc_jitter::compile_stmt_for_loop(gccjit::function &func, const syntax_tree_interface_ptr &fs) {
    DEBUG_ASSERT(fs->type() == syntax_tree_type::syntax_tree_type_for_loop);
    auto for_loop_ptr = std::dynamic_pointer_cast<syntax_tree_for_loop>(fs);

    // for a = 1, 10, 1 do ... end
    auto name = for_loop_ptr->name();
    auto for_block_ptr = for_loop_ptr->block();
    auto exp_begin = for_loop_ptr->exp_begin();
    auto exp_end = for_loop_ptr->exp_end();
    auto exp_step = for_loop_ptr->exp_step();// maybe nullptr
    if (!exp_step) {
        // default is 1
        auto one_exp = std::make_shared<syntax_tree_exp>(exp_end->loc());
        one_exp->set_type("number");
        one_exp->set_value("1");
        exp_step = one_exp;
    }

    gccjit::block init_block = func.new_block(new_block_name("for loop init", fs));
    gccjit::block cond_block = func.new_block(new_block_name("for loop cond", fs));
    gccjit::block for_block = func.new_block(new_block_name("for loop body", fs));
    gccjit::block step_block;// maybe return in body block, so when use it, init it
    gccjit::block after_block = func.new_block(new_block_name("for loop after", fs));

    // use to break jump
    cur_function_data_.stack_end_blocks.emplace_back(after_block);

    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.end_with_jump(init_block, new_location(fs));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = init_block;

    auto init_ret = compile_exp(func, exp_begin);
    auto end_ret = compile_exp(func, exp_end);
    auto step_ret = compile_exp(func, exp_step);

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);
    auto is_const = cur_function_data_.is_const;

    // init the iterator var, eg: a = 1
    auto iter = func.new_local(the_var_type, name, new_location(exp_begin));
    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.add_assignment(iter, init_ret, new_location(exp_begin));
    // add to local vars
    save_stack_lvalue_by_name(name, iter, exp_begin);

    // init the end var, eg: pre = 10
    auto end_pre_name = std::format("__fakelua_jit_pre_{}__", cur_function_data_.tmp_index++);
    auto end_pre = func.new_local(the_var_type, end_pre_name, new_location(exp_end));
    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.add_assignment(end_pre, end_ret, new_location(exp_end));
    // add to local vars
    save_stack_lvalue_by_name(end_pre_name, end_pre, exp_end);

    // init the step var, eg: pre = 1
    auto step_pre_name = std::format("__fakelua_jit_pre_{}__", cur_function_data_.tmp_index++);
    auto step_pre = func.new_local(the_var_type, step_pre_name, new_location(exp_step));
    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.add_assignment(step_pre, step_ret, new_location(exp_step));
    // add to local vars
    save_stack_lvalue_by_name(step_pre_name, step_pre, exp_step);

    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.end_with_jump(cond_block, new_location(fs));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = cond_block;

    // check the iterator var, eg: a <= 10
    // make the cond exp
    auto cond_exp = std::make_shared<syntax_tree_exp>(exp_end->loc());
    cond_exp->set_type("binop");
    auto cond_op = std::make_shared<syntax_tree_binop>(exp_end->loc());
    cond_op->set_op("LESS_EQUAL");
    cond_exp->set_op(cond_op);
    auto cond_left_var = std::make_shared<syntax_tree_var>(exp_end->loc());
    cond_left_var->set_type("simple");
    cond_left_var->set_name(name);
    auto cond_left_prefix = std::make_shared<syntax_tree_prefixexp>(exp_end->loc());
    cond_left_prefix->set_type("var");
    cond_left_prefix->set_value(cond_left_var);
    auto cond_left_exp = std::make_shared<syntax_tree_exp>(exp_end->loc());
    cond_left_exp->set_type("prefixexp");
    cond_left_exp->set_right(cond_left_prefix);
    cond_exp->set_left(cond_left_exp);
    auto cond_right_var = std::make_shared<syntax_tree_var>(exp_end->loc());
    cond_right_var->set_type("simple");
    cond_right_var->set_name(end_pre_name);
    auto cond_right_prefix = std::make_shared<syntax_tree_prefixexp>(exp_end->loc());
    cond_right_prefix->set_type("var");
    cond_right_prefix->set_value(cond_right_var);
    auto cond_right_exp = std::make_shared<syntax_tree_exp>(exp_end->loc());
    cond_right_exp->set_type("prefixexp");
    cond_right_exp->set_right(cond_right_prefix);
    cond_exp->set_right(cond_right_exp);

    auto cond_ret = compile_exp(func, cond_exp);

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
                                                               "test_var", params, 0, new_location(cond_exp));
    auto test_ret = gccjit_context_->new_call(test_func, args, new_location(cond_exp));

    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.end_with_conditional(test_ret, for_block, after_block, new_location(exp_end));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = for_block;

    compile_stmt_block(func, for_block_ptr);

    cur_function_data_.stack_end_blocks.pop_back();

    if (!is_block_ended()) {
        step_block = func.new_block(new_block_name("for loop step", fs));
        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.end_with_jump(step_block, new_location(fs));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
        cur_function_data_.cur_block = step_block;

        // step the iterator var, eg: a = a + 1
        // make an assign stmt
        auto assign_stmt = std::make_shared<syntax_tree_assign>(exp_step->loc());
        auto assign_stmt_varlist = std::make_shared<syntax_tree_varlist>(exp_step->loc());
        auto assign_stmt_var = std::make_shared<syntax_tree_var>(exp_step->loc());
        assign_stmt_var->set_type("simple");
        assign_stmt_var->set_name(name);
        assign_stmt_varlist->add_var(assign_stmt_var);
        assign_stmt->set_varlist(assign_stmt_varlist);
        auto assign_stmt_explist = std::make_shared<syntax_tree_explist>(exp_step->loc());
        auto assign_stmt_exp = std::make_shared<syntax_tree_exp>(exp_step->loc());
        assign_stmt_exp->set_type("binop");
        auto assign_stmt_op = std::make_shared<syntax_tree_binop>(exp_step->loc());
        assign_stmt_op->set_op("PLUS");
        assign_stmt_exp->set_op(assign_stmt_op);
        auto assign_stmt_left_var = std::make_shared<syntax_tree_var>(exp_step->loc());
        assign_stmt_left_var->set_type("simple");
        assign_stmt_left_var->set_name(name);
        auto assign_stmt_left_prefix = std::make_shared<syntax_tree_prefixexp>(exp_step->loc());
        assign_stmt_left_prefix->set_type("var");
        assign_stmt_left_prefix->set_value(assign_stmt_left_var);
        auto assign_stmt_left_exp = std::make_shared<syntax_tree_exp>(exp_step->loc());
        assign_stmt_left_exp->set_type("prefixexp");
        assign_stmt_left_exp->set_right(assign_stmt_left_prefix);
        assign_stmt_exp->set_left(assign_stmt_left_exp);
        auto assign_stmt_right_var = std::make_shared<syntax_tree_var>(exp_step->loc());
        assign_stmt_right_var->set_type("simple");
        assign_stmt_right_var->set_name(step_pre_name);
        auto assign_stmt_right_prefix = std::make_shared<syntax_tree_prefixexp>(exp_step->loc());
        assign_stmt_right_prefix->set_type("var");
        assign_stmt_right_prefix->set_value(assign_stmt_right_var);
        auto assign_stmt_right_exp = std::make_shared<syntax_tree_exp>(exp_step->loc());
        assign_stmt_right_exp->set_type("prefixexp");
        assign_stmt_right_exp->set_right(assign_stmt_right_prefix);
        assign_stmt_exp->set_right(assign_stmt_right_exp);
        assign_stmt_explist->add_exp(assign_stmt_exp);
        assign_stmt->set_explist(assign_stmt_explist);

        compile_stmt(func, assign_stmt);

        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.end_with_jump(cond_block, new_location(fs));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    }

    cur_function_data_.cur_block = after_block;
}

void gcc_jitter::compile_stmt_for_in(gccjit::function &func, const syntax_tree_interface_ptr &fs) {
    DEBUG_ASSERT(fs->type() == syntax_tree_type::syntax_tree_type_for_in);
    auto for_in_ptr = std::dynamic_pointer_cast<syntax_tree_for_in>(fs);

    // for k, v in pairs(t) do ... end
    auto namelist = for_in_ptr->namelist();
    auto explist = for_in_ptr->explist();
    auto block_ptr = for_in_ptr->block();

    std::string key_name;
    std::string value_name;
    syntax_tree_interface_ptr for_args_ptr;

    // get k, v name from namelist
    auto namelist_ptr = std::dynamic_pointer_cast<syntax_tree_namelist>(namelist);
    auto names = namelist_ptr->names();
    if (names.size() != 1 && names.size() != 2) {
        throw_error(std::format("for in namelist size must be 1 or 2, but got {}", names.size()), namelist);
    }
    key_name = names[0];
    if (names.size() == 2) {
        value_name = names[1];
    }

    // check explist only have one exp
    auto explist_ptr = std::dynamic_pointer_cast<syntax_tree_explist>(explist);
    auto exps = explist_ptr->exps();
    if (exps.size() != 1) {
        throw_error(std::format("for in explist size must be 1, but got {}", exps.size()), explist);
    }
    auto exp = exps[0];
    // check exp must be ipairs() or pairs()
    DEBUG_ASSERT(exp->type() == syntax_tree_type::syntax_tree_type_exp);
    auto exp_ptr = std::dynamic_pointer_cast<syntax_tree_exp>(exp);
    if (exp_ptr->exp_type() != "prefixexp") {
        throw_error("for in exp (expect prefixexp) must be ipairs() or pairs()", exp);
    }
    auto prefixexp = exp_ptr->right();
    DEBUG_ASSERT(prefixexp->type() == syntax_tree_type::syntax_tree_type_prefixexp);
    auto prefixexp_ptr = std::dynamic_pointer_cast<syntax_tree_prefixexp>(prefixexp);
    if (prefixexp_ptr->get_type() != "functioncall") {
        throw_error("for in exp (expect functioncall) must be ipairs() or pairs()", exp);
    }
    auto functioncall = prefixexp_ptr->get_value();
    DEBUG_ASSERT(functioncall->type() == syntax_tree_type::syntax_tree_type_functioncall);
    auto functioncall_ptr = std::dynamic_pointer_cast<syntax_tree_functioncall>(functioncall);
    auto call_name = get_simple_prefixexp_name(functioncall_ptr->prefixexp());
    if (call_name != "ipairs" && call_name != "pairs") {
        throw_error("for in exp (expect ipairs/pairs) must be ipairs() or pairs()", exp);
    }
    auto for_args = functioncall_ptr->args();
    DEBUG_ASSERT(for_args);
    DEBUG_ASSERT(for_args->type() == syntax_tree_type::syntax_tree_type_args);
    for_args_ptr = std::dynamic_pointer_cast<syntax_tree_args>(for_args);

    // for in block, just like for in
    gccjit::block init_block = func.new_block(new_block_name("for in init", fs));
    gccjit::block cond_block = func.new_block(new_block_name("for in cond", fs));
    gccjit::block for_block = func.new_block(new_block_name("for in body", fs));
    gccjit::block step_block;// maybe return in body block, so when use it, init it
    gccjit::block after_block = func.new_block(new_block_name("for in after", fs));

    // use to break jump
    cur_function_data_.stack_end_blocks.emplace_back(after_block);

    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.end_with_jump(init_block, new_location(fs));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = init_block;

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
    auto the_bool_type = gccjit_context_->get_type(GCC_JIT_TYPE_BOOL);
    auto the_size_t_type = gccjit_context_->get_type(GCC_JIT_TYPE_SIZE_T);
    auto is_const = cur_function_data_.is_const;

    // init the iterator var, eg: size_t pre0 = 0
    auto iter_name = std::format("__fakelua_jit_pre_{}__", cur_function_data_.tmp_index++);
    auto iter = func.new_local(the_size_t_type, iter_name, new_location(exp));
    auto iter_init_value = gccjit_context_->zero(the_size_t_type);
    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.add_assignment(iter, iter_init_value, new_location(exp));

    // make the iterator dst
    auto args_ret = compile_args(func, for_args_ptr);
    if (args_ret.size() != 1) {
        throw_error(std::format("for in ipairs() or pairs() args size must be 1, but got {}", args_ret.size()), for_args);
    }
    // store in local, eg: pre1 = table
    auto iter_dst_name = std::format("__fakelua_jit_pre_{}__", cur_function_data_.tmp_index++);
    auto iter_dst = func.new_local(gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR), iter_dst_name, new_location(for_args_ptr));
    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.add_assignment(iter_dst, args_ret[0], new_location(for_args_ptr));

    // init the iterator end var, eg: size_t pre2 = size(pre1)
    auto iter_end_name = std::format("__fakelua_jit_pre_{}__", cur_function_data_.tmp_index++);
    auto iter_end = func.new_local(the_size_t_type, iter_end_name, new_location(for_args_ptr));

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
                                                               "table_size", params, 0, new_location(for_args_ptr));
    auto size_ret = gccjit_context_->new_call(size_func, args, new_location(for_args_ptr));

    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.add_assignment(iter_end, size_ret, new_location(for_args_ptr));

    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.end_with_jump(cond_block, new_location(fs));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = cond_block;

    // check cond
    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.end_with_conditional(iter < iter_end, for_block, after_block, new_location(exp));
    cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    cur_function_data_.cur_block = for_block;

    // start init cur loop local var, eg: k, v = pre1[pre0]
    auto key = func.new_local(the_var_type, key_name, new_location(namelist_ptr));

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
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR), "table_key_by_pos",
                                          params, 0, new_location(namelist_ptr));
    auto key_ret = gccjit_context_->new_call(key_at_func, args, new_location(namelist_ptr));

    DEBUG_ASSERT(!is_block_ended());
    cur_function_data_.cur_block.add_assignment(key, key_ret, new_location(namelist_ptr));
    save_stack_lvalue_by_name(key_name, key, namelist_ptr);

    if (!value_name.empty()) {
        auto value = func.new_local(the_var_type, value_name, new_location(namelist_ptr));

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
                                              "table_value_by_pos", params, 0, new_location(namelist_ptr));
        auto value_ret = gccjit_context_->new_call(value_at_func, args, new_location(namelist_ptr));

        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.add_assignment(value, value_ret, new_location(namelist_ptr));
        save_stack_lvalue_by_name(value_name, value, namelist_ptr);
    }

    compile_stmt_block(func, block_ptr);

    cur_function_data_.stack_end_blocks.pop_back();

    if (!is_block_ended()) {
        step_block = func.new_block(new_block_name("for in step", fs));

        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.end_with_jump(step_block, new_location(fs));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
        cur_function_data_.cur_block = step_block;

        // step the iterator var, eg: pre0 = pre0 + 1
        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.add_assignment_op(iter, GCC_JIT_BINARY_OP_PLUS, gccjit_context_->one(the_size_t_type),
                                                       new_location(fs));

        DEBUG_ASSERT(!is_block_ended());
        cur_function_data_.cur_block.end_with_jump(cond_block, new_location(fs));
        cur_function_data_.ended_blocks.insert(cur_function_data_.cur_block.get_inner_block());
    }

    cur_function_data_.cur_block = after_block;
}

void gcc_jitter::set_var_int(gccjit::lvalue &var, int64_t v, bool is_const, const syntax_tree_interface_ptr &p) {
    // set type
    cur_function_data_.cur_block.add_assignment(var.access_field(var_type_field_),
                                                gccjit_context_->new_rvalue(int_type_, (int) static_cast<int>(var_type::VAR_INT)),
                                                new_location(p));
    if (is_const) {
        // flag |= 1 << 0
        cur_function_data_.cur_block.add_assignment_op(var.access_field(var_flag_field_), GCC_JIT_BINARY_OP_BITWISE_OR,
                                                       gccjit_context_->new_rvalue(int_type_, (int) 1 << VAR_FLAG_CONST_IDX),
                                                       new_location(p));
    } else {
        // flag = 0
        cur_function_data_.cur_block.add_assignment(var.access_field(var_flag_field_), gccjit_context_->new_rvalue(int_type_, 0),
                                                    new_location(p));
    }
    // set data.i = v
    cur_function_data_.cur_block.add_assignment(var.access_field(var_data_field_).access_field(var_data_i_field_),
                                                gccjit_context_->new_rvalue(int_type_, static_cast<long>(v)), new_location(p));
}

void gcc_jitter::set_var_float(gccjit::lvalue &var, double v, bool is_const, const syntax_tree_interface_ptr &p) {
    // set type
    cur_function_data_.cur_block.add_assignment(var.access_field(var_type_field_),
                                                gccjit_context_->new_rvalue(int_type_, (int) static_cast<int>(var_type::VAR_FLOAT)),
                                                new_location(p));
    if (is_const) {
        // flag |= 1 << 0
        cur_function_data_.cur_block.add_assignment_op(var.access_field(var_flag_field_), GCC_JIT_BINARY_OP_BITWISE_OR,
                                                       gccjit_context_->new_rvalue(int_type_, (int) 1 << VAR_FLAG_CONST_IDX),
                                                       new_location(p));
    } else {
        // flag = 0
        cur_function_data_.cur_block.add_assignment(var.access_field(var_flag_field_), gccjit_context_->new_rvalue(int_type_, 0),
                                                    new_location(p));
    }
    // set data.f = v
    cur_function_data_.cur_block.add_assignment(var.access_field(var_data_field_).access_field(var_data_f_field_),
                                                gccjit_context_->new_rvalue(double_type_, v), new_location(p));
}

void gcc_jitter::set_var_string(gccjit::lvalue &var, const std::string &v, bool is_const, const syntax_tree_interface_ptr &p) {
    auto container_str = std::dynamic_pointer_cast<state>(sp_)->get_var_string_heap().alloc(v, is_const);
    // set type
    cur_function_data_.cur_block.add_assignment(var.access_field(var_type_field_),
                                                gccjit_context_->new_rvalue(int_type_, (int) static_cast<int>(var_type::VAR_STRING)),
                                                new_location(p));
    if (is_const) {
        // flag |= 1 << 0
        cur_function_data_.cur_block.add_assignment_op(var.access_field(var_flag_field_), GCC_JIT_BINARY_OP_BITWISE_OR,
                                                       gccjit_context_->new_rvalue(int_type_, (int) 1 << VAR_FLAG_CONST_IDX),
                                                       new_location(p));
    } else {
        // flag = 0
        cur_function_data_.cur_block.add_assignment(var.access_field(var_flag_field_), gccjit_context_->new_rvalue(int_type_, 0),
                                                    new_location(p));
    }
    // set data.s = container_str
    cur_function_data_.cur_block.add_assignment(var.access_field(var_data_field_).access_field(var_data_s_field_),
                                                gccjit_context_->new_rvalue(void_ptr_type_, (void *) container_str), new_location(p));
}

}// namespace fakelua
