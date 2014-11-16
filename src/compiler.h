#pragma once

#include "types.h"
#include "codegen.h"
#include "syntree.h"

struct fake;
class myflexer;
class binary;
class compiler
{
public:
    compiler(fake * fk) : m_fk(fk)
    {
    }
    ~compiler() {}

    void clear();
    bool compile(myflexer * mf);

private:
    bool compile_func(func_desc_node * funcnode);
    bool compile_block(codegen & cg, block_node * blocknode);
    bool compile_node(codegen & cg, syntree_node * node);
    bool compile_while_stmt(codegen & cg, while_stmt * ws);
	bool compile_for_stmt(codegen & cg, for_stmt * fs);
    bool compile_if_stmt(codegen & cg, if_stmt * is);
    bool compile_return_stmt(codegen & cg, return_stmt * rs);
    bool compile_assign_stmt(codegen & cg, assign_stmt * as);
    bool compile_math_assign_stmt(codegen & cg, math_assign_stmt * ms);
    bool compile_break_stmt(codegen & cg, break_stmt * bs);
    bool compile_cmp_stmt(codegen & cg, cmp_stmt * cs);
    bool compile_explicit_value(codegen & cg, explicit_value_node * ev);
    bool compile_variable_node(codegen & cg, variable_node * vn);
    bool compile_var_node(codegen & cg, var_node * vn);
    bool compile_function_call_node(codegen & cg, function_call_node * fn);
    bool compile_math_expr_node(codegen & cg, math_expr_node * mn);
    bool compile_identifier_node(codegen & cg, identifier_node * in);
    
private:
    fake * m_fk;
    command m_cur_addr;
	typedef std::vector<int> beak_pos_list;
	typedef std::vector<beak_pos_list> loop_break_pos_stack;
	loop_break_pos_stack m_loop_break_pos_stack;
};

