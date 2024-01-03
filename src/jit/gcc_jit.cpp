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

void gcc_jitter::compile(fakelua_state_ptr sp, compile_config cfg, const std::string &file_name, const syntax_tree_interface_ptr &chunk) {
    LOG_INFO("start gcc_jitter::compile {}", file_name);
    gcc_jit_handle_ = std::make_shared<gcc_jit_handle>(sp.get());
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
        FILE *fp = fopen(logfilename.c_str(), "wb");
        if (fp) {
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

    // just walk through the chunk, and save the function declaration and then we can call the function by name
    // first, check the global const define, the const define must be an assignment expression at the top level
    compile_const_defines(chunk);

    // second, walk through the chunk, and save the function declaration
    compile_functions(chunk);

    // compile the global const define init func
    compile_const_defines_init_func();

    // at last, compile the chunk
    auto result = gccjit_context_->compile();
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
    call_const_defines_init_func();

    // register the function
    for (auto &ele: function_infos_) {
        auto &name = ele.first;
        auto &info = ele.second;
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
    check_syntax_tree_type(chunk, {syntax_tree_type::syntax_tree_type_block});
    // walk through the block
    auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk);
    for (auto &stmt: block->stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_function) {
            auto func = std::dynamic_pointer_cast<syntax_tree_function>(stmt);
            auto name = compile_funcname(func->funcname());
            compile_function(name, func->funcbody());
        } else if (stmt->type() == syntax_tree_type::syntax_tree_type_local_function) {
            auto func = std::dynamic_pointer_cast<syntax_tree_local_function>(stmt);
            compile_function(func->name(), func->funcbody());
        }
    }
}

std::string gcc_jitter::compile_funcname(const syntax_tree_interface_ptr &ptr) {
    check_syntax_tree_type(ptr, {syntax_tree_type::syntax_tree_type_funcname});

    std::string ret;

    auto name = std::dynamic_pointer_cast<syntax_tree_funcname>(ptr);
    auto funcnamelistptr = name->funcnamelist();

    if (funcnamelistptr) {
        check_syntax_tree_type(funcnamelistptr, {syntax_tree_type::syntax_tree_type_funcnamelist});
        auto funcnamelist = std::dynamic_pointer_cast<syntax_tree_funcnamelist>(funcnamelistptr);
        auto &names = funcnamelist->funcnames();
        // join the names with .
        ret = join_string(names, "_");
    }

    if (!name->colon_name().empty()) {
        ret += "_" + name->colon_name();
    }

    return ret;
}

void gcc_jitter::compile_function(const std::string &name, const syntax_tree_interface_ptr &funcbody) {
    LOG_INFO("start compile function: {}", name);

    check_syntax_tree_type(funcbody, {syntax_tree_type::syntax_tree_type_funcbody});
    auto funcbody_ptr = std::dynamic_pointer_cast<syntax_tree_funcbody>(funcbody);

    // reset data
    cur_function_data_ = function_data();

    // compile the input params
    auto parlist = funcbody_ptr->parlist();
    int is_variadic = 0;
    std::vector<std::pair<std::string, gccjit::param>> func_params;
    if (parlist) {
        func_params = compile_parlist(parlist, is_variadic);
    }
    auto actual_params_count = func_params.size();
    if (is_variadic) {
        // insert variadic in the front of params
        func_params.insert(func_params.begin(), std::make_pair("__fakelua_variadic__",
                                                               gccjit_context_->new_param(gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR),
                                                                                          "__fakelua_variadic__")));
    }

    // add params to new stack frame
    function_data::stack_frame sf;
    for (auto &param: func_params) {
        sf.local_vars[param.first] = param.second;
    }
    cur_function_data_.stack_frames.clear();
    cur_function_data_.stack_frames.push_back(sf);

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);

    // get every second in func_params
    std::vector<gccjit::param> call_func_params;
    std::transform(func_params.begin(), func_params.end(), std::back_inserter(call_func_params),
                   [](const auto &pair) { return pair.second; });
    auto func = gccjit_context_->new_function(GCC_JIT_FUNCTION_EXPORTED, the_var_type, name.c_str(), call_func_params, is_variadic,
                                              new_location(funcbody_ptr));

    // compile the function body
    auto block = funcbody_ptr->block();
    auto ret_block = compile_block(func, block);

    // check the return block, if the return block is not ended with return, we add a return nil
    check_return_block(func, ret_block, funcbody_ptr);

    // save the function info
    function_infos_[name] = {static_cast<int>(actual_params_count), is_variadic > 0};
}

