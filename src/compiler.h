#pragma once

#include "types.h"
#include "binary.h"
#include "codegen.h"
#include "syntree.h"

class fuck;
class myflexer;
class compiler
{
public:
    compiler(fuck * fk) : m_fk(fk), m_binary(fk)
    {
        clear();
    }
    ~compiler() {}

    void clear();
    
    bool compile(myflexer * mf);

private:
    bool compile_func(func_desc_node * funcnode);
    bool compile_block(codegen & cg, block_node * blocknode, int stack_level);
    bool compile_node(codegen & cg, syntree_node * node, int stack_level);
    bool compile_while_stmt(codegen & cg, while_stmt * ws, int stack_level);
    bool compile_if_stmt(codegen & cg, if_stmt * is, int stack_level);
    bool compile_return_stmt(codegen & cg, return_stmt * rs, int stack_level);
    bool compile_assign_stmt(codegen & cg, assign_stmt * as, int stack_level);
    bool compile_break_stmt(codegen & cg, break_stmt * bs, int stack_level);
    bool compile_cmp_stmt(codegen & cg, cmp_stmt * cs, int stack_level);
    bool compile_explicit_value(codegen & cg, explicit_value_node * ev, int stack_level);
    bool compile_variable_node(codegen & cg, variable_node * vn, int stack_level);
    bool compile_var_node(codegen & cg, var_node * vn, int stack_level);
    bool compile_function_call_node(codegen & cg, function_call_node * fn, int stack_level);
    bool compile_math_expr_node(codegen & cg, math_expr_node * mn, int stack_level);
    bool compile_identifier_node(codegen & cg, identifier_node * in, int stack_level);
    
private:
    bool add_stack_identifier();
    void push_stack_identifiers();
    void pop_stack_identifiers();
    
private:
    fuck * m_fk;
    binary m_binary;
};

