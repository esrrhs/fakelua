#pragma once

#include "types.h"
#include "binary.h"
#include "variant.h"

typedef std::vector<char> asm_code_list;

struct fuck;
class func_native;
class asmgen
{
public:
    asmgen(fuck * fk) : m_fk(fk)
    {
        clear();
    }
    ~asmgen() {}

    void clear()
    {
        m_asm_code_list.clear();
    }

    void push(char c)
    {
        m_asm_code_list.push_back(c);
    }

    void start_func()
    {
        push(0x55);
        push(0x48);
        push(0x89);
        push(0xe5);
    }
    
    void alloc_stack(int size)
    {
        push(0x48);
        push(0x81);
        push(0xec);
        push(((char*)&size)[0]);
        push(((char*)&size)[1]);
        push(((char*)&size)[2]);
        push(((char*)&size)[3]);
    }
    
    void stop_func()
    {
        push(0xc9);
        push(0xc3);
    }
    
    void copy_const(variant * p, size_t num);

    void output(const String & name, func_native * nt);
    
private:
    fuck * m_fk;
	asm_code_list m_asm_code_list;
};