void gcc_jitter::check_return_block(gccjit::function &func, gccjit::block &the_block, const syntax_tree_interface_ptr &ptr) {
    if (cur_function_data_.ended_blocks.find(the_block.get_inner_block()) == cur_function_data_.ended_blocks.end()) {
        // return nil
        auto stmt = std::make_shared<syntax_tree_return>(ptr->loc());
        compile_stmt_return(func, the_block, stmt);
    }
}

void gcc_jitter::compile_const_defines(const syntax_tree_interface_ptr &chunk) {
    // the chunk must be a block
    check_syntax_tree_type(chunk, {syntax_tree_type::syntax_tree_type_block});
    // walk through the block
    auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk);
    for (auto &stmt: block->stmts()) {
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
    auto local_var = std::dynamic_pointer_cast<syntax_tree_local_var>(stmt);
    check_syntax_tree_type(local_var->namelist(), {syntax_tree_type::syntax_tree_type_namelist});
    auto keys = std::dynamic_pointer_cast<syntax_tree_namelist>(local_var->namelist());
    auto &names = keys->names();
    if (!local_var->explist()) {
        throw_error("the const define must have a value, but the value is null, it's useless", local_var);
    }
    check_syntax_tree_type(local_var->explist(), {syntax_tree_type::syntax_tree_type_explist});
    auto values = std::dynamic_pointer_cast<syntax_tree_explist>(local_var->explist());
    auto &values_exps = values->exps();

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);

    for (size_t i = 0; i < names.size(); ++i) {
        auto name = names[i];
        if (i >= values_exps.size()) {
            throw_error("the const define not match, the value is not enough", values);
        }

        LOG_INFO("compile const define: {}", name);

        auto dst = gccjit_context_->new_global(GCC_JIT_GLOBAL_INTERNAL, the_var_type, name.c_str(), new_location(keys));

        dst.set_initializer_rvalue(gccjit_context_->new_rvalue(the_var_type, nullptr));

        if (global_const_vars_.find(name) != global_const_vars_.end()) {
            throw_error("the const define name is duplicated: " + name, values);
        }

        global_const_vars_[name] = std::make_pair(dst, values_exps[i]);
        global_const_vars_vec_.push_back(name);// we record the order of the const define
    }
}

