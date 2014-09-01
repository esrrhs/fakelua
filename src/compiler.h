#pragma once

#include "types.h"
#include "binary.h"
#include "codegen.h"

class fuck;
class myflexer;
class func_desc_node;
class block_node;
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

private:
    bool add_stack_identifier();
    void push_stack_identifiers();
    void pop_stack_identifiers();
    
private:
    fuck * m_fk;
    binary m_binary;
};

