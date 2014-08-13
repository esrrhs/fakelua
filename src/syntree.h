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
    est_stmt,
    est_while_stmt,
    est_cmp_stmt,
    est_if_stmt,
    est_else_stmt,
};

struct syntree_node
{
    syntree_node() : lineno(0) {}
    virtual ~syntree_node() {}

    virtual esyntreetype gettype()
    {
        return est_func_nil;
    }

    int lineno;
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

    func_desc_arglist arglist;
};

struct stmt_node : public syntree_node
{
    stmt_node() {}
    virtual ~stmt_node() {}
    
    virtual esyntreetype gettype()
    {
        return est_stmt;
    }

};

struct cmp_stmt : public stmt_node
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
struct while_stmt : public stmt_node
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

struct else_stmt : public stmt_node
{
    else_stmt() {}
    virtual ~else_stmt() {}
    
    virtual esyntreetype gettype()
    {
        return est_else_stmt;
    }

    block_node * block;
};

struct if_stmt : public stmt_node
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

typedef std::list<stmt_node *> stmt_node_list;

struct block_node : public syntree_node
{
    block_node() {}
    virtual ~block_node() {}

    virtual esyntreetype gettype()
    {
        return est_block;
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

