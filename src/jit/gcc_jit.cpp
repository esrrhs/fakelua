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
    if (!result) {
        // should not happen, but just in case
        throw_fakelua_exception("gcc_jitter compile failed");
    }
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
        if (!func) {
            throw_fakelua_exception("gcc_jit_result_get_code failed " + name);
        }
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
    stack_frame sf;
    for (auto &param: func_params) {
        sf.local_vars[param.first] = param.second;
    }
    stack_frames_.clear();
    stack_frames_.push_back(sf);

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);

    // get every second in func_params
    std::vector<gccjit::param> call_func_params;
    std::transform(func_params.begin(), func_params.end(), std::back_inserter(call_func_params),
                   [](const auto &pair) { return pair.second; });
    auto func = gccjit_context_->new_function(GCC_JIT_FUNCTION_EXPORTED, the_var_type, name.c_str(), call_func_params, is_variadic,
                                              new_location(funcbody_ptr));

    auto block = funcbody_ptr->block();
    compile_block(func, block);

    // save the function info
    function_infos_[name] = {static_cast<int>(actual_params_count), is_variadic > 0};
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
        std::regex e("^[-+]?[0-9]+$");
        if (std::regex_match(value, e)) {
            func_name = is_const ? "new_const_var_int" : "new_var_int";
            auto the_int_type = gccjit_context_->get_type(GCC_JIT_TYPE_INT64_T);
            params.push_back(gccjit_context_->new_param(the_int_type, "val"));

            int64_t val = std::stoll(value);
            args.push_back(gccjit_context_->new_rvalue(the_int_type, (long) val));
        } else {
            func_name = is_const ? "new_const_var_float" : "new_var_float";
            auto the_float_type = gccjit_context_->get_type(GCC_JIT_TYPE_DOUBLE);
            params.push_back(gccjit_context_->new_param(the_float_type, "val"));

            double val = std::stod(value);
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
        func_name = "new_var_wrap";
        params.push_back(gccjit_context_->new_param(the_var_type, "val"));
        auto pe = e->right();
        args.push_back(compile_prefixexp(pe, is_const));
    } else if (exp_type == "var_params") {
        func_name = "new_var_wrap";
        params.push_back(gccjit_context_->new_param(the_var_type, "val"));
        args.push_back(find_lvalue_by_name("__fakelua_variadic__", e));
    } else if (exp_type == "tableconstructor") {
        func_name = "new_var_wrap";
        params.push_back(gccjit_context_->new_param(the_var_type, "val"));
        auto tc = e->right();
        args.push_back(compile_tableconstructor(tc, is_const));
    } else if (exp_type == "binop") {
        // TODO
        return nullptr;
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
    stack_frames_.push_back(stack_frame());

    auto &stmts = block_ptr->stmts();
    for (auto &stmt: stmts) {
        compile_stmt(func, the_block, stmt);
    }

    // free stack frame
    stack_frames_.pop_back();

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
        // return nothing
        the_block.end_with_return(new_location(return_stmt));
        return;
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

    for (auto &kv: global_const_vars_) {
        auto name = kv.first;
        auto dst = kv.second.first;
        auto exp = kv.second.second;

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
    for (auto iter = stack_frames_.rbegin(); iter != stack_frames_.rend(); ++iter) {
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
    for (auto iter = stack_frames_.rbegin(); iter != stack_frames_.rend(); ++iter) {
        auto &local_vars = iter->local_vars;
        auto iter2 = local_vars.find(name);
        if (iter2 != local_vars.end()) {
            throw_error("the local var name is duplicated: " + name, ptr);
        }
    }
    stack_frames_.back().local_vars[name] = value;
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
    int index = 1;
    for (auto &field: fields) {
        auto field_ret = compile_field(field, is_const, index);
        ret.push_back(field_ret.first);
        ret.push_back(field_ret.second);
    }

    return ret;
}

std::pair<gccjit::rvalue, gccjit::rvalue> gcc_jitter::compile_field(const syntax_tree_interface_ptr &field, bool is_const, int &index) {
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
            auto k = std::make_shared<syntax_tree_exp>(field->loc());
            k->set_type("number");
            k->set_value(std::to_string(index));
            auto k_ret = compile_exp(k, is_const);
            ret.first = k_ret;
            ++index;
        }

        auto exp = field_ptr->value();
        auto exp_ret = compile_exp(exp, is_const);

        ret.second = exp_ret;
    } else {
        throw_error("not support field type: " + field_type, field);
    }

    return ret;
}

}// namespace fakelua
