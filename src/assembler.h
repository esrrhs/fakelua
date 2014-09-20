#pragma once

#include "types.h"
#include "codegen.h"
#include "syntree.h"

struct fuck;
class native;
class binary;
class func_binary;
class assembler
{
public:
    assembler(fuck * fk, native * nt) : m_fk(fk), m_native(nt)
    {
    }
    ~assembler() {}

    void clear();
    bool compile(binary * bin);

private:
    bool compile_func(func_binary * fb);
    bool compile_command(command cmd);
    
private:
    fuck * m_fk;
    native * m_native;
};

