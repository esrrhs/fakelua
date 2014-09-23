#pragma once

#include "types.h"
#include "codegen.h"
#include "syntree.h"
#include "asmgen.h"

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
    bool compile_func(const func_binary & fb);
    bool compile_next(asmgen & asg, const func_binary & fb);
    bool compile_assign(asmgen & asg, const func_binary & fb, command cmd);
    bool compile_return(asmgen & asg, const func_binary & fb, command cmd);
    bool compile_math(asmgen & asg, const func_binary & fb, command cmd);
	bool compile_math_assign(asmgen & asg, const func_binary & fb, command cmd);
	bool compile_cmp(asmgen & asg, const func_binary & fb, command cmd);

private:
    fuck * m_fk;
    native * m_native;
    int m_pos;
};

