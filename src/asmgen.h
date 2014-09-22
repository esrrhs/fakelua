#pragma once

#include "types.h"
#include "binary.h"
#include "variant.h"

typedef std::vector<char> asm_code_list;


/*

低地址 rsp
             |
             |      
             | stack
             |
             |
高地址 rbp

*/

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

    // 保存函数栈指针
    // push   %rbp
    // mov    %rsp,%rbp
    void start_func()
    {
        push(0x55);
        push(0x48);
        push(0x89);
        push(0xe5);
        m_source += "push   %rbp\n"
        "mov    %rsp,%rbp\n";
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

    // 分配栈空间
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

    // 弹出rax
    // pop    %rax
    void pop_rax()
    {
        push(0x58);
        m_source += "pop    %rax\n";
    }

    // 压入rax
    // push    %rax
    void push_rax()
    {
        push(0x50);
        m_source += "push    %rax\n";
    }

    // 把rdx放到offset的地方
    // mov    %rdx,offset(%rbp)
    void mov_rdx_rbp(int offset)
    {
        assert(offset <= 0);
        push(0x48);
        push(0x89);
        push(0x95);
        push_int(offset);
        m_source += "mov    %rdx,-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp)\n";
    }
    
    // 把offset的地方放到rdx
    // mov    offset(%rbp),%rdx
    void mov_rbp_rdx(int offset)
    {
        assert(offset <= 0);
        push(0x48);
        push(0x8b);
        push(0x95);
        push_int(offset);
        m_source += "mov    -";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp),%rdx\n";
    }

    // 弹出rdx
    // pop    %rdx
    void pop_rdx()
    {
        push(0x5a);
        m_source += "pop    %rdx\n";
    }

    // 压入rdx
    // push    %rdx
    void push_rdx()
    {
        push(0x52);
        m_source += "push    %rdx\n";
    }

    // 把offset的地方放到xmm0
    // movsd    offset(%rbp),%xmm0
    void movsd_rbp_xmm0(int offset)
    {
        assert(offset <= 0);
        push(0xf2);
        push(0x0f);
        push(0x10);
        push(0x85);
        push_int(offset);
        m_source += "movsd    -";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp),%xmm0\n";
    }

    // 把xmm0放到offset的地方
    // movsd    %xmm0,offset(%rbp)
    void movsd_xmm0_rbp(int offset)
    {
        assert(offset <= 0);
        push(0xf2);
        push(0x0f);
        push(0x11);
        push(0x85);
        push_int(offset);
        m_source += "movsd    %xmm0,-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp)\n";
    }

    // 把offset的地方和xmm0做加法
    // addsd    offset(%rbp),%xmm0
    void addsd_rbp_xmm0(int offset)
    {
        assert(offset <= 0);
        push(0xf2);
        push(0x0f);
        push(0x58);
        push(0x85);
        push_int(offset);
        m_source += "addsd    -";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp),%xmm0\n";
    }
    
    // 把offset的地方和xmm0做减法
    // subsd    offset(%rbp),%xmm0
    void subsd_rbp_xmm0(int offset)
    {
        assert(offset <= 0);
        push(0xf2);
        push(0x0f);
        push(0x5c);
        push(0x85);
        push_int(offset);
        m_source += "subsd    -";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp),%xmm0\n";
    }
    
    // 把offset的地方和xmm0做乘法
    // mulsd    offset(%rbp),%xmm0
    void mulsd_rbp_xmm0(int offset)
    {
        assert(offset <= 0);
        push(0xf2);
        push(0x0f);
        push(0x59);
        push(0x85);
        push_int(offset);
        m_source += "mulsd    -";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp),%xmm0\n";
    }
    
    // 把offset的地方和xmm0做除法
    // divsd    offset(%rbp),%xmm0
    void divsd_rbp_xmm0(int offset)
    {
        assert(offset <= 0);
        push(0xf2);
        push(0x0f);
        push(0x5e);
        push(0x85);
        push_int(offset);
        m_source += "divsd    -";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp),%xmm0\n";
    }

	// 取模运算
	// cvttsd2si leftoff(%rbp), %rax
	// cvttsd2si rightoff(%rbp), %rcx
	// mov %rax, %rdx
	// sar $0x3f, %rdx
	// idivq %rcx
	// cvtsi2sd %rdx, %xmm0
	// movq %xmm0, destoff(%rbp)
	void divide_mod_rbp(int leftoff, int rightoff, int destoff)
	{
		assert(leftoff <= 0);
		assert(rightoff <= 0);
		assert(destoff <= 0);
		push(0xf2);
		push(0x48); 
		push(0x0f); 
		push(0x2c); 
		push(0x85);
		push_int(leftoff);
		push(0xf2); 
		push(0x48); 
		push(0x0f); 
		push(0x2c); 
		push(0x8d);
		push_int(rightoff);
		push(0x48); push(0x89); push(0xc2);
		push(0x48); push(0xc1); push(0xfa); push(0x3f);
		push(0x48); push(0xf7); push(0xf9);
		push(0xf2); push(0x48); push(0x0f); push(0x2a); push(0xc2);
		push(0x66); push(0x0f); push(0xd6); push(0x85);
		push_int(destoff);
	}

    // 返回
    // leaveq
    // retq
    void stop_func()
    {
        push(0xc9);
        push(0xc3);
        m_source += "leaveq\nretq\n";
    }

    void copy_param(size_t num);
    void copy_const(variant * p, size_t num, int start);
    
    void variant_assign(int leftpos, int rightpos);
    void variant_ret(int pos);
    void variant_add(int destpos, int leftpos, int rightpos);
    void variant_sub(int destpos, int leftpos, int rightpos);
	void variant_mul(int destpos, int leftpos, int rightpos);
	void variant_div(int destpos, int leftpos, int rightpos);
	void variant_div_mod(int destpos, int leftpos, int rightpos);
    
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

