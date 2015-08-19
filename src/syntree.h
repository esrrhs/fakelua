#pragma once

#include <string>
#include <list>
#include <vector>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <typeinfo>
#include <stdio.h>
#include "types.h"

struct fake;

enum esyntreetype
{
	est_nil,
	est_func_desc,
	est_arglist,
	est_block,
	est_while_stmt,
	est_cmp_stmt,
	est_if_stmt,
	est_else_stmt,
	est_for_stmt,
	est_explicit_value,
	est_return_stmt,
	est_return_value_list,
	est_assign_stmt,
	est_math_assign_stmt,
	est_variable,
	est_var,
	est_function_call,
	est_call_arglist,
	est_math_expr,
	est_break,
	est_identifier,
	est_multi_assign_stmt,
	est_var_list,
	est_container_get,
	est_struct_memlist,
	est_struct_pointer,
	est_continue,
	est_sleep,
	est_yield,
	est_switch_stmt,
	est_switch_caselist,
	est_switch_case_node,
	est_elseif_stmt,
	est_elseif_stmt_list,
	est_for_loop_stmt,
};

const char * get_syntree_node_name(esyntreetype type);

struct syntree_node
{
	syntree_node() : fk(0), lno(0) {}
	virtual ~syntree_node() {}

	virtual esyntreetype gettype()
	{
		return est_nil;
	}
	
	virtual const char * gettypename()
	{
		return get_syntree_node_name(gettype());
	}
	
	virtual String dump(int indent)
	{
		return gentab(indent) + "nil\n";
	}

	virtual void recycle() = 0;
	
	String gentab(int indent)
	{
		String ret;
		ret += "LINE:";
		ret += fkitoa(lno);
		ret += " ";
		for (int i = 0; i < indent; i++)
		{
			ret += "\t";
		}
		return ret;
	}

	int lineno() const
	{
		return lno;
	}

	fake * fk;
	int lno;
};

struct identifier_node : public syntree_node
{
	identifier_node() {}
	virtual ~identifier_node() {}
	
	virtual esyntreetype gettype()
	{
		return est_identifier;
	}
	
	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[identifier]:";
		ret += str;
		ret += "\n";
		return ret;
	}

	virtual void recycle();
	
	String str;
};

typedef std::vector<String> func_desc_arglist;

struct func_desc_arglist_node : public syntree_node
{
	func_desc_arglist_node() {}
	virtual ~func_desc_arglist_node() {}

	virtual esyntreetype gettype()
	{
		return est_arglist;
	}
	
	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[func_desc_arglist]:";
		for (int i = 0; i < (int)arglist.size(); i++)
		{
			ret += arglist[i];
			ret += ",";
		}
		ret += "\n";
		return ret;
	}

	virtual void recycle();
	
	void add_arg(syntree_node * p);

	func_desc_arglist arglist;
};

struct explicit_value_node : public syntree_node
{
	explicit_value_node() {}
	virtual ~explicit_value_node() {}
	
	enum explicit_value_type
	{
		EVT_TRUE,
		EVT_FALSE,
		EVT_NUM,
		EVT_STR,
		EVT_FLOAT,
		EVT_UUID,
	};

	virtual esyntreetype gettype()
	{
		return est_explicit_value;
	}

	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[explicit_value]:";
		ret += str;
		ret += "\n";
		return ret;
	}
	
	virtual void recycle();
	
	explicit_value_type getvaluetype() const
	{
		return type;
	}

	String str;
	explicit_value_type type;
};

typedef std::map<String, explicit_value_node*> explicit_value_map;

typedef std::vector<syntree_node *> return_value_list;

struct return_value_list_node : public syntree_node
{
	return_value_list_node() {}
	virtual ~return_value_list_node() {}

	virtual esyntreetype gettype()
	{
		return est_return_value_list;
	}

	virtual String dump(int indent);

	virtual void recycle();

