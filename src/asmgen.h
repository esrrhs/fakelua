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
        m_source.clear();
    }

    void push(char c)
    {
        m_asm_code_list.push_back(c);
    }

    // push   %rbp
    // mov    %rsp,%rbp
    void start_func()
    {
        push(0x55);
        push(0x48);
        push(0x89);
        push(0xe5);
        m_source += "push   %rbp\nmov    %rsp,%rbp\n";
    }

    void push_int(int i)
    {
        push(((char*)&i)[0]);
        push(((char*)&i)[1]);
        push(((char*)&i)[2]);
        push(((char*)&i)[3]);
    }
    
    void push_int64(int64_t i)
    {
        push(((char*)&i)[0]);
        push(((char*)&i)[1]);
        push(((char*)&i)[2]);
        push(((char*)&i)[3]);
        push(((char*)&i)[4]);
        push(((char*)&i)[5]);
        push(((char*)&i)[6]);
        push(((char*)&i)[7]);
    }

    // sub    $size,%rsp
    void alloc_stack(int size)
    {
        push(0x48);
        push(0x81);
        push(0xec);
        push_int(size);
        m_source += "sub    $";
        m_source += fkxtoa(size, 8);
        m_source += ",%rsp\n";
    }

    // 把num立即数放到offset的地方
    // movl   $num,offset(%rbp)
    void mov_l_rbp(int num, int offset)
    {
        assert(offset <= 0);
        push(0xc7);
        push(0x85);
        push_int(offset);
        push_int(num);
        m_source += "movl   $";
        m_source += fkxtoa(num, 8);
        m_source += ",-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp)\n";
    }

    // 把num立即数放到rax的地方
    // mov    $num,%rax
    void mov_ll_rax(int64_t num)
    {
        push(0x48);
        push(0xb8);
        push_int64(num);
        m_source += "mov    $";
        m_source += fkxtoa(num, 16);
        m_source += ",%rax\n";
    }

    // 把rax放到offset的地方
    // mov    %rax,offset(%rbp)
    void mov_rax_rbp(int offset)
    {
        assert(offset <= 0);
        push(0x48);
        push(0x89);
        push(0x85);
        push_int(offset);
        m_source += "mov    %rax,-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp)\n";
    }
    
    // 把offset的地方放到rax
    // mov    offset(%rbp),%rax
    void mov_rbp_rax(int offset)
    {
        assert(offset <= 0);
        push(0x48);
        push(0x8b);
        push(0x85);
        push_int(offset);
        m_source += "mov    -";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp),%rax\n";
    }
    
    // leaveq
    // retq
    void stop_func()
    {
        push(0xc9);
        push(0xc3);
        m_source += "leaveq\nretq\n";
    }
    
    void copy_const(variant * p, size_t num, int start);

    void variant_assign(int leftpos, int rightpos);

    void output(const String & name, func_native * nt);

    const String & source() const
    {
        return m_source;
    }
private:
    fuck * m_fk;
	asm_code_list m_asm_code_list;
	String m_source;
};

