#include "preprocessor.h"
#include "fakelua.h"
#include "state/state.h"
#include "util/common.h"
#include "util/exception.h"
#include "util/file_util.h"

namespace fakelua {

void pre_processor::process(const fakelua_state_ptr &sp, const compile_config &cfg, const std::string &file_name,
                            const syntax_tree_interface_ptr &chunk) {
    LOG_INFO("start pre_processor::process {}", file_name);

    sp_ = sp;
    file_name_ = file_name;
    int debug_step = 0;

    // extracts all literal constants (e.g., integers, floats, strings),
    // adds them to the const definition, and replaces them with the corresponding const name.
    preprocess_extracts_literal_constants(chunk);
    if (cfg.debug_mode) {
        dump_debug_file(chunk, debug_step++);
    }

    // make the const define 'a = 1' to 'a = nil', and add a new stmt 'a = 1' in function __fakelua_global_init__
    // because the const value is not supported function, so we need to make it to nil. and then assign.
    preprocess_const(chunk);
    if (cfg.debug_mode) {
        dump_debug_file(chunk, debug_step++);
    }

    // make the function name like 'function a.b.c()' to a temp name 'function __fakelua_global_1__()',
    // and add a new stmt 'xxx.yyy.zzz = "__fakelua_global_1__"' in function __fakelua_global_init__
    // also, we need to add 'self' in the front of the params if the function is a member function.
    preprocess_functions_name(chunk);
    if (cfg.debug_mode) {
        dump_debug_file(chunk, debug_step++);
    }

    // now we have funtion __fakelua_global_init__, we need to add it to the chunk. maybe later we will insert more stmts to it.
    save_preprocess_global_init(chunk);
    if (cfg.debug_mode) {
        dump_debug_file(chunk, debug_step++);
    }

    // change the table assign stmt like 'a.b.c = some_value' lvalue to temp name.
    // like 'local __fakelua_global_2__; __fakelua_global_2__ = some_value; __fakelua_set_table__(a.b, "c", __fakelua_global_2__)'
    // so we can easily always get the value of a.b.c as rvalue.
    preprocess_table_assigns(chunk);
    if (cfg.debug_mode) {
        dump_debug_file(chunk, debug_step++);
    }

    LOG_INFO("end pre_processor::compile {}", file_name);
}

void pre_processor::dump_debug_file(const syntax_tree_interface_ptr &chunk, int step) {
    const auto dumpfile = generate_tmp_filename("fakelua_gccjit_", std::format(".pre{}", step));
    std::ofstream file(dumpfile);
    DEBUG_ASSERT(file.is_open());
    file << chunk->dump(0);
    file.close();
}

void pre_processor::save_preprocess_global_init(const syntax_tree_interface_ptr &chunk) {
    // add new function __fakelua_global_init__
    const auto funcname = std::make_shared<syntax_tree_funcname>(chunk->loc());
    const auto funcnamelist = std::make_shared<syntax_tree_funcnamelist>(chunk->loc());
    funcnamelist->add_name("__fakelua_global_init__");
    funcname->set_funcnamelist(funcnamelist);
    const auto funcbody = std::make_shared<syntax_tree_funcbody>(chunk->loc());
    const auto block = std::make_shared<syntax_tree_block>(chunk->loc());
    for (auto &stmt: global_init_new_stmt_) {
        block->add_stmt(stmt);
    }
    funcbody->set_block(block);
    const auto func = std::make_shared<syntax_tree_function>(chunk->loc());
    func->set_funcname(funcname);
    func->set_funcbody(funcbody);
    const auto chunk_ptr = std::dynamic_pointer_cast<syntax_tree_block>(chunk);
    chunk_ptr->add_stmt(func);
    global_init_new_stmt_.clear();
}

void pre_processor::preprocess_const(const syntax_tree_interface_ptr &chunk) {
    // the chunk must be a block
    DEBUG_ASSERT(chunk->type() == syntax_tree_type::syntax_tree_type_block);
    // walk through the block
    for (const auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk); auto &stmt: block->stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_local_var) {
            preprocess_const_define(stmt);
        }
    }
}

