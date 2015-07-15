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
	SYN_NODE_DEF(est_for_loop_stmt)
	
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
	if (elseifs)
	{
		elseifs->recycle();
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
	if (returnlist)
	{
		returnlist->recycle();
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

void return_value_list_node::recycle()
{
	FKLOG("recycle return_value_list_node");
	for (int i = 0; i < (int)returnlist.size(); i++)
	{
		returnlist[i]->recycle();
	}
	fkdelete<return_value_list_node>(fk, this);
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

void var_list_node::recycle()
{
	FKLOG("recycle var_list_node");
	for (int i = 0; i < (int)varlist.size(); i++)
	{
		varlist[i]->recycle();
	}
	fkdelete<var_list_node>(fk, this);
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

void container_get_node::recycle()
{
	FKLOG("recycle container_get_node");
	key->recycle();
	fkdelete<container_get_node>(fk, this);
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

void multi_assign_stmt::recycle()
{
	FKLOG("recycle var_list_node");
	varlist->recycle();
	value->recycle();
	fkdelete<multi_assign_stmt>(fk, this);
}

void struct_desc_memlist_node::recycle()
{
	FKLOG("recycle struct_desc_memlist_node");
	fkdelete<struct_desc_memlist_node>(fk, this);
}

void struct_pointer_node::recycle()
{
	FKLOG("recycle struct_pointer_node");
	fkdelete<struct_pointer_node>(fk, this);
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

void continue_stmt::recycle()
{
	FKLOG("recycle continue_stmt");
	fkdelete<continue_stmt>(fk, this);
}

void sleep_stmt::recycle()
{
	FKLOG("recycle sleep_stmt");
	time->recycle();
	fkdelete<sleep_stmt>(fk, this);
}

void yield_stmt::recycle()
{
	FKLOG("recycle yield_stmt");
	time->recycle();
	fkdelete<yield_stmt>(fk, this);
}

void switch_case_node::recycle()
{
	FKLOG("recycle switch_case_node");
	cmp->recycle();
	if (block)
	{
		block->recycle();
	}
	fkdelete<switch_case_node>(fk, this);
}

void switch_caselist_node::recycle()
{
	FKLOG("recycle switch_caselist_node");
	for (int i = 0; i < (int)list.size(); i++)
	{
		list[i]->recycle();
	}
	fkdelete<switch_caselist_node>(fk, this);
}

void switch_stmt::recycle()
{
	FKLOG("recycle switch_stmt");
	cmp->recycle();
	caselist->recycle();
	if (def)
	{
		def->recycle();
	}
	fkdelete<switch_stmt>(fk, this);
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

void elseif_stmt::recycle()
{
	FKLOG("recycle elseif_stmt");
	cmp->recycle();
	if (block)
	{
		block->recycle();
	}
	fkdelete<elseif_stmt>(fk, this);
}

void elseif_stmt_list::recycle()
{
	FKLOG("recycle elseif_stmt_list");
	for (int i = 0; i < (int)stmtlist.size(); i++)
	{
		stmtlist[i]->recycle();
	}
	fkdelete<elseif_stmt_list>(fk, this);
}

void for_loop_stmt::recycle()
{
	FKLOG("recycle for_loop_stmt");
	var->recycle();
	begin->recycle();
	end->recycle();
	add->recycle();
	if (block)
	{
		block->recycle();
	}
	fkdelete<for_loop_stmt>(fk, this);
}

