#include "gcc_jit.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
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
    LOG(INFO) << "start gcc_jitter::compile " << file_name;
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
        FILE *fp = fopen(logfilename.c_str(), "wb");
        if (fp) {
            gccjit_context_->set_logfile(fp, 0, 0);
            gcc_jit_handle_->set_log_fp(fp);
            LOG(INFO) << file_name << " gccjit log file: " << logfilename;
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

    // at last, compile the chunk
    auto result = gccjit_context_->compile();
    if (!result) {
        // should not happen, but just in case
        throw std::runtime_error("gcc_jitter compile failed");
    }
    gcc_jit_handle_->set_result(result);

    // dump to file
    if (cfg.debug_mode) {
        auto dumpfile = generate_tmp_filename("fakelua_gccjit_", ".c");
        gccjit_context_->dump_to_file(dumpfile, true);
        LOG(INFO) << file_name << " dump to file: " << dumpfile;
    }

    gccjit_context_->release();
    gccjit_context_ = nullptr;

    // register the function
    for (auto &name: function_names_) {
        auto func = gcc_jit_result_get_code(result, name.c_str());
        if (!func) {
            throw std::runtime_error("gcc_jit_result_get_code failed " + name);
        }
        std::dynamic_pointer_cast<state>(sp_)->get_vm().register_function(name, std::make_shared<vm_function>(gcc_jit_handle_, func));
        LOG(INFO) << "register function: " << name;
    }

    LOG(INFO) << "end gcc_jitter::compile " << file_name;
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
            function_names_.insert(name);
        } else if (stmt->type() == syntax_tree_type::syntax_tree_type_local_function) {
            auto func = std::dynamic_pointer_cast<syntax_tree_local_function>(stmt);
            compile_function(func->name(), func->funcbody());
            function_names_.insert(func->name());
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
        ret = join_string(names, ".");
    }

    if (!name->colon_name().empty()) {
        ret += ":" + name->colon_name();
    }

    return ret;
}

void gcc_jitter::compile_function(const std::string &name, const syntax_tree_interface_ptr &funcbody) {
    LOG(INFO) << "compile function: " << name;

    check_syntax_tree_type(funcbody, {syntax_tree_type::syntax_tree_type_funcbody});
    auto funcbody_ptr = std::dynamic_pointer_cast<syntax_tree_funcbody>(funcbody);

    auto parlist = funcbody_ptr->parlist();
    int is_variadic = 0;
    std::vector<gccjit::param> func_params;
    if (parlist) {
        func_params = compile_parlist(parlist, is_variadic);
    }

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);

    auto func = gccjit_context_->new_function(GCC_JIT_FUNCTION_EXPORTED, the_var_type, name.c_str(), func_params, is_variadic,
                                              new_location(funcbody_ptr));

    auto block = funcbody_ptr->block();
    auto the_block = compile_block(func, block);
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
            throw std::runtime_error("the chunk top level only support const define and function define");
        }
    }
}

void gcc_jitter::compile_const_define(const syntax_tree_interface_ptr &stmt) {
    auto local_var = std::dynamic_pointer_cast<syntax_tree_local_var>(stmt);
    check_syntax_tree_type(local_var->namelist(), {syntax_tree_type::syntax_tree_type_namelist});
    auto keys = std::dynamic_pointer_cast<syntax_tree_namelist>(local_var->namelist());
    auto &names = keys->names();
    if (!local_var->explist()) {
        throw std::runtime_error("the const define must have a value, but the value is null, it's useless");
    }
    check_syntax_tree_type(local_var->explist(), {syntax_tree_type::syntax_tree_type_explist});
    auto values = std::dynamic_pointer_cast<syntax_tree_explist>(local_var->explist());
    auto &values_exps = values->exps();

    for (size_t i = 0; i < names.size(); ++i) {
        auto name = names[i];
        if (i >= values_exps.size()) {
            throw std::runtime_error("the const define not match, the value is not enough");
        }

        LOG(INFO) << "compile const define: " << name;

        // TODO gcc_jit_context_new_global
    }
}