gccjit::rvalue gcc_jitter::compile_exp(const syntax_tree_interface_ptr &exp, bool is_const) {
    // the chunk must be an exp
    check_syntax_tree_type(exp, {syntax_tree_type::syntax_tree_type_exp});
    // start compile the expression
    auto e = std::dynamic_pointer_cast<syntax_tree_exp>(exp);
    const auto &exp_type = e->exp_type();
    const auto &value = e->exp_value();

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, is_const ? "h" : "s"));

    std::string func_name;

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, is_const ? (void *) gcc_jit_handle_.get() : (void *) sp_.get()));

    if (exp_type == "nil") {
        func_name = is_const ? "new_const_var_nil" : "new_var_nil";
    } else if (exp_type == "false") {
        func_name = is_const ? "new_const_var_false" : "new_var_false";
    } else if (exp_type == "true") {
        func_name = is_const ? "new_const_var_true" : "new_var_true";
    } else if (exp_type == "number") {
        if (is_integer(value)) {
            func_name = is_const ? "new_const_var_int" : "new_var_int";
            auto the_int_type = gccjit_context_->get_type(GCC_JIT_TYPE_INT64_T);
            params.push_back(gccjit_context_->new_param(the_int_type, "val"));

            int64_t val = to_integer(value);
            args.push_back(gccjit_context_->new_rvalue(the_int_type, (long) val));
        } else {
            func_name = is_const ? "new_const_var_float" : "new_var_float";
            auto the_float_type = gccjit_context_->get_type(GCC_JIT_TYPE_DOUBLE);
            params.push_back(gccjit_context_->new_param(the_float_type, "val"));

            double val = to_float(value);
            args.push_back(gccjit_context_->new_rvalue(the_float_type, val));
        }
    } else if (exp_type == "string") {
        func_name = is_const ? "new_const_var_string" : "new_var_string";
        auto the_string_type = gccjit_context_->get_type(GCC_JIT_TYPE_CONST_CHAR_PTR);
        params.push_back(gccjit_context_->new_param(the_string_type, "val"));
        auto the_int_type = gccjit_context_->get_type(GCC_JIT_TYPE_INT);
        params.push_back(gccjit_context_->new_param(the_int_type, "len"));

        auto container_str = gcc_jit_handle_->alloc_str(value);
        args.push_back(gccjit_context_->new_rvalue(the_string_type, (void *) container_str.data()));
        args.push_back(gccjit_context_->new_rvalue(the_int_type, (int) container_str.size()));
    } else if (exp_type == "prefixexp") {
        auto pe = e->right();
        return compile_prefixexp(pe, is_const);
    } else if (exp_type == "var_params") {
        if (is_const) {
            throw_error("... can not be const", exp);
        }
        return find_lvalue_by_name("__fakelua_variadic__", e);
    } else if (exp_type == "tableconstructor") {
        auto tc = e->right();
        return compile_tableconstructor(tc, is_const);
    } else if (exp_type == "binop") {
        auto left = e->left();
        auto right = e->right();
        auto op = e->op();
        return compile_binop(left, right, op, is_const);
    } else if (exp_type == "unop") {
        // TODO
        return nullptr;
    } else {
        throw_error("not support exp type: " + exp_type, exp);
    }

    if (func_name.empty()) {
        throw_error("empty exp func_name: " + exp_type, exp);
    }

    gccjit::function new_var_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, func_name, params, 0, new_location(e));
    auto ret = gccjit_context_->new_call(new_var_func, args, new_location(e));
    return ret;
}

