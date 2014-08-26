#pragma once

#include <string>
#include <list>
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

enum esyntreetype
{
    est_func_nil,
    est_func_desc,
    est_bin,
    est_arglist,
    est_block,
    est_while_stmt,
    est_cmp_stmt,
    est_if_stmt,
    est_else_stmt,
    est_explicit_value,
    est_return_stmt,
    est_assign_stmt,
    est_variable,
    est_var,
    est_function_call,
    est_call_arglist,
    est_math_expr,
    est_break,
    est_identifier,
};

const char * get_syntree_node_name(esyntreetype type);

struct syntree_node
{
    syntree_node() : lineno(0) {}
    virtual ~syntree_node() {}

    virtual esyntreetype gettype()
    {
        return est_func_nil;
    }
    virtual const char * gettypename()
    {
        return get_syntree_node_name(gettype());
    }

    int lineno;
};

struct identifier_node : public syntree_node
{
    identifier_node() {}
    virtual ~identifier_node() {}
    
    virtual esyntreetype gettype()
    {
        return est_identifier;
    }

    String str;
};

typedef std::list<String> func_desc_arglist;

struct func_desc_arglist_node : public syntree_node
{
    func_desc_arglist_node() {}
    virtual ~func_desc_arglist_node() {}

    virtual esyntreetype gettype()
    {
        return est_arglist;
    }

    virtual void add_arg(syntree_node * p)
    {
        assert(p->gettype() == est_identifier);		
        identifier_node * pi = dynamic_cast<identifier_node*>(p);
        arglist.push_back(pi->str);

        FKLOG("%p add arg %s", this, pi->str.c_str());
    }

    func_desc_arglist arglist;
};

struct explicit_value_node : public syntree_node
{
    explicit_value_node() {}
    virtual ~explicit_value_node() {}
    
    virtual esyntreetype gettype()
    {
        return est_explicit_value;
    }

    String str;
};

struct return_stmt : public syntree_node
{
    return_stmt() {}
    virtual ~return_stmt() {}

    virtual esyntreetype gettype()
    {
        return est_return_stmt;
    }

    syntree_node * ret;
};

struct cmp_stmt : public syntree_node
{
    cmp_stmt() {}
    virtual ~cmp_stmt() {}
    
    virtual esyntreetype gettype()
    {
        return est_cmp_stmt;
    }

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

    block_node * block;
};

struct if_stmt : public syntree_node
{
    if_stmt() {}
    virtual ~if_stmt() {}
    
    virtual esyntreetype gettype()
    {
        return est_if_stmt;
    }

    cmp_stmt * cmp;
    block_node * block;
    else_stmt * elses;
};

typedef std::list<syntree_node *> stmt_node_list;

struct block_node : public syntree_node
{
    block_node() {}
    virtual ~block_node() {}

    virtual esyntreetype gettype()
    {
        return est_block;
    }

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

    String funcname;
    func_desc_arglist_node * arglist;
    block_node * block;
};

struct assign_stmt : public syntree_node
{
    assign_stmt() {}
    virtual ~assign_stmt() {}

    virtual esyntreetype gettype()
    {
        return est_assign_stmt;
    }

    syntree_node * var;
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

    String str;
};

typedef std::list<syntree_node*> func_call_arglist;

struct function_call_arglist_node : public syntree_node
{
    function_call_arglist_node() {}
    virtual ~function_call_arglist_node() {}

    virtual esyntreetype gettype()
    {
        return est_call_arglist;
    }

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
};