	void add_arg(syntree_node * p);

	return_value_list returnlist;
};

struct return_stmt : public syntree_node
{
	return_stmt() {}
	virtual ~return_stmt() {}

	virtual esyntreetype gettype()
	{
		return est_return_stmt;
	}

	virtual String dump(int indent)
	{
		String sret;
		sret += gentab(indent);
		sret += "[return]:";
		sret += "\n";
		if (returnlist)
		{
			sret += returnlist->dump(indent + 1);
		}
		return sret;
	}
	
	virtual void recycle();
	
	return_value_list_node * returnlist;
};

struct cmp_stmt : public syntree_node
{
	cmp_stmt() {}
	virtual ~cmp_stmt() {}
	
	virtual esyntreetype gettype()
	{
		return est_cmp_stmt;
	}

	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[cmp]:";
		ret += cmp;
		ret += "\n";
		if (left)
		{
			ret += gentab(indent + 1);
			ret += "[left]:\n";
			ret += left->dump(indent + 2);
		}
		if (right)
		{
			ret += gentab(indent + 1);
			ret += "[right]:\n";
			ret += right->dump(indent + 2);
		}
		return ret;
	}
	
	virtual void recycle();
	
	String cmp;
	syntree_node * left;
	syntree_node * right;
};

struct block_node;
struct while_stmt : public syntree_node
{
	while_stmt() {}
	virtual ~while_stmt() {}
	
	virtual esyntreetype gettype()
	{
		return est_while_stmt;
	}

	virtual String dump(int indent);
	
	virtual void recycle();
	
	cmp_stmt * cmp;
	block_node * block;
};

struct else_stmt : public syntree_node
{
	else_stmt() {}
	virtual ~else_stmt() {}
	
	virtual esyntreetype gettype()
	{
		return est_else_stmt;
	}

	virtual String dump(int indent);
	
	virtual void recycle();
	
	block_node * block;
};

struct elseif_stmt : public syntree_node
{
	elseif_stmt() {}
	virtual ~elseif_stmt() {}
	
	virtual esyntreetype gettype()
	{
		return est_elseif_stmt;
	}

	virtual String dump(int indent);
	
	virtual void recycle();
	
	cmp_stmt * cmp;
	syntree_node * block;
};

typedef std::vector<syntree_node *> stmt_node_list;

struct elseif_stmt_list : public syntree_node
{
	elseif_stmt_list() {}
	virtual ~elseif_stmt_list() {}
	
	virtual esyntreetype gettype()
	{
		return est_elseif_stmt_list;
	}

	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[elseif_stmt_list]:\n";
		for (int i = 0; i < (int)stmtlist.size(); i++)
		{
			ret += gentab(indent + 1);
			ret += "[stmt";
			ret += fkitoa(i);
			ret += "]:\n";
			ret += stmtlist[i]->dump(indent + 2);
		}
		return ret;
	}
	
	virtual void recycle();

	void add_stmt(syntree_node * stmt)
	{
		FKLOG("elseif_stmt_list add stmt %s", stmt->gettypename());
		stmtlist.push_back(stmt);
	}

	stmt_node_list stmtlist;
};

struct if_stmt : public syntree_node
{
	if_stmt() {}
	virtual ~if_stmt() {}
	
	virtual esyntreetype gettype()
	{
		return est_if_stmt;
	}

	virtual String dump(int indent);
	
	virtual void recycle();
	
	cmp_stmt * cmp;
	block_node * block;
	elseif_stmt_list * elseifs;
	else_stmt * elses;
};

struct for_stmt : public syntree_node
{
	for_stmt() {}
	virtual ~for_stmt() {}

	virtual esyntreetype gettype()
	{
		return est_for_stmt;
	}

	virtual String dump(int indent);

	virtual void recycle();

	block_node * beginblock;
	cmp_stmt * cmp;
	block_node * endblock;
	block_node * block;
};