std::vector<std::pair<std::string, gccjit::param>> gcc_jitter::compile_parlist(syntax_tree_interface_ptr parlist, int &is_variadic) {
    check_syntax_tree_type(parlist, {syntax_tree_type::syntax_tree_type_parlist});
    auto parlist_ptr = std::dynamic_pointer_cast<syntax_tree_parlist>(parlist);

    std::vector<std::pair<std::string, gccjit::param>> ret;

    auto namelist = parlist_ptr->namelist();
    if (namelist) {
        check_syntax_tree_type(namelist, {syntax_tree_type::syntax_tree_type_namelist});
        auto namelist_ptr = std::dynamic_pointer_cast<syntax_tree_namelist>(namelist);
        auto &param_names = namelist_ptr->names();

        // check dumplicated
        std::set<std::string> param_names_set;
        // insert global names
        for (auto &ele: global_const_vars_) {
            param_names_set.insert(ele.first);
        }
        for (auto &name: param_names) {
            if (param_names_set.find(name) != param_names_set.end()) {
                throw_error("the param name is duplicated: " + name, namelist_ptr);
            }
            param_names_set.insert(name);
        }

        auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
        for (auto &name: param_names) {
            auto param = gccjit_context_->new_param(the_var_type, name, new_location(namelist_ptr));
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

gccjit::block gcc_jitter::compile_block(gccjit::function &func, const syntax_tree_interface_ptr &block) {
    check_syntax_tree_type(block, {syntax_tree_type::syntax_tree_type_block});
    auto block_ptr = std::dynamic_pointer_cast<syntax_tree_block>(block);

    auto the_block = func.new_block();

    // alloc new stack frame
    cur_function_data_.stack_frames.push_back(function_data::stack_frame());

    auto &stmts = block_ptr->stmts();
    for (auto &stmt: stmts) {
        compile_stmt(func, the_block, stmt);
    }

    // free stack frame
    cur_function_data_.stack_frames.pop_back();

    return the_block;
}

void gcc_jitter::compile_stmt(gccjit::function &func, gccjit::block &the_block, const syntax_tree_interface_ptr &stmt) {
    switch (stmt->type()) {
        case syntax_tree_type::syntax_tree_type_return: {
            compile_stmt_return(func, the_block, stmt);
            break;
        }
        case syntax_tree_type::syntax_tree_type_local_var: {
            compile_stmt_local_var(func, the_block, stmt);
            break;
        }
        case syntax_tree_type::syntax_tree_type_assign: {
            compile_stmt_assign(func, the_block, stmt);
            break;
        }
        default: {
            throw_error(std::format("not support stmt type: {}", magic_enum::enum_name(stmt->type())), stmt);
        }
    }
}

void gcc_jitter::compile_stmt_return(gccjit::function &func, gccjit::block &the_block, const syntax_tree_interface_ptr &stmt) {
    check_syntax_tree_type(stmt, {syntax_tree_type::syntax_tree_type_return});
    auto return_stmt = std::dynamic_pointer_cast<syntax_tree_return>(stmt);

    auto explist = return_stmt->explist();
    if (!explist) {
        // return nil
        explist = std::make_shared<syntax_tree_explist>(return_stmt->loc());
        auto exp = std::make_shared<syntax_tree_exp>(return_stmt->loc());
        exp->set_type("nil");
        std::dynamic_pointer_cast<syntax_tree_explist>(explist)->add_exp(exp);
    }

    auto explist_ret = compile_explist(func, the_block, explist);

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));
    params.push_back(gccjit_context_->new_param(gccjit_context_->get_type(GCC_JIT_TYPE_INT), "n"));

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(gccjit_context_->get_type(GCC_JIT_TYPE_INT), (int) explist_ret.size()));

    for (auto &exp_ret: explist_ret) {
        args.push_back(exp_ret);
    }

    gccjit::function wrap_return_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, "wrap_return_var", params, 1, new_location(explist));
    auto ret = gccjit_context_->new_call(wrap_return_func, args, new_location(explist));

    the_block.end_with_return(ret, new_location(return_stmt));

    cur_function_data_.ended_blocks.insert(the_block.get_inner_block());
}

std::vector<gccjit::rvalue> gcc_jitter::compile_explist(gccjit::function &func, gccjit::block &the_block,
                                                        const syntax_tree_interface_ptr &explist) {
    check_syntax_tree_type(explist, {syntax_tree_type::syntax_tree_type_explist});
    auto explist_ptr = std::dynamic_pointer_cast<syntax_tree_explist>(explist);

    std::vector<gccjit::rvalue> ret;
    auto &exps = explist_ptr->exps();
    for (auto &exp: exps) {
        auto exp_ret = compile_exp(exp, false);
        ret.push_back(exp_ret);
    }

    return ret;
}

void gcc_jitter::compile_const_defines_init_func() {
    auto the_void_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID);

    std::vector<gccjit::param> params;
    auto func = gccjit_context_->new_function(GCC_JIT_FUNCTION_EXPORTED, the_void_type, "__fakelua_global_const_defines_init__", params, 0,
                                              gccjit::location());

    auto the_block = func.new_block();

    for (auto &name: global_const_vars_vec_) {
        auto kv = global_const_vars_[name];
        auto dst = kv.first;
        auto exp = kv.second;

        auto exp_ret = compile_exp(exp, true);

        the_block.add_assignment(dst, exp_ret, new_location(exp));
    }

    the_block.end_with_return(gccjit::location());
}