void pre_processor::preprocess_const_define(const syntax_tree_interface_ptr &stmt) {
    const auto local_var = std::dynamic_pointer_cast<syntax_tree_local_var>(stmt);
    DEBUG_ASSERT(local_var->namelist()->type() == syntax_tree_type::syntax_tree_type_namelist);
    const auto keys = std::dynamic_pointer_cast<syntax_tree_namelist>(local_var->namelist());
    auto &names = keys->names();
    if (!local_var->explist()) {
        throw_error("the const define must have a value, but the value is null, it's useless", local_var);
    }
    DEBUG_ASSERT(local_var->explist()->type() == syntax_tree_type::syntax_tree_type_explist);
    const auto values = std::dynamic_pointer_cast<syntax_tree_explist>(local_var->explist());
    auto &values_exps = values->exps();

    for (size_t i = 0; i < names.size(); ++i) {
        auto name = names[i];
        if (i >= values_exps.size()) {
            throw_error("the const define not match, the value is not enough", values);
        }

        const auto assign_stmt = std::make_shared<syntax_tree_assign>(stmt->loc());
        const auto varlist = std::make_shared<syntax_tree_varlist>(stmt->loc());
        const auto var = std::make_shared<syntax_tree_var>(stmt->loc());
        var->set_type("simple");
        var->set_name(name);
        varlist->add_var(var);
        assign_stmt->set_varlist(varlist);
        const auto explist = std::make_shared<syntax_tree_explist>(stmt->loc());
        explist->add_exp(values_exps[i]);
        assign_stmt->set_explist(explist);

        // now replace the value to nil
        const auto nil_exp = std::make_shared<syntax_tree_exp>(stmt->loc());
        nil_exp->set_type("nil");
        values_exps[i] = nil_exp;

        global_init_new_stmt_.push_back(assign_stmt);
    }
}

void pre_processor::preprocess_functions_name(const syntax_tree_interface_ptr &chunk) {
    // the chunk must be a block
    DEBUG_ASSERT(chunk->type() == syntax_tree_type::syntax_tree_type_block);
    // walk through the block
    for (const auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk); auto &stmt: block->stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_function) {
            auto func = std::dynamic_pointer_cast<syntax_tree_function>(stmt);
            preprocess_function_name(func);
        }
    }
}