struct block_node : public syntree_node
{
	block_node() {}
	virtual ~block_node() {}

	virtual esyntreetype gettype()
	{
		return est_block;
	}

	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[block]:\n";
		for (int i = 0; i < (int)stmtlist.size(); i++)
		{
			ret += gentab(indent + 1);
			ret += "[stmt";
			ret += fkitoa(i);
			ret += "]:\n";
			ret += stmtlist[i]->dump(indent + 2);
		}
		return ret;
	}
	
	virtual void recycle();
	
	void add_stmt(syntree_node * stmt)
	{
		FKLOG("block add stmt %s", stmt->gettypename());
		stmtlist.push_back(stmt);
	}

	stmt_node_list stmtlist;
};

struct func_desc_node : public syntree_node
{
	func_desc_node() {}
	virtual ~func_desc_node() {}

	virtual esyntreetype gettype()
	{
		return est_func_desc;
	}

	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[func_desc]:";
		ret += funcname;
		ret += "\n";
		if (arglist)
		{
			ret += arglist->dump(indent + 1);
		}
		if (block)
		{
			ret += block->dump(indent + 1);
		}
		return ret;
	}
	
	virtual void recycle();
	
	String funcname;
	func_desc_arglist_node * arglist;
	block_node * block;
	int endline;
};

typedef std::vector<func_desc_node*> func_desc_list;

struct assign_stmt : public syntree_node
{
	assign_stmt() {}
	virtual ~assign_stmt() {}

	virtual esyntreetype gettype()
	{
		return est_assign_stmt;
	}

	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[assign]:\n";
		ret += gentab(indent + 1);
		ret += "[var]:\n";
		ret += var->dump(indent + 2);
		ret += gentab(indent + 1);
		ret += "[value]:\n";
		ret += value->dump(indent + 2);
		return ret;
	}
	
	virtual void recycle();
	
	syntree_node * var;
	syntree_node * value;
	bool isnew;
};

struct math_assign_stmt : public syntree_node
{
	math_assign_stmt() {}
	virtual ~math_assign_stmt() {}

	virtual esyntreetype gettype()
	{
		return est_math_assign_stmt;
	}

	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[mathassign]:";
		ret += oper;
		ret += "\n";
		ret += gentab(indent + 1);
		ret += "[var]:\n";
		ret += var->dump(indent + 2);
		ret += gentab(indent + 1);
		ret += "[value]:\n";
		ret += value->dump(indent + 2);
		return ret;
	}
	
	virtual void recycle();
	
	syntree_node * var;
	String oper;
	syntree_node * value;
};

struct variable_node : public syntree_node
{
	variable_node() {}
	virtual ~variable_node() {}

	virtual esyntreetype gettype()
	{
		return est_variable;
	}

	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[variable]:";
		ret += str;
		ret += "\n";
		return ret;
	}
	
	virtual void recycle();
	
	String str;
};

struct var_node : public syntree_node
{
	var_node() {}
	virtual ~var_node() {}

	virtual esyntreetype gettype()
	{
		return est_var;
	}

	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[var]:";
		ret += str;
		ret += "\n";
		return ret;
	}
	
	virtual void recycle();
	
	String str;
};

typedef std::vector<syntree_node*> func_call_arglist;

struct function_call_arglist_node : public syntree_node
{
	function_call_arglist_node() {}
	virtual ~function_call_arglist_node() {}

	virtual esyntreetype gettype()
	{
		return est_call_arglist;
	}

	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[func_call_arglist]:\n";
		for (int i = 0; i < (int)arglist.size(); i++)
		{
			ret += gentab(indent + 1);
			ret += "[arg";
			ret += fkitoa(i);
			ret += "]:\n";
			ret += arglist[i]->dump(indent + 2);
		}
		return ret;
	}
	
	virtual void recycle();
	
	virtual void add_arg(syntree_node * p)
	{	
		arglist.push_back(p);

		FKLOG("%p add call arg %s", this, p->gettypename());
	}
	
	func_call_arglist arglist;
};