void gcc_jitter::call_const_defines_init_func() {
    if (global_const_vars_.empty()) {
        return;
    }
    auto init_func = (void (*)()) gcc_jit_result_get_code(gcc_jit_handle_->get_result(), "__fakelua_global_const_defines_init__");
    if (!init_func) {
        throw_fakelua_exception("gcc_jit_result_get_code failed __fakelua_global_const_defines_init__");
    }
    init_func();
}

gccjit::rvalue gcc_jitter::compile_prefixexp(const syntax_tree_interface_ptr &pe, bool is_const) {
    check_syntax_tree_type(pe, {syntax_tree_type::syntax_tree_type_prefixexp});
    auto pe_ptr = std::dynamic_pointer_cast<syntax_tree_prefixexp>(pe);

    const auto &pe_type = pe_ptr->get_type();
    auto value = pe_ptr->get_value();

    if (pe_type == "var") {
        return compile_var(value, is_const);
    } else if (pe_type == "functioncall") {
        if (is_const) {
            throw_error("functioncall can not be const", pe);
        }
        // TODO
        return NULL;
    } else if (pe_type == "exp") {
        return compile_exp(value, is_const);
    } else {
        throw_error("not support prefixexp type: " + pe_type, pe);
    }
}

std::string gcc_jitter::location_str(const syntax_tree_interface_ptr &ptr) {
    return std::format("{}:{}:{}", file_name_, ptr->loc().begin.line, ptr->loc().begin.column);
}

gccjit::lvalue gcc_jitter::compile_var(const syntax_tree_interface_ptr &v, bool is_const) {
    check_syntax_tree_type(v, {syntax_tree_type::syntax_tree_type_var});
    auto v_ptr = std::dynamic_pointer_cast<syntax_tree_var>(v);

    const auto &type = v_ptr->get_type();
    if (type == "simple") {
        const auto &name = v_ptr->get_name();
        return find_lvalue_by_name(name, v_ptr);
    } else if (type == "square") {
        // TODO
        return NULL;
    } else if (type == "DOT") {
        // TODO
        return NULL;
    } else {
        throw_error("not support var type: " + type, v);
    }
}

gccjit::lvalue gcc_jitter::find_lvalue_by_name(const std::string &name, const syntax_tree_interface_ptr &ptr) {
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

    throw_error("can not find var: " + name, ptr);
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

void gcc_jitter::compile_stmt_local_var(gccjit::function &function, gccjit::block &the_block, const syntax_tree_interface_ptr &stmt) {
    check_syntax_tree_type(stmt, {syntax_tree_type::syntax_tree_type_local_var});
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
        auto exp_ret = compile_exp(nil_exp, false);
        the_block.add_assignment(dst, exp_ret, new_location(nil_exp));

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

    compile_stmt_assign(function, the_block, assign_stmt);
}

std::vector<gccjit::lvalue> gcc_jitter::compile_varlist(gccjit::function &func, gccjit::block &the_block,
                                                        const syntax_tree_interface_ptr &varlist) {
    check_syntax_tree_type(varlist, {syntax_tree_type::syntax_tree_type_varlist});
    auto varlist_ptr = std::dynamic_pointer_cast<syntax_tree_varlist>(varlist);

    std::vector<gccjit::lvalue> ret;
    auto &vars = varlist_ptr->vars();
    for (auto &var: vars) {
        auto lvalue = compile_var(var, false);
        ret.push_back(lvalue);
    }

    return ret;
}

void gcc_jitter::compile_stmt_assign(gccjit::function &function, gccjit::block &the_block, const syntax_tree_interface_ptr &stmt) {
    check_syntax_tree_type(stmt, {syntax_tree_type::syntax_tree_type_assign});
    auto assign = std::dynamic_pointer_cast<syntax_tree_assign>(stmt);

    auto vars = assign->varlist();
    auto varlist = compile_varlist(function, the_block, vars);

    auto exps = assign->explist();
    auto explist = compile_explist(function, the_block, exps);

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));
    params.push_back(gccjit_context_->new_param(gccjit_context_->get_type(GCC_JIT_TYPE_INT), "src_n"));
    params.push_back(gccjit_context_->new_param(gccjit_context_->get_type(GCC_JIT_TYPE_INT), "dst_n"));

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, (void *) sp_.get()));
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
    the_block.add_eval(ret, new_location(stmt));
}

