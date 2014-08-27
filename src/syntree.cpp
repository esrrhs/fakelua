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

String while_stmt::dump(int indent)
{
    String ret;
    ret += gentab(indent);
    ret += "[while]:\n";
    ret += cmp->dump(indent + 1);
    if (block)
    {
        ret += block->dump(indent + 1);
    }
    else
    {
        ret += "nil\n";
    }
    return ret;
}

String else_stmt::dump(int indent)
{
    String ret;
    ret += gentab(indent);
    ret += "[else]:\n";
    if (block)
    {
        ret += block->dump(indent + 1);
    }
    else
    {
        ret += "nil\n";
    }
    return ret;
}

String if_stmt::dump(int indent)
{
    String ret;
    ret += gentab(indent);
    ret += "[if]:\n";
    ret += cmp->dump(indent + 1);
    if (block)
    {
        ret += block->dump(indent + 1);
    }
    ret += elses->dump(indent);
    return ret;
}