struct function_call_node : public syntree_node
{
	function_call_node() {}
	virtual ~function_call_node() {}

	virtual esyntreetype gettype()
	{
		return est_function_call;
	}
	
	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		if (fakecall)
		{
			ret += "[func_fake_call]:";
		}
		else
		{
			ret += "[func_call]:";
		}
		ret += fuc;
		ret += "\n";
		if (arglist)
		{
			ret += arglist->dump(indent + 1);
		}
		return ret;
	}

	virtual void recycle();

	bool fakecall;
	String fuc;
	function_call_arglist_node * arglist;
};

struct math_expr_node : public syntree_node
{
	math_expr_node() {}
	virtual ~math_expr_node() {}

	virtual esyntreetype gettype()
	{
		return est_math_expr;
	}

	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[math_expr]:";
		ret += oper;
		ret += "\n";
		ret += gentab(indent + 1);
		ret += "[left]:\n";
		ret += left->dump(indent + 2);
		ret += gentab(indent + 1);
		ret += "[right]:\n";
		ret += right->dump(indent + 2);
		return ret;
	}

	virtual void recycle();
	
	String oper;
	syntree_node * left;
	syntree_node * right;
};

struct break_stmt : public syntree_node
{
	break_stmt() {}
	virtual ~break_stmt() {}

	virtual esyntreetype gettype()
	{
		return est_break;
	}
	
	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[break]:\n";
		return ret;
	}
	
	virtual void recycle();
	
};

typedef std::vector<syntree_node *> var_list;

struct var_list_node : public syntree_node
{
	var_list_node() {}
	virtual ~var_list_node() {}

	virtual esyntreetype gettype()
	{
		return est_var_list;
	}

	virtual String dump(int indent);

	virtual void recycle();

	void add_arg(syntree_node * p);

	var_list varlist;
};

struct multi_assign_stmt : public syntree_node
{
	multi_assign_stmt() {}
	virtual ~multi_assign_stmt() {}

	virtual esyntreetype gettype()
	{
		return est_multi_assign_stmt;
	}

	virtual String dump(int indent);

	virtual void recycle();

	var_list_node * varlist;
	syntree_node * value;
	bool isnew;
};

struct container_get_node : public syntree_node
{
	container_get_node() {}
	virtual ~container_get_node() {}

	virtual esyntreetype gettype()
	{
		return est_container_get;
	}

	virtual String dump(int indent);

	virtual void recycle();

	String container;
	syntree_node * key;
};

typedef std::vector<String> struct_memlist;

struct struct_desc_memlist_node : public syntree_node
{
	struct_desc_memlist_node() {}
	virtual ~struct_desc_memlist_node() {}

	virtual esyntreetype gettype()
	{
		return est_struct_memlist;
	}

	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[struct_desc_memlist_node]:\n";
		for (int i = 0; i < (int)memlist.size(); i++)
		{
			ret += gentab(indent + 1);
			ret += memlist[i];
			ret += "\n";
		}
		return ret;
	}

	virtual void recycle();

	void add_arg(String mem)
	{
		memlist.push_back(mem);
	}

	struct_memlist memlist;
};

struct struct_pointer_node : public syntree_node
{
	struct_pointer_node() {}
	virtual ~struct_pointer_node() {}

	virtual esyntreetype gettype()
	{
		return est_struct_pointer;
	}

	virtual String dump(int indent);

	virtual void recycle();

	String str;
};

struct continue_stmt : public syntree_node
{
	continue_stmt() {}
	virtual ~continue_stmt() {}

	virtual esyntreetype gettype()
	{
		return est_continue;
	}
	
	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[continue]:\n";
		return ret;
	}
	
	virtual void recycle();
	
};

struct sleep_stmt : public syntree_node
{
	sleep_stmt() {}
	virtual ~sleep_stmt() {}