gccjit::rvalue gcc_jitter::compile_tableconstructor(const syntax_tree_interface_ptr &tc, bool is_const) {
    check_syntax_tree_type(tc, {syntax_tree_type::syntax_tree_type_tableconstructor});
    auto tc_ptr = std::dynamic_pointer_cast<syntax_tree_tableconstructor>(tc);

    std::string func_name = is_const ? "new_const_var_table" : "new_var_table";
    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);

    std::vector<gccjit::rvalue> kvs;
    auto fieldlist = tc_ptr->fieldlist();
    if (fieldlist) {
        kvs = compile_fieldlist(fieldlist, is_const);
    }

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, is_const ? "h" : "s"));
    params.push_back(gccjit_context_->new_param(gccjit_context_->get_type(GCC_JIT_TYPE_INT), "n"));

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, is_const ? (void *) gcc_jit_handle_.get() : (void *) sp_.get()));
    args.push_back(gccjit_context_->new_rvalue(gccjit_context_->get_type(GCC_JIT_TYPE_INT), (int) kvs.size()));
    for (auto &kv: kvs) {
        args.push_back(kv);
    }

    gccjit::function new_table_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, func_name, params, 1, new_location(tc));
    auto ret = gccjit_context_->new_call(new_table_func, args, new_location(tc));
    return ret;
}

std::vector<gccjit::rvalue> gcc_jitter::compile_fieldlist(const syntax_tree_interface_ptr &fieldlist, bool is_const) {
    check_syntax_tree_type(fieldlist, {syntax_tree_type::syntax_tree_type_fieldlist});
    auto fieldlist_ptr = std::dynamic_pointer_cast<syntax_tree_fieldlist>(fieldlist);

    std::vector<gccjit::rvalue> ret;
    auto &fields = fieldlist_ptr->fields();
    for (auto &field: fields) {
        auto field_ret = compile_field(field, is_const);
        ret.push_back(field_ret.first);
        ret.push_back(field_ret.second);
    }

    return ret;
}

std::pair<gccjit::rvalue, gccjit::rvalue> gcc_jitter::compile_field(const syntax_tree_interface_ptr &field, bool is_const) {
    check_syntax_tree_type(field, {syntax_tree_type::syntax_tree_type_field});
    auto field_ptr = std::dynamic_pointer_cast<syntax_tree_field>(field);

    std::pair<gccjit::rvalue, gccjit::rvalue> ret;

    auto field_type = field_ptr->get_type();
    if (field_type == "object") {
        auto name = field_ptr->name();
        auto name_exp = std::make_shared<syntax_tree_exp>(field->loc());
        name_exp->set_type("string");
        name_exp->set_value(name);
        auto k = compile_exp(name_exp, is_const);

        auto exp = field_ptr->value();
        auto exp_ret = compile_exp(exp, is_const);

        ret.first = k;
        ret.second = exp_ret;
    } else if (field_type == "array") {
        auto key = field_ptr->key();
        if (key) {
            auto k_ret = compile_exp(key, is_const);
            ret.first = k_ret;
        } else {
            // use nullptr key, means use the auto increment key
            auto k_ret = gccjit_context_->new_rvalue(gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR), nullptr);
            ret.first = k_ret;
        }

        auto exp = field_ptr->value();
        auto exp_ret = compile_exp(exp, is_const);

        ret.second = exp_ret;
    } else {
        throw_error("not support field type: " + field_type, field);
    }

    return ret;
}

