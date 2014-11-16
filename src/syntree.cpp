#include "syntree.h"
#include "fake.h"

const char * get_syntree_node_name(esyntreetype type)
{
    switch (type)
    {
#define SYN_NODE_DEF(type) \
    case type:\
        return #type;
        
    SYN_NODE_DEF(est_nil)
    SYN_NODE_DEF(est_func_desc)
    SYN_NODE_DEF(est_arglist)
    SYN_NODE_DEF(est_block)
    SYN_NODE_DEF(est_while_stmt)
    SYN_NODE_DEF(est_cmp_stmt)
    SYN_NODE_DEF(est_if_stmt)
    SYN_NODE_DEF(est_else_stmt)
    SYN_NODE_DEF(est_explicit_value)
    SYN_NODE_DEF(est_return_stmt)
    SYN_NODE_DEF(est_assign_stmt)
    SYN_NODE_DEF(est_math_assign_stmt)
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
        ret += gentab(indent + 1);
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
        ret += gentab(indent + 1);
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
    if (elses)
    {
        ret += elses->dump(indent);
    }
    return ret;
}

String for_stmt::dump(int indent)
{
	String ret;
	ret += gentab(indent);
	ret += "[for]:\n";
	ret += gentab(indent + 1);
	ret += "[beginblock]:\n";
	if (beginblock)
	{
		ret += beginblock->dump(indent + 2);
	}
	ret += gentab(indent + 1);
	ret += "[cmp]:\n";
	if (cmp)
	{
		ret += cmp->dump(indent + 2);
	}
	ret += gentab(indent + 1);
	ret += "[endblock]:\n";
	if (endblock)
	{
		ret += endblock->dump(indent + 2);
	}
	ret += gentab(indent + 1);
	ret += "[block]:\n";
	if (block)
	{
		ret += block->dump(indent + 2);
	}
	return ret;
}

void for_stmt::recycle()
{
	FKLOG("recycle for_stmt");
	if (beginblock)
	{
		beginblock->recycle();
	}
	if (cmp)
	{
		cmp->recycle();
	}
	if (endblock)
	{
		endblock->recycle();
	}
	if (block)
	{
		block->recycle();
	}
	safe_fkfree(fk, this);
}

void while_stmt::recycle()
{
    FKLOG("recycle while_stmt");
    if (cmp)
    {
        cmp->recycle();
    }
    if (block)
    {
        block->recycle();
    }
    safe_fkfree(fk, this);
}

void else_stmt::recycle()
{
    FKLOG("recycle else_stmt");
    if (block)
    {
        block->recycle();
    }
    safe_fkfree(fk, this);
}

void if_stmt::recycle()
{
    FKLOG("recycle if_stmt");
    if (cmp)
    {
        cmp->recycle();
    }
    if (block)
    {
        block->recycle();
    }
    if (elses)
    {
        elses->recycle();
    }
    safe_fkfree(fk, this);
}

void identifier_node::recycle()
{
    FKLOG("recycle identifier_node");
    fkdelete<identifier_node>(fk, this);
}

void func_desc_arglist_node::add_arg(syntree_node * p)
{
    assert(p->gettype() == est_identifier);		
    identifier_node * pi = dynamic_cast<identifier_node*>(p);
    arglist.push_back(pi->str);
    FKLOG("%p add arg %s", this, pi->str.c_str());
    fkdelete<identifier_node>(fk, pi);
}

void func_desc_arglist_node::recycle()
{
    FKLOG("recycle func_desc_arglist_node");
    fkdelete<func_desc_arglist_node>(fk, this);
}
    
void explicit_value_node::recycle()
{
    FKLOG("recycle explicit_value_node");
    fkdelete<explicit_value_node>(fk, this);
}

void return_stmt::recycle()
{
    FKLOG("recycle return_stmt");
    if (ret)
    {
        ret->recycle();
    }
    fkdelete<return_stmt>(fk, this);
}

void cmp_stmt::recycle()
{
    FKLOG("recycle cmp_stmt");
    if (left)
    {
        left->recycle();
    }
    if (right)
    {
        right->recycle();
    }
    fkdelete<cmp_stmt>(fk, this);
}

void block_node::recycle()
{
    FKLOG("recycle block_node");
    for (int i = 0; i < (int)stmtlist.size(); i++)
    {
        stmtlist[i]->recycle();
    }
    fkdelete<block_node>(fk, this);
}

void func_desc_node::recycle()
{
    FKLOG("recycle func_desc_node");
    if (arglist)
    {
        arglist->recycle();
    }
    if (block)
    {
        block->recycle();
    }
    fkdelete<func_desc_node>(fk, this);
}

void assign_stmt::recycle()
{
    FKLOG("recycle assign_stmt");
    if (var)
    {
        var->recycle();
    }
    if (value)
    {
        value->recycle();
    }
    fkdelete<assign_stmt>(fk, this);
}

void math_assign_stmt::recycle()
{
    FKLOG("recycle math_assign_stmt");
    if (var)
    {
        var->recycle();
    }
    if (value)
    {
        value->recycle();
    }
    fkdelete<math_assign_stmt>(fk, this);
}

void variable_node::recycle()
{
    FKLOG("recycle variable_node");
    fkdelete<variable_node>(fk, this);
}

void var_node::recycle()
{
    FKLOG("recycle var_node");
    fkdelete<var_node>(fk, this);
}

void function_call_arglist_node::recycle()
{
    FKLOG("recycle function_call_arglist_node");
    for (int i = 0; i < (int)arglist.size(); i++)
    {
        arglist[i]->recycle();
    }
    fkdelete<function_call_arglist_node>(fk, this);
}

void function_call_node::recycle()
{
    FKLOG("recycle function_call_node");
    if (arglist)
    {
        arglist->recycle();
    }
    fkdelete<function_call_node>(fk, this);
}

void math_expr_node::recycle()
{
    FKLOG("recycle math_expr_node");
    if (left)
    {
        left->recycle();
    }
    if (right)
    {
        right->recycle();
    }
    fkdelete<math_expr_node>(fk, this);
}

void break_stmt::recycle()
{
    FKLOG("recycle break_stmt");
    fkdelete<break_stmt>(fk, this);
}