void pre_processor::preprocess_function_name(const syntax_tree_interface_ptr &func) {
    DEBUG_ASSERT(func->type() == syntax_tree_type::syntax_tree_type_function);
    const auto func_ptr = std::dynamic_pointer_cast<syntax_tree_function>(func);

    const auto funcname = std::dynamic_pointer_cast<syntax_tree_funcname>(func_ptr->funcname());
    const auto funcnamelistptr = funcname->funcnamelist();

    const auto newfuncnamelistptr = std::make_shared<syntax_tree_funcnamelist>(funcnamelistptr->loc());
    std::vector<std::string> newnamelist;

    DEBUG_ASSERT(funcnamelistptr->type() == syntax_tree_type::syntax_tree_type_funcnamelist);
    const auto funcnamelist = std::dynamic_pointer_cast<syntax_tree_funcnamelist>(funcnamelistptr);
    const auto &names = funcnamelist->funcnames();
    newnamelist.insert(newnamelist.end(), names.begin(), names.end());

    if (!funcname->colon_name().empty()) {
        newnamelist.push_back(funcname->colon_name());
        // insert self in the front of params
        const auto funcbody = func_ptr->funcbody();
        DEBUG_ASSERT(funcbody->type() == syntax_tree_type::syntax_tree_type_funcbody);
        const auto funcbody_ptr = std::dynamic_pointer_cast<syntax_tree_funcbody>(funcbody);
        auto parlist = funcbody_ptr->parlist();
        const auto new_parlist = std::make_shared<syntax_tree_parlist>(funcbody_ptr->loc());
        const auto namelist = std::make_shared<syntax_tree_namelist>(funcbody_ptr->loc());
        namelist->add_name("self");
        if (parlist) {
            for (const auto old_namelist = std::dynamic_pointer_cast<syntax_tree_parlist>(parlist)->namelist();
                 auto &name: std::dynamic_pointer_cast<syntax_tree_namelist>(old_namelist)->names()) {
                namelist->add_name(name);
            }
        }
        std::dynamic_pointer_cast<syntax_tree_parlist>(new_parlist)->set_namelist(namelist);
        parlist = new_parlist;
        funcbody_ptr->set_parlist(parlist);

        funcname->set_colon_name("");
    }

    DEBUG_ASSERT(!newnamelist.empty());

    if (newnamelist.size() == 1) {
        // global function
        newfuncnamelistptr->add_name(newnamelist[0]);
        funcname->set_funcnamelist(newfuncnamelistptr);
    } else {
        // member function
        // xxx.yyy(), we need alloc special function name, and set the name to xxx.yyy
        const auto name = std::dynamic_pointer_cast<state>(sp_)->get_vm().alloc_global_name();
        newfuncnamelistptr->add_name(name);
        funcname->set_funcnamelist(newfuncnamelistptr);

        // set the name to xxx.yyy.zzz = name in preprocess_trunk_new_stmt_
        const auto assign_stmt = std::make_shared<syntax_tree_assign>(funcname->loc());
        const auto varlist = std::make_shared<syntax_tree_varlist>(funcname->loc());

        const auto prefixexp = std::make_shared<syntax_tree_prefixexp>(funcname->loc());
        const auto first_var = std::make_shared<syntax_tree_var>(funcname->loc());
        first_var->set_type("simple");
        first_var->set_name(newnamelist[0]);
        prefixexp->set_type("var");
        prefixexp->set_value(first_var);

        syntax_tree_interface_ptr last_prefixexp = prefixexp;
        syntax_tree_interface_ptr last_var = first_var;
        for (size_t i = 1; i < newnamelist.size(); ++i) {
            const auto var = std::make_shared<syntax_tree_var>(funcname->loc());
            var->set_type("dot");
            var->set_name(newnamelist[i]);
            var->set_prefixexp(last_prefixexp);

            const auto cur_prefixexp = std::make_shared<syntax_tree_prefixexp>(funcname->loc());
            cur_prefixexp->set_type("var");
            cur_prefixexp->set_value(var);

            last_var = var;
            last_prefixexp = cur_prefixexp;
        }

        varlist->add_var(last_var);
        assign_stmt->set_varlist(varlist);

        const auto explist = std::make_shared<syntax_tree_explist>(funcname->loc());
        const auto exp = std::make_shared<syntax_tree_exp>(funcname->loc());
        exp->set_type("string");
        exp->set_value(name);
        explist->add_exp(exp);

        assign_stmt->set_explist(explist);

        global_init_new_stmt_.push_back(assign_stmt);
    }
}

[[noreturn]] void pre_processor::throw_error(const std::string &msg, const syntax_tree_interface_ptr &ptr) {
    throw_fakelua_exception(std::format("{} at {}", msg, location_str(ptr)));
}

std::string pre_processor::location_str(const syntax_tree_interface_ptr &ptr) {
    return std::format("{}:{}:{}", file_name_, ptr->loc().begin.line, ptr->loc().begin.column);
}

