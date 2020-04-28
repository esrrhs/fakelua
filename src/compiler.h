#pragma once

#include "types.h"
#include "codegen.h"
#include "syntree.h"

struct fake;

class binary;

class myflexer;

class compiler {
public:
    compiler(fake *fk, myflexer *mf) : m_fk(fk), m_mf(mf) {
    }

    ~compiler() {
        clear();
    }

    void clear();

    bool compile();

private:
    bool compile_const_head();

    bool compile_body();

    bool compile_func(func_desc_node *funcnode);

    bool compile_block(codegen &cg, block_node *blocknode);

    bool compile_node(codegen &cg, syntree_node *node);

    bool compile_while_stmt(codegen &cg, while_stmt *ws);

    bool compile_for_stmt(codegen &cg, for_stmt *fs);

    bool compile_for_loop_stmt(codegen &cg, for_loop_stmt *fs);

    bool compile_if_stmt(codegen &cg, if_stmt *is);

    bool compile_return_stmt(codegen &cg, return_stmt *rs);

    bool compile_assign_stmt(codegen &cg, assign_stmt *as);

    bool compile_multi_assign_stmt(codegen &cg, multi_assign_stmt *as);

    bool compile_math_assign_stmt(codegen &cg, math_assign_stmt *ms);

    bool compile_break_stmt(codegen &cg, break_stmt *bs);

    bool compile_continue_stmt(codegen &cg, continue_stmt *cs);

    bool compile_cmp_stmt(codegen &cg, cmp_stmt *cs);

    bool compile_explicit_value(codegen &cg, explicit_value_node *ev);

    bool compile_variable_node(codegen &cg, variable_node *vn);

    bool compile_var_node(codegen &cg, var_node *vn);

    bool compile_function_call_node(codegen &cg, function_call_node *fn);

    bool compile_math_expr_node(codegen &cg, math_expr_node *mn);

    bool compile_return_value_list(codegen &cg, return_value_list_node *rn);

    bool compile_container_get(codegen &cg, container_get_node *cn);

    bool compile_struct_pointer(codegen &cg, struct_pointer_node *sn);

    bool compile_sleep_stmt(codegen &cg, sleep_stmt *ss);

    bool compile_yield_stmt(codegen &cg, yield_stmt *ys);

    bool compile_switch_stmt(codegen &cg, switch_stmt *ss);

private:
    void compile_seterror(syntree_node *node, fake *fk, efkerror err, const char *fmt, ...);

    bool compile_explicit_value_node_to_variant(explicit_value_node *ev, variant &v);

private:
    fake *m_fk;
    myflexer *m_mf;
    command m_cur_addr;
    command m_cur_addrs[MAX_FAKE_RETURN_NUM];
    typedef std::vector<int> beak_pos_list;
    typedef std::vector<beak_pos_list> loop_break_pos_stack;
    loop_break_pos_stack m_loop_break_pos_stack;
    typedef std::vector<int> continue_pos_stack;
    continue_pos_stack m_loop_continue_pos_stack;
    typedef std::vector<int> continue_end_pos_list;
    typedef std::vector<continue_end_pos_list> continue_end_pos_stack;
    continue_end_pos_stack m_continue_end_pos_stack;
    int m_func_ret_num;
    String m_cur_compile_func;
    bool m_new_var;
    int m_cmp_deps;
    bool m_cmp_jne;
};