gccjit::rvalue gcc_jitter::compile_binop(const syntax_tree_interface_ptr &left, const syntax_tree_interface_ptr &right,
                                         const syntax_tree_interface_ptr &op, bool is_const) {
    check_syntax_tree_type(op, {syntax_tree_type::syntax_tree_type_binop});
    auto op_ptr = std::dynamic_pointer_cast<syntax_tree_binop>(op);
    auto opstr = op_ptr->get_op();

    DEBUG_ASSERT(opstr == "PLUS" || opstr == "MINUS" || opstr == "STAR" || opstr == "SLASH" || opstr == "DOUBLE_SLASH" || opstr == "POW" ||
                 opstr == "XOR" || opstr == "MOD" || opstr == "BITAND" || opstr == "BITNOT" || opstr == "BITOR" || opstr == "RIGHT_SHIFT" ||
                 opstr == "LEFT_SHIFT" || opstr == "CONCAT" || opstr == "LESS" || opstr == "LESS_EQUAL" || opstr == "MORE" ||
                 opstr == "MORE_EQUAL" || opstr == "EQUAL" || opstr == "NOT_EQUAL" || opstr == "AND" || opstr == "OR");

    if (opstr == "AND") {
        // TODO
    } else if (opstr == "OR") {
        // TODO
    }

    auto left_ret = compile_exp(left, is_const);
    auto right_ret = compile_exp(right, is_const);

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, is_const ? "h" : "s"));
    params.push_back(gccjit_context_->new_param(the_var_type, "left"));
    params.push_back(gccjit_context_->new_param(the_var_type, "right"));

    std::string func_name;

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, is_const ? (void *) gcc_jit_handle_.get() : (void *) sp_.get()));
    args.push_back(left_ret);
    args.push_back(right_ret);

    if (opstr == "PLUS") {
        func_name = is_const ? "binop_const_plus" : "binop_plus";
    } else if (opstr == "MINUS") {
        func_name = is_const ? "binop_const_minus" : "binop_minus";
    } else if (opstr == "STAR") {
        func_name = is_const ? "binop_const_star" : "binop_star";
    } else if (opstr == "SLASH") {
        func_name = is_const ? "binop_const_slash" : "binop_slash";
    } else if (opstr == "DOUBLE_SLASH") {
        func_name = is_const ? "binop_const_double_slash" : "binop_double_slash";
    } else if (opstr == "POW") {
        func_name = is_const ? "binop_const_pow" : "binop_pow";
    } else if (opstr == "XOR") {
        func_name = is_const ? "binop_const_xor" : "binop_xor";
    } else if (opstr == "MOD") {
        func_name = is_const ? "binop_const_mod" : "binop_mod";
    } else if (opstr == "BITAND") {
        func_name = is_const ? "binop_const_bitand" : "binop_bitand";
    } else if (opstr == "BITNOT") {
        func_name = is_const ? "binop_const_bitnot" : "binop_bitnot";
    } else if (opstr == "BITOR") {
        func_name = is_const ? "binop_const_bitor" : "binop_bitor";
    } else if (opstr == "RIGHT_SHIFT") {
        func_name = is_const ? "binop_const_right_shift" : "binop_right_shift";
    } else if (opstr == "LEFT_SHIFT") {
        func_name = is_const ? "binop_const_left_shift" : "binop_left_shift";
    } else if (opstr == "CONCAT") {
        func_name = is_const ? "binop_const_concat" : "binop_concat";
    } else if (opstr == "LESS") {
        func_name = is_const ? "binop_const_less" : "binop_less";
    } else if (opstr == "LESS_EQUAL") {
        func_name = is_const ? "binop_const_less_equal" : "binop_less_equal";
    } else if (opstr == "MORE") {
        func_name = is_const ? "binop_const_more" : "binop_more";
    } else if (opstr == "MORE_EQUAL") {
        func_name = is_const ? "binop_const_more_equal" : "binop_more_equal";
    } else if (opstr == "EQUAL") {
        func_name = is_const ? "binop_const_equal" : "binop_equal";
    } else if (opstr == "NOT_EQUAL") {
        func_name = is_const ? "binop_const_not_equal" : "binop_not_equal";
    }

    gccjit::function binop_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, func_name, params, 0, new_location(op));
    auto ret = gccjit_context_->new_call(binop_func, args, new_location(op));

    return ret;
}

}// namespace fakelua
