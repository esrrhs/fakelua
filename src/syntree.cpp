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
    SYN_NODE_DEF(est_for_stmt)
    SYN_NODE_DEF(est_for_loop_stmt)
	SYN_NODE_DEF(est_return_value_list)
	SYN_NODE_DEF(est_multi_assign_stmt)
	SYN_NODE_DEF(est_var_list)
	SYN_NODE_DEF(est_container_get)
	SYN_NODE_DEF(est_struct_memlist)
	SYN_NODE_DEF(est_struct_pointer)
	SYN_NODE_DEF(est_continue)
	SYN_NODE_DEF(est_sleep)
	SYN_NODE_DEF(est_yield)
	SYN_NODE_DEF(est_switch_stmt)
	SYN_NODE_DEF(est_switch_caselist)
	SYN_NODE_DEF(est_switch_case_node)
	SYN_NODE_DEF(est_elseif_stmt)
	SYN_NODE_DEF(est_elseif_stmt_list)
	SYN_NODE_DEF(est_constmaplist)
	SYN_NODE_DEF(est_constmapvalue)
	SYN_NODE_DEF(est_constarraylist)
	
#undef SYN_NODE_DEF
	}
	assert(0);
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
	if (elseifs)
	{
		ret += elseifs->dump(indent + 1);
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

String for_loop_stmt::dump(int indent)
{
    String ret;
    ret += gentab(indent);
    ret += "[for]:\n";
    ret += gentab(indent + 1);
    ret += "[iter]:\n";
    if (iter)
    {
        ret += iter->dump(indent + 2);
    }
    ret += gentab(indent + 1);
    ret += "[begin]:\n";
    if (begin)
    {
        ret += begin->dump(indent + 2);
    }
    ret += gentab(indent + 1);
    ret += "[end]:\n";
    if (end)
    {
        ret += end->dump(indent + 2);
    }
    ret += gentab(indent + 1);
    ret += "[step]:\n";
    if (step)
    {
        ret += step->dump(indent + 2);
    }
    ret += gentab(indent + 1);
    ret += "[block]:\n";
    if (block)
    {
        ret += block->dump(indent + 2);
    }
    return ret;
}

void func_desc_arglist_node::add_arg(syntree_node * p)
{
	assert(p->gettype() == est_identifier);		
	identifier_node * pi = dynamic_cast<identifier_node*>(p);
	arglist.push_back(pi->str);
	FKLOG("%p add arg %s", this, pi->str.c_str());
}

String return_value_list_node::dump(int indent)
{
	String ret;
	ret += gentab(indent);
	ret += "[return_value_list]:\n";
	for (int i = 0; i < (int)returnlist.size(); i++)
	{
		ret += returnlist[i]->dump(indent + 1);
	}
	return ret;
}

void return_value_list_node::add_arg(syntree_node * p)
{
	returnlist.push_back(p);
}

String var_list_node::dump(int indent)
{
	String ret;
	ret += gentab(indent);
	ret += "[var_list]:\n";
	for (int i = 0; i < (int)varlist.size(); i++)
	{
		ret += varlist[i]->dump(indent + 1);
	}
	return ret;
}

void var_list_node::add_arg(syntree_node * p)
{
	varlist.push_back(p);
}

String multi_assign_stmt::dump(int indent)
{
	String ret;
	ret += gentab(indent);
	ret += "[multi_assign]:\n";

	ret += gentab(indent + 1);
	ret += "[var]:\n";
	ret += varlist->dump(indent + 2);

	ret += gentab(indent + 1);
	ret += "[value]:\n";
	ret += value->dump(indent + 2);
	return ret;

}

String container_get_node::dump(int indent)
{
	String ret;
	ret += gentab(indent);
	ret += "[container_get]:\n";

	ret += gentab(indent + 1);
	ret += "[container]:";
	ret += container;
	ret += "\n";

	ret += gentab(indent + 1);
	ret += "[key]:\n";
	ret += key->dump(indent + 2);
	return ret;

}

String struct_pointer_node::dump(int indent)
{
	String ret;
	ret += gentab(indent);
	ret += "[struct_pointer_node]:";
	ret += str;
	ret += "\n";
	return ret;
}

String elseif_stmt::dump(int indent)
{
	String ret;
	ret += gentab(indent);
	ret += "[elseif_stmt]:\n";
	ret += cmp->dump(indent + 1);
	if (block)
	{
		ret += block->dump(indent + 1);
	}
	return ret;
}