void pre_processor::preprocess_table_assigns(const syntax_tree_interface_ptr &chunk) {
    // the chunk must be a block
    DEBUG_ASSERT(chunk->type() == syntax_tree_type::syntax_tree_type_block);
    // walk through the block
    for (const auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk); auto &stmt: block->stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_function) {
            const auto func = std::dynamic_pointer_cast<syntax_tree_function>(stmt);
            preprocess_table_assign(func->funcbody());
        } else if (stmt->type() == syntax_tree_type::syntax_tree_type_local_function) {
            const auto func = std::dynamic_pointer_cast<syntax_tree_local_function>(stmt);
            preprocess_table_assign(func->funcbody());
        }
    }
}

void pre_processor::preprocess_table_assign(const syntax_tree_interface_ptr &funcbody) {
    DEBUG_ASSERT(funcbody->type() == syntax_tree_type::syntax_tree_type_funcbody);
    const auto funcbody_ptr = std::dynamic_pointer_cast<syntax_tree_funcbody>(funcbody);

    const auto block = funcbody_ptr->block();
    DEBUG_ASSERT(block->type() == syntax_tree_type::syntax_tree_type_block);
    auto block_ptr = std::dynamic_pointer_cast<syntax_tree_block>(block);

    std::vector<syntax_tree_interface_ptr> new_stmts;
    for (const auto stmts = block_ptr->stmts(); auto &stmt: stmts) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_assign) {
            const auto assign = std::dynamic_pointer_cast<syntax_tree_assign>(stmt);
            const auto varlist = assign->varlist();
            const auto explist = assign->explist();
            DEBUG_ASSERT(varlist->type() == syntax_tree_type::syntax_tree_type_varlist);
            DEBUG_ASSERT(explist->type() == syntax_tree_type::syntax_tree_type_explist);
            const auto varlist_ptr = std::dynamic_pointer_cast<syntax_tree_varlist>(varlist);
            const auto explist_ptr = std::dynamic_pointer_cast<syntax_tree_explist>(explist);
            auto &vars = varlist_ptr->vars();

            std::vector<syntax_tree_interface_ptr> pre;
            std::vector<syntax_tree_interface_ptr> post;

            for (auto &var: vars) {
                DEBUG_ASSERT(var->type() == syntax_tree_type::syntax_tree_type_var);
                if (const auto var_ptr = std::dynamic_pointer_cast<syntax_tree_var>(var);
                    var_ptr->get_type() == "square" || var_ptr->get_type() == "dot") {
                    const auto name = std::format("__fakelua_pp_pre_{}__", pre_index_++);

                    // add new stmt 'local _pre;'
                    {
                        const auto local_var = std::make_shared<syntax_tree_local_var>(stmt->loc());
                        const auto namelist = std::make_shared<syntax_tree_namelist>(stmt->loc());
                        namelist->add_name(name);
                        local_var->set_namelist(namelist);
                        pre.push_back(local_var);
                        LOG_INFO("preprocess_table_assigns add new pre stmt {}", local_var->dump(0));
                    }

                    // set stmt '_pre = some_value;'
                    {
                        const auto new_var = std::make_shared<syntax_tree_var>(var_ptr->loc());
                        new_var->set_type("simple");
                        new_var->set_name(name);
                        var = new_var;
                        LOG_INFO("preprocess_table_assigns change new var {}", new_var->dump(0));
                    }

                    // add new stmt '__fakelua_set_table__(a.b, "c", _pre)'
                    {
                        const auto func_call = std::make_shared<syntax_tree_functioncall>(stmt->loc());
                        const auto prefixexp = std::make_shared<syntax_tree_prefixexp>(stmt->loc());
                        const auto call_var = std::make_shared<syntax_tree_var>(stmt->loc());
                        call_var->set_type("simple");
                        call_var->set_name("__fakelua_set_table__");
                        prefixexp->set_type("var");
                        prefixexp->set_value(call_var);
                        func_call->set_prefixexp(prefixexp);

                        const auto args = std::make_shared<syntax_tree_args>(stmt->loc());
                        const auto args_explist = std::make_shared<syntax_tree_explist>(stmt->loc());

                        // a.b
                        {
                            const auto args_exp = std::make_shared<syntax_tree_exp>(stmt->loc());
                            const auto args_exp_prefixexp = var_ptr->get_prefixexp();
                            args_exp->set_type("prefixexp");
                            args_exp->set_right(args_exp_prefixexp);
                            args_explist->add_exp(args_exp);
                        }

                        // "c"
                        {
                            if (var_ptr->get_type() == "square") {
                                const auto args_exp = var_ptr->get_exp();
                                args_explist->add_exp(args_exp);
                            } else {
                                const auto args_exp = std::make_shared<syntax_tree_exp>(stmt->loc());
                                args_exp->set_type("string");
                                args_exp->set_value(var_ptr->get_name());
                                args_explist->add_exp(args_exp);
                            }
                        }

                        // _pre
                        {
                            const auto args_exp = std::make_shared<syntax_tree_exp>(stmt->loc());
                            const auto args_exp_prefixexp = std::make_shared<syntax_tree_prefixexp>(stmt->loc());
                            const auto args_exp_var = std::make_shared<syntax_tree_var>(stmt->loc());
                            args_exp_var->set_type("simple");
                            args_exp_var->set_name(name);
                            args_exp_prefixexp->set_type("var");
                            args_exp_prefixexp->set_value(args_exp_var);
                            args_exp->set_type("prefixexp");
                            args_exp->set_right(args_exp_prefixexp);
                            args_explist->add_exp(args_exp);
                        }

                        args->set_explist(args_explist);
                        args->set_type("explist");
                        func_call->set_args(args);

                        post.push_back(func_call);
                        LOG_INFO("preprocess_table_assigns add new post stmt {}", func_call->dump(0));
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
    block_ptr->set_stmts(new_stmts);
}

void pre_processor::preprocess_extracts_literal_constants(const syntax_tree_interface_ptr &chunk) {
    // the chunk must be a block
    DEBUG_ASSERT(chunk->type() == syntax_tree_type::syntax_tree_type_block);

    // walk tree and find all literal
    std::map<std::string, std::string> integer_map;
    std::map<std::string, std::string> float_map;
    std::map<std::string, std::string> string_map;

    auto walk_func = [&](const syntax_tree_interface_ptr &ptr) {
        if (ptr->type() == syntax_tree_type::syntax_tree_type_exp) {
            const auto exp = std::dynamic_pointer_cast<syntax_tree_exp>(ptr);
            const auto exp_type = exp->exp_type();
            const auto value = exp->exp_value();
            if (exp_type == "number") {
                if (is_integer(value)) {
                    if (!integer_map.contains(value)) {
                        integer_map[value] = std::format("__fakelua_pp_pre_{}__", pre_index_++);
                    }
                } else {
                    if (!float_map.contains(value)) {
                        float_map[value] = std::format("__fakelua_pp_pre_{}__", pre_index_++);
                    }
                }
            } else if (exp_type == "string") {
                if (!string_map.contains(value)) {
                    string_map[value] = std::format("__fakelua_pp_pre_{}__", pre_index_++);
                }
            }
        } else if (ptr->type() == syntax_tree_type::syntax_tree_type_var) {
            // a.b.c = 1, the "b" and "c" is common string
            // change to a[__global_string_1__][__global_string_2__] = 1
            if (const auto v_ptr = std::dynamic_pointer_cast<syntax_tree_var>(ptr); v_ptr->get_type() == "dot") {
                if (const auto name = v_ptr->get_name(); !string_map.contains(name)) {
                    string_map[name] = std::format("__fakelua_pp_pre_{}__", pre_index_++);
                }
            }
        } else if (ptr->type() == syntax_tree_type::syntax_tree_type_field) {
            // { a = 1 }, the "a" is a common string
            // change to { [__global_string_1__] = 1 }
            if (const auto field_ptr = std::dynamic_pointer_cast<syntax_tree_field>(ptr); field_ptr->get_type() == "object") {
                if (const auto name = field_ptr->name(); !string_map.contains(name)) {
                    string_map[name] = std::format("__fakelua_pp_pre_{}__", pre_index_++);
                }
            }
        } else if (ptr->type() == syntax_tree_type::syntax_tree_type_functioncall) {
            // a:b(), the "b" is common string
            // change to a:__global_string_1__(), and then replace it to var in the compiler
            if (const auto functioncall_ptr = std::dynamic_pointer_cast<syntax_tree_functioncall>(ptr); !functioncall_ptr->name().empty()) {
                if (const auto name = functioncall_ptr->name(); !string_map.contains(name)) {
                    string_map[name] = std::format("__fakelua_pp_pre_{}__", pre_index_++);
                }
            }
        }
    };

    // walk through the block function
    const auto block = std::dynamic_pointer_cast<syntax_tree_block>(chunk);
    for (auto &stmt: block->stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_function) {
            const auto func = std::dynamic_pointer_cast<syntax_tree_function>(stmt);
            walk_syntax_tree(func, walk_func);
        }
    }

    auto cur_stmts = block->stmts();

    // now we have all literal constants, we need to add them to chunk front
    for (const auto &[value, name]: integer_map) {
        const auto local_var = std::make_shared<syntax_tree_local_var>(chunk->loc());
        const auto namelist = std::make_shared<syntax_tree_namelist>(chunk->loc());
        namelist->add_name(name);
        local_var->set_namelist(namelist);
        const auto explist = std::make_shared<syntax_tree_explist>(chunk->loc());
        const auto exp = std::make_shared<syntax_tree_exp>(chunk->loc());
        exp->set_type("number");
        exp->set_value(value);
        explist->add_exp(exp);
        local_var->set_explist(explist);
        cur_stmts.insert(cur_stmts.begin(), local_var);
    }
    for (const auto &[value, name]: float_map) {
        const auto local_var = std::make_shared<syntax_tree_local_var>(chunk->loc());
        const auto namelist = std::make_shared<syntax_tree_namelist>(chunk->loc());
        namelist->add_name(name);
        local_var->set_namelist(namelist);
        const auto explist = std::make_shared<syntax_tree_explist>(chunk->loc());
        const auto exp = std::make_shared<syntax_tree_exp>(chunk->loc());
        exp->set_type("number");
        exp->set_value(value);
        explist->add_exp(exp);
        local_var->set_explist(explist);
        cur_stmts.insert(cur_stmts.begin(), local_var);
    }
    for (const auto &[value, name]: string_map) {
        const auto local_var = std::make_shared<syntax_tree_local_var>(chunk->loc());
        const auto namelist = std::make_shared<syntax_tree_namelist>(chunk->loc());
        namelist->add_name(name);
        local_var->set_namelist(namelist);
        const auto explist = std::make_shared<syntax_tree_explist>(chunk->loc());
        const auto exp = std::make_shared<syntax_tree_exp>(chunk->loc());
        exp->set_type("string");
        exp->set_value(value);
        explist->add_exp(exp);
        local_var->set_explist(explist);
        cur_stmts.insert(cur_stmts.begin(), local_var);
    }

    block->set_stmts(cur_stmts);

    // now replace all the literal to const name
    const auto replace_func = [&](const syntax_tree_interface_ptr &ptr) {
        if (ptr->type() == syntax_tree_type::syntax_tree_type_exp) {
            const auto exp = std::dynamic_pointer_cast<syntax_tree_exp>(ptr);
            const auto exp_type = exp->exp_type();
            const auto value = exp->exp_value();
            if (exp_type == "number") {
                if (is_integer(value)) {
                    DEBUG_ASSERT(integer_map.contains(value));
                    const auto new_var_name = integer_map[value];
                    const auto prefix = std::make_shared<syntax_tree_prefixexp>(exp->loc());
                    const auto var = std::make_shared<syntax_tree_var>(exp->loc());
                    var->set_type("simple");
                    var->set_name(new_var_name);
                    prefix->set_type("var");
                    prefix->set_value(var);
                    exp->set_type("prefixexp");
                    exp->set_right(prefix);
                } else {
                    DEBUG_ASSERT(float_map.contains(value));
                    const auto new_var_name = float_map[value];
                    const auto prefix = std::make_shared<syntax_tree_prefixexp>(exp->loc());
                    const auto var = std::make_shared<syntax_tree_var>(exp->loc());
                    var->set_type("simple");
                    var->set_name(new_var_name);
                    prefix->set_type("var");
                    prefix->set_value(var);
                    exp->set_type("prefixexp");
                    exp->set_right(prefix);
                }
            } else if (exp_type == "string") {
                DEBUG_ASSERT(string_map.contains(value));
                const auto new_var_name = string_map[value];
                const auto prefix = std::make_shared<syntax_tree_prefixexp>(exp->loc());
                const auto var = std::make_shared<syntax_tree_var>(exp->loc());
                var->set_type("simple");
                var->set_name(new_var_name);
                prefix->set_type("var");
                prefix->set_value(var);
                exp->set_type("prefixexp");
                exp->set_right(prefix);
            }
        } else if (ptr->type() == syntax_tree_type::syntax_tree_type_var) {
            if (const auto v_ptr = std::dynamic_pointer_cast<syntax_tree_var>(ptr); v_ptr->get_type() == "dot") {
                const auto name = v_ptr->get_name();
                DEBUG_ASSERT(string_map.contains(name));
                const auto new_var_name = string_map[name];
                const auto exp = std::make_shared<syntax_tree_exp>(ptr->loc());
                const auto prefix = std::make_shared<syntax_tree_prefixexp>(ptr->loc());
                const auto var = std::make_shared<syntax_tree_var>(ptr->loc());
                var->set_type("simple");
                var->set_name(new_var_name);
                prefix->set_type("var");
                prefix->set_value(var);
                exp->set_type("prefixexp");
                exp->set_right(prefix);
                v_ptr->set_type("square");
                v_ptr->set_exp(exp);
            }
        } else if (ptr->type() == syntax_tree_type::syntax_tree_type_field) {
            if (const auto field_ptr = std::dynamic_pointer_cast<syntax_tree_field>(ptr); field_ptr->get_type() == "object") {
                const auto name = field_ptr->name();
                DEBUG_ASSERT(string_map.contains(name));
                const auto new_var_name = string_map[name];
                const auto exp = std::make_shared<syntax_tree_exp>(ptr->loc());
                const auto prefix = std::make_shared<syntax_tree_prefixexp>(ptr->loc());
                const auto var = std::make_shared<syntax_tree_var>(ptr->loc());
                var->set_type("simple");
                var->set_name(new_var_name);
                prefix->set_type("var");
                prefix->set_value(var);
                exp->set_type("prefixexp");
                exp->set_right(prefix);
                field_ptr->set_type("array");
                field_ptr->set_key(exp);
            }
        } else if (ptr->type() == syntax_tree_type::syntax_tree_type_functioncall) {
            if (const auto functioncall_ptr = std::dynamic_pointer_cast<syntax_tree_functioncall>(ptr); !functioncall_ptr->name().empty()) {
                const auto name = functioncall_ptr->name();
                DEBUG_ASSERT(string_map.contains(name));
                const auto new_var_name = string_map[name];
                functioncall_ptr->set_name(new_var_name);
            }
        }
    };

    // walk through the block function
    for (const auto &stmt: block->stmts()) {
        if (stmt->type() == syntax_tree_type::syntax_tree_type_function) {
            const auto func = std::dynamic_pointer_cast<syntax_tree_function>(stmt);
            walk_syntax_tree(func, replace_func);
        }
    }
}

}// namespace fakelua
