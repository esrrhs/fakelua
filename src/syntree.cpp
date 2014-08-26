#include "syntree.h"

const char * get_syntree_node_name(esyntreetype type)
{
    switch (type)
    {
#define SYN_NODE_DEF(type) \
    case type:\
        return #type;
        
    SYN_NODE_DEF(est_func_nil)
    SYN_NODE_DEF(est_func_desc)
    SYN_NODE_DEF(est_bin)
    SYN_NODE_DEF(est_arglist)
    SYN_NODE_DEF(est_block)
    SYN_NODE_DEF(est_while_stmt)
    SYN_NODE_DEF(est_cmp_stmt)
    SYN_NODE_DEF(est_if_stmt)
    SYN_NODE_DEF(est_else_stmt)
    SYN_NODE_DEF(est_explicit_value)
    SYN_NODE_DEF(est_return_stmt)
    SYN_NODE_DEF(est_assign_stmt)
    SYN_NODE_DEF(est_variable)
    SYN_NODE_DEF(est_var)
    SYN_NODE_DEF(est_function_call)
    SYN_NODE_DEF(est_call_arglist)
    SYN_NODE_DEF(est_math_expr)
    SYN_NODE_DEF(est_break)
    SYN_NODE_DEF(est_identifier)
    
#undef SYN_NODE_DEF
    }
    return "unknow";
}