	virtual esyntreetype gettype()
	{
		return est_sleep;
	}
	
	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[sleep]:\n";
		ret += gentab(indent + 1);
		ret += "[time]:\n";
		ret += time->dump(indent + 2);
		return ret;
	}
	
	virtual void recycle();

	syntree_node * time;
};

struct yield_stmt : public syntree_node
{
	yield_stmt() {}
	virtual ~yield_stmt() {}

	virtual esyntreetype gettype()
	{
		return est_yield;
	}
	
	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[yield]:\n";
		ret += gentab(indent + 1);
		ret += "[time]:\n";
		ret += time->dump(indent + 2);
		return ret;
	}
	
	virtual void recycle();
	
	syntree_node * time;
};

struct switch_case_node : public syntree_node
{
	switch_case_node() {}
	virtual ~switch_case_node() {}

	virtual esyntreetype gettype()
	{
		return est_switch_case_node;
	}
	
	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[case]:\n";
		ret += gentab(indent + 1);
		ret += "[cmp]:\n";
		ret += cmp->dump(indent + 2);
		ret += gentab(indent + 1);
		if (block)
		{
			ret += "[block]:\n";
			ret += block->dump(indent + 2);
		}
		return ret;
	}

	virtual void recycle();
	
	syntree_node * cmp;
	syntree_node * block;
};

typedef std::vector<syntree_node *> switch_case_list;

struct switch_caselist_node : public syntree_node
{
	switch_caselist_node() {}
	virtual ~switch_caselist_node() {}

	virtual esyntreetype gettype()
	{
		return est_switch_caselist;
	}
	
	virtual String dump(int indent)
	{
		String ret;
		for (int i = 0; i < (int)list.size(); i++)
		{
			ret += list[i]->dump(indent);
		}
		return ret;
	}
	
	void add_case(syntree_node * p)
	{
		list.push_back(p);
	}
	
	virtual void recycle();
	
	switch_case_list list;
};

struct switch_stmt : public syntree_node
{
	switch_stmt() {}
	virtual ~switch_stmt() {}

	virtual esyntreetype gettype()
	{
		return est_switch_stmt;
	}
	
	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[switch]:\n";
		ret += gentab(indent + 1);
		ret += "[cmp]:\n";
		ret += cmp->dump(indent + 2);
		ret += gentab(indent + 1);
		ret += "[case list]:\n";
		ret += caselist->dump(indent + 2);
		ret += gentab(indent + 1);
		if (def)
		{
			ret += "[default]:\n";
			ret += def->dump(indent + 2);
		}
		return ret;
	}
	
	virtual void recycle();
	
	syntree_node * cmp;
	syntree_node * caselist;
	syntree_node * def;
};

struct for_loop_stmt : public syntree_node
{
	for_loop_stmt() {}
	virtual ~for_loop_stmt() {}

	virtual esyntreetype gettype()
	{
		return est_for_loop_stmt;
	}
	
	virtual String dump(int indent)
	{
		String ret;
		ret += gentab(indent);
		ret += "[for_loop_stmt]:\n";
		ret += gentab(indent + 1);
		ret += "[var]:\n";
		ret += var->dump(indent + 2);
		ret += gentab(indent + 1);
		ret += "[begin]:\n";
		ret += begin->dump(indent + 2);
		ret += gentab(indent + 1);
		ret += "[end]:\n";
		ret += end->dump(indent + 2);
		ret += gentab(indent + 1);
		ret += "[add]:\n";
		ret += add->dump(indent + 2);
		ret += gentab(indent + 1);
		if (block)
		{
			ret += "[block]:\n";
			ret += block->dump(indent + 2);
		}
		return ret;
	}
	
	virtual void recycle();
	
	syntree_node * var;
	syntree_node * begin;
	syntree_node * end;
	syntree_node * add;
	block_node * block;
};