gccjit::rvalue gcc_jitter::compile_exp(gccjit::function &func, gccjit::block &the_block, const syntax_tree_interface_ptr &exp) {
    // the chunk must be an exp
    check_syntax_tree_type(exp, {syntax_tree_type::syntax_tree_type_exp});
    // start compile the expression
    auto e = std::dynamic_pointer_cast<syntax_tree_exp>(exp);
    const auto &exp_type = e->exp_type();
    const auto &value = e->exp_value();

    auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);

    std::vector<gccjit::param> params;
    params.push_back(gccjit_context_->new_param(the_var_type, "s"));

    std::string func_name;

    std::vector<gccjit::rvalue> args;
    args.push_back(gccjit_context_->new_rvalue(the_var_type, sp_.get()));

    if (exp_type == "nil") {
        func_name = "new_var_nil";
    } else if (exp_type == "false") {
        func_name = "new_var_false";
    } else if (exp_type == "true") {
        func_name = "new_var_true";
    } else if (exp_type == "number") {
        std::regex e("^[-+]?[0-9]+$");
        if (std::regex_match(value, e)) {
            func_name = "new_var_int";
            auto the_int_type = gccjit_context_->get_type(GCC_JIT_TYPE_INT64_T);
            params.push_back(gccjit_context_->new_param(the_int_type, "val"));

            int64_t val = std::stoll(value);
            args.push_back(gccjit_context_->new_rvalue(the_int_type, (long) val));
        } else {
            func_name = "new_var_float";
            auto the_float_type = gccjit_context_->get_type(GCC_JIT_TYPE_DOUBLE);
            params.push_back(gccjit_context_->new_param(the_float_type, "val"));

            double val = std::stod(value);
            args.push_back(gccjit_context_->new_rvalue(the_float_type, val));
        }
    } else if (exp_type == "string") {
        func_name = "new_var_string";
        auto the_string_type = gccjit_context_->get_type(GCC_JIT_TYPE_CONST_CHAR_PTR);
        params.push_back(gccjit_context_->new_param(the_string_type, "val"));

        auto container_str = gcc_jit_handle_->alloc_str(value);
        args.push_back(gccjit_context_->new_rvalue(the_string_type, (void *) container_str->c_str()));
    } else if (exp_type == "var") {
        // TODO
        return nullptr;
    } else if (exp_type == "function") {
        // TODO
        return nullptr;
    } else if (exp_type == "paren") {
        // TODO
        return nullptr;
    } else if (exp_type == "call") {
        // TODO
        return nullptr;
    } else if (exp_type == "index") {
        // TODO
        return nullptr;
    } else if (exp_type == "method") {
        // TODO
        return nullptr;
    } else if (exp_type == "vararg") {
        // TODO
        return nullptr;
    } else if (exp_type == "var_params") {
        // TODO
        return nullptr;
    } else if (exp_type == "functiondef") {
        // TODO
        return nullptr;
    } else if (exp_type == "prefixexp") {
        // TODO
        return nullptr;
    } else if (exp_type == "tableconstructor") {
        // TODO
        return nullptr;
    } else if (exp_type == "binop") {
        // TODO
        return nullptr;
    } else if (exp_type == "unop") {
        // TODO
        return nullptr;
    } else {
        throw std::runtime_error("not support exp type: " + exp_type);
    }

    if (func_name.empty()) {
        throw std::runtime_error("not support exp type: " + exp_type);
    }

    gccjit::function new_var_func =
            gccjit_context_->new_function(GCC_JIT_FUNCTION_IMPORTED, the_var_type, func_name, params, 0, new_location(e));
    auto ret = gccjit_context_->new_call(new_var_func, args, new_location(e));
    return ret;
}

std::vector<gccjit::param> gcc_jitter::compile_parlist(syntax_tree_interface_ptr parlist, int &is_variadic) {
    check_syntax_tree_type(parlist, {syntax_tree_type::syntax_tree_type_parlist});
    auto parlist_ptr = std::dynamic_pointer_cast<syntax_tree_parlist>(parlist);

    std::vector<gccjit::param> ret;

    if (!parlist_ptr->var_params()) {
        auto namelist = parlist_ptr->namelist();
        check_syntax_tree_type(namelist, {syntax_tree_type::syntax_tree_type_namelist});
        auto namelist_ptr = std::dynamic_pointer_cast<syntax_tree_namelist>(namelist);
        auto &param_names = namelist_ptr->names();

        auto the_var_type = gccjit_context_->get_type(GCC_JIT_TYPE_VOID_PTR);
        for (auto &name: param_names) {
            auto param = gccjit_context_->new_param(the_var_type, name, new_location(namelist_ptr));
            ret.push_back(param);
        }
    } else {
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

    auto &stmts = block_ptr->stmts();
    for (auto &stmt: stmts) {
        compile_stmt(func, the_block, stmt);
    }

    return the_block;
}

void gcc_jitter::compile_stmt(gccjit::function &func, gccjit::block &the_block, const syntax_tree_interface_ptr &stmt) {
    switch (stmt->type()) {
        case syntax_tree_type::syntax_tree_type_return: {
            compile_stmt_return(func, the_block, stmt);
            break;
        }
        default: {
            throw std::runtime_error(std::format("not support stmt type: {}", magic_enum::enum_name(stmt->type())));
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
        auto exp_ret = compile_exp(func, the_block, exp);
        ret.push_back(exp_ret);
    }

    return ret;
}

}// namespace fakelua
