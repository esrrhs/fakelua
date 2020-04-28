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

struct fake;
struct func_native;

class asmgen {
public:
    asmgen(fake *fk) : m_fk(fk) {
        clear();
    }

    ~asmgen() {}

    void clear() {
        m_asm_code_list.clear();
        m_source.clear();
    }

    void push(unsigned char c) {
        m_asm_code_list.push_back(c);
    }

    size_t size() const {
        return m_asm_code_list.size();
    }

    void set_int(int offset, int i) {
        assert(m_asm_code_list.size() >= offset + sizeof(i));
        m_asm_code_list[offset] = (((char *) &i)[0]);
        m_asm_code_list[offset + 1] = (((char *) &i)[1]);
        m_asm_code_list[offset + 2] = (((char *) &i)[2]);
        m_asm_code_list[offset + 3] = (((char *) &i)[3]);
    }

    // 保存函数栈指针
    // push   %rbp
    // mov	%rsp,%rbp
    void start_func() {
        push(0x55);
        push(0x48);
        push(0x89);
        push(0xe5);
        m_source += "push   %rbp\n"
                    "mov	%rsp,%rbp\n";
    }

    void push_int(int i) {
        push(((char *) &i)[0]);
        push(((char *) &i)[1]);
        push(((char *) &i)[2]);
        push(((char *) &i)[3]);
    }

    void push_int64(int64_t i) {
        push(((char *) &i)[0]);
        push(((char *) &i)[1]);
        push(((char *) &i)[2]);
        push(((char *) &i)[3]);
        push(((char *) &i)[4]);
        push(((char *) &i)[5]);
        push(((char *) &i)[6]);
        push(((char *) &i)[7]);
    }

    // 分配栈空间
    // sub	$size,%rsp
    void alloc_stack(int size) {
        push(0x48);
        push(0x81);
        push(0xec);
        push_int(size);
        m_source += "sub	$";
        m_source += fkxtoa(size, 8);
        m_source += ",%rsp\n";
    }

    // 把num立即数放到offset的地方
    // movl   $num,offset(%rbp)
    void mov_l_rbp(int num, int offset) {
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
    // mov	$num,%rax
    void mov_ll_rax(int64_t num) {
        push(0x48);
        push(0xb8);
        push_int64(num);
        m_source += "mov	$";
        m_source += fkxtoa(num, 16);
        m_source += ",%rax\n";
    }

    // 把立即数放到rax指向的地方
    // movq   $num,(%rax)
    void movq_p_rax(int num) {
        push(0x48);
        push(0xc7);
        push(0x00);
        push_int(num);
        m_source += "movq	$";
        m_source += fkxtoa(num, 16);
        m_source += ",(%rax)\n";
    }

    // 把num立即数放到rdi的地方
    // mov	$num,%rdi
    void mov_ll_rdi(int64_t num) {
        push(0x48);
        push(0xbf);
        push_int64(num);
        m_source += "mov	$";
        m_source += fkxtoa(num, 16);
        m_source += ",%rdi\n";
    }

    // 把num立即数放到rcx的地方
    // mov	$num,%rcx
    void mov_ll_rcx(int64_t num) {
        push(0x48);
        push(0xb9);
        push_int64(num);
        m_source += "mov	$";
        m_source += fkxtoa(num, 16);
        m_source += ",%rcx\n";
    }

    // 把rax放到offset的地方
    // mov	%rax,offset(%rbp)
    void mov_rax_rbp(int offset) {
        assert(offset <= 0);
        push(0x48);
        push(0x89);
        push(0x85);
        push_int(offset);
        m_source += "mov	%rax,-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp)\n";
    }

    // 把offset的地方放到rax
    // mov	offset(%rbp),%rax
    void mov_rbp_rax(int offset) {
        assert(offset <= 0);
        push(0x48);
        push(0x8b);
        push(0x85);
        push_int(offset);
        m_source += "mov	-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp),%rax\n";
    }

    // 弹出rax
    // pop	%rax
    void pop_rax() {
        push(0x58);
        m_source += "pop	%rax\n";
    }

    // 压入rax
    // push	%rax
    void push_rax() {
        push(0x50);
        m_source += "push	%rax\n";
    }

    // 把rdx放到offset的地方
    // mov	%rdx,offset(%rbp)
    void mov_rdx_rbp(int offset) {
        assert(offset <= 0);
        push(0x48);
        push(0x89);
        push(0x95);
        push_int(offset);
        m_source += "mov	%rdx,-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp)\n";
    }

    // 把offset的地方放到rdx
    // mov	offset(%rbp),%rdx
    void mov_rbp_rdx(int offset) {
        assert(offset <= 0);
        push(0x48);
        push(0x8b);
        push(0x95);
        push_int(offset);
        m_source += "mov	-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp),%rdx\n";
    }

    // 把rdi放到offset的地方
    // mov	%rdi,offset(%rbp)
    void mov_rdi_rbp(int offset) {
        assert(offset <= 0);
        push(0x48);
        push(0x89);
        push(0xbd);
        push_int(offset);
        m_source += "mov	%rdi,-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp)\n";
    }

    // 把offset的地方放到rdi
    // mov	offset(%rbp),%rdi
    void mov_rbp_rdi(int offset) {
        assert(offset <= 0);
        push(0x48);
        push(0x8b);
        push(0xbd);
        push_int(offset);
        m_source += "mov	-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp),%rdi\n";
    }

    // 弹出rdx
    // pop	%rdx
    void pop_rdx() {
        push(0x5a);
        m_source += "pop	%rdx\n";
    }

    // 压入rdx
    // push	%rdx
    void push_rdx() {
        push(0x52);
        m_source += "push	%rdx\n";
    }

    // 把offset的地方放到xmm0
    // movsd	offset(%rbp),%xmm0
    void movsd_rbp_xmm0(int offset) {
        assert(offset <= 0);
        push(0xf2);
        push(0x0f);
        push(0x10);
        push(0x85);
        push_int(offset);
        m_source += "movsd	-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp),%xmm0\n";
    }

    // 把xmm0放到offset的地方
    // movsd	%xmm0,offset(%rbp)
    void movsd_xmm0_rbp(int offset) {
        assert(offset <= 0);
        push(0xf2);
        push(0x0f);
        push(0x11);
        push(0x85);
        push_int(offset);
        m_source += "movsd	%xmm0,-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp)\n";
    }

    // 把offset的地方和xmm0做加法
    // addsd	offset(%rbp),%xmm0
    void addsd_rbp_xmm0(int offset) {
        assert(offset <= 0);
        push(0xf2);
        push(0x0f);
        push(0x58);
        push(0x85);
        push_int(offset);
        m_source += "addsd	-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp),%xmm0\n";
    }

    // 把offset的地方和xmm0做减法
    // subsd	offset(%rbp),%xmm0
    void subsd_rbp_xmm0(int offset) {
        assert(offset <= 0);
        push(0xf2);
        push(0x0f);
        push(0x5c);
        push(0x85);
        push_int(offset);
        m_source += "subsd	-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp),%xmm0\n";
    }

    // 把offset的地方和xmm0做乘法
    // mulsd	offset(%rbp),%xmm0
    void mulsd_rbp_xmm0(int offset) {
        assert(offset <= 0);
        push(0xf2);
        push(0x0f);
        push(0x59);
        push(0x85);
        push_int(offset);
        m_source += "mulsd	-";
        m_source += fkxtoa(-offset, 8);
        m_source += "(%rbp),%xmm0\n";
    }

    // 把offset的地方和xmm0做除法
    // divsd	offset(%rbp),%xmm0
    void divsd_rbp_xmm0(int offset) {
        assert(offset <= 0);
        push(0xf2);
        push(0x0f);
        push(0x5e);
        push(0x85);
        push_int(offset);
        m_source += "divsd	-";
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
    void divide_mod_rbp(int leftoff, int rightoff, int destoff) {
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
        push(0x48);
        push(0x89);
        push(0xc2);
        push(0x48);
        push(0xc1);
        push(0xfa);
        push(0x3f);
        push(0x48);
        push(0xf7);
        push(0xf9);
        push(0xf2);
        push(0x48);
        push(0x0f);
        push(0x2a);
        push(0xc2);
        push(0x66);
        push(0x0f);
        push(0xd6);
        push(0x85);
        push_int(destoff);
        m_source += "cvttsd2si -" + fkxtoa(-leftoff, 8) + "(%rbp), %rax\n";
        m_source += "cvttsd2si -" + fkxtoa(-rightoff, 8) + "(%rbp), %rcx\n";
        m_source += "mov %rax, %rdx\n";
        m_source += "sar $0x3f, %rdx\n";
        m_source += "idivq %rcx\n";
        m_source += "cvtsi2sd %rdx, %xmm0\n";
        m_source += "movq %xmm0, -" + fkxtoa(-destoff, 8) + "(%rbp)\n";
    }

    // AND运算
    //	xorpd  %xmm0,%xmm0
    //	ucomisd leftoff(%rbp),%xmm0
    //	setne  %dl
    //	setp   %al
    //	or	 %eax,%edx
    //	xorpd  %xmm0,%xmm0
    //	ucomisd rightoff(%rbp),%xmm0
    //	setne  %al
    //	setp   %cl
    //	or	 %ecx,%eax
    //	and	%edx,%eax
    //	movzbl %al,%eax
    //	cvtsi2sd %eax,%xmm0
    //	movsd  %xmm0,destoff(%rbp)
    void and_rbp(int leftoff, int rightoff, int destoff) {
        assert(leftoff <= 0);
        assert(rightoff <= 0);
        assert(destoff <= 0);
        push(0x66);
        push(0x0f);
        push(0x57);
        push(0xc0);
        push(0x66);
        push(0x0f);
        push(0x2e);
        push(0x85);
        push_int(leftoff);
        push(0x0f);
        push(0x95);
        push(0xc2);
        push(0x0f);
        push(0x9a);
        push(0xc0);
        push(0x09);
        push(0xc2);
        push(0x66);
        push(0x0f);
        push(0x57);
        push(0xc0);
        push(0x66);
        push(0x0f);
        push(0x2e);
        push(0x85);
        push_int(rightoff);
        push(0x0f);
        push(0x95);
        push(0xc0);
        push(0x0f);
        push(0x9a);
        push(0xc1);
        push(0x09);
        push(0xc8);
        push(0x21);
        push(0xd0);
        push(0x0f);
        push(0xb6);
        push(0xc0);
        push(0xf2);
        push(0x0f);
        push(0x2a);
        push(0xc0);
        push(0xf2);
        push(0x0f);
        push(0x11);
        push(0x85);
        push_int(destoff);
        m_source += "xorpd  %xmm0, %xmm0\n";
        m_source += "ucomisd -" + fkxtoa(-leftoff, 8) + "(%rbp), %xmm0\n";
        m_source += "setne  %dl\n";
        m_source += "setp   %al\n";
        m_source += "or	 %eax, %edx\n";
        m_source += "xorpd  %xmm0, %xmm0\n";
        m_source += "ucomisd -" + fkxtoa(-rightoff, 8) + "(%rbp), %xmm0\n";
        m_source += "setne  %al\n";
        m_source += "setp   %cl\n";
        m_source += "or	 %ecx, %eax\n";
        m_source += "and	%edx, %eax\n";
        m_source += "movzbl %al, %eax\n";
        m_source += "cvtsi2sd %eax, %xmm0\n";
        m_source += "movsd  %xmm0, -" + fkxtoa(-destoff, 8) + "(%rbp)\n";
    }

    // OR运算
    //	xorpd  %xmm0,%xmm0
    //	ucomisd leftoff(%rbp),%xmm0
    //	setne  %dl
    //	setp   %al
    //	or	 %eax,%edx
    //	xorpd  %xmm0,%xmm0
    //	ucomisd rightoff(%rbp),%xmm0
    //	setne  %al
    //	setp   %cl
    //	or	 %ecx,%eax
    //	or	%edx,%eax
    //	movzbl %al,%eax
    //	cvtsi2sd %eax,%xmm0
    //	movsd  %xmm0,destoff(%rbp)
    void or_rbp(int leftoff, int rightoff, int destoff) {
        assert(leftoff <= 0);
        assert(rightoff <= 0);
        assert(destoff <= 0);
        push(0x66);
        push(0x0f);
        push(0x57);
        push(0xc0);
        push(0x66);
        push(0x0f);
        push(0x2e);
        push(0x85);
        push_int(leftoff);
        push(0x0f);
        push(0x95);
        push(0xc2);
        push(0x0f);
        push(0x9a);
        push(0xc0);
        push(0x09);
        push(0xc2);
        push(0x66);
        push(0x0f);
        push(0x57);
        push(0xc0);
        push(0x66);
        push(0x0f);
        push(0x2e);
        push(0x85);
        push_int(rightoff);
        push(0x0f);
        push(0x95);
        push(0xc0);
        push(0x0f);
        push(0x9a);
        push(0xc1);
        push(0x09);
        push(0xc8);
        push(0x09);
        push(0xd0);
        push(0x0f);
        push(0xb6);
        push(0xc0);
        push(0xf2);
        push(0x0f);
        push(0x2a);
        push(0xc0);
        push(0xf2);
        push(0x0f);
        push(0x11);
        push(0x85);
        push_int(destoff);
        m_source += "xorpd  %xmm0, %xmm0\n";
        m_source += "ucomisd -" + fkxtoa(-leftoff, 8) + "(%rbp), %xmm0\n";
        m_source += "setne  %dl\n";
        m_source += "setp   %al\n";
        m_source += "or	 %eax, %edx\n";
        m_source += "xorpd  %xmm0, %xmm0\n";
        m_source += "ucomisd -" + fkxtoa(-rightoff, 8) + "(%rbp), %xmm0\n";
        m_source += "setne  %al\n";
        m_source += "setp   %cl\n";
        m_source += "or	 %ecx, %eax\n";
        m_source += "or	%edx, %eax\n";
        m_source += "movzbl %al, %eax\n";
        m_source += "cvtsi2sd %eax, %xmm0\n";
        m_source += "movsd  %xmm0, -" + fkxtoa(-destoff, 8) + "(%rbp)\n";
    }

    // LESS运算
    //  movsd  leftoff(%rbp),%xmm1
    //	movsd  rightoff(%rbp), %xmm0
    // 	ucomisd %xmm1, %xmm0
    // 	seta   %al
    // 	test   %al, %al
    // 	je	 .+0x0e
    // 	mov	$0x3ff0000000000000, %rax
    // 	jmp	.+0x07
    // 	mov	$0x0, %eax
    // 	mov	%rax, destoff(%rbp)
    void less_rbp(int leftoff, int rightoff, int destoff) {
        assert(leftoff <= 0);
        assert(rightoff <= 0);
        assert(destoff <= 0);
        push(0xf2);
        push(0x0f);
        push(0x10);
        push(0x8d);
        push_int(leftoff);
        push(0xf2);
        push(0x0f);
        push(0x10);
        push(0x85);
        push_int(rightoff);
        push(0x66);
        push(0x0f);
        push(0x2e);
        push(0xc1);
        push(0x0f);
        push(0x97);
        push(0xc0);
        push(0x84);
        push(0xc0);
        push(0x74);
        push(0x0c);
        push(0x48);
        push(0xb8);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0xf0);
        push(0x3f);
        push(0xeb);
        push(0x05);
        push(0xb8);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x48);
        push(0x89);
        push(0x85);
        push_int(destoff);
        m_source += "movsd  -" + fkxtoa(-leftoff, 8) + "(%rbp), %xmm1\n";
        m_source += "movsd  -" + fkxtoa(-rightoff, 8) + "(%rbp), %xmm0\n";
        m_source += "ucomisd %xmm1, %xmm0\n";
        m_source += "seta   %al\n";
        m_source += "test   %al, %al\n";
        m_source += "je	 .+0x0e\n";
        m_source += "mov	$0x3ff0000000000000, %rax\n";
        m_source += "jmpq   .+0x07\n";
        m_source += "mov	$0x0, %eax\n";
        m_source += "mov	%rax, -" + fkxtoa(-destoff, 8) + "(%rbp)\n";
    }

    // MORE运算
    void more_rbp(int leftoff, int rightoff, int destoff) {
        // 交换律
        less_equal_rbp(rightoff, leftoff, destoff);
    }

    // EQUAL运算
    //	movsd  leftoff(%rbp), %xmm0
    //	ucomisd rightoff(%rbp), %xmm0
    // 	jne	.+0x10
    // 	jp	 .+0x0e
    // 	mov	$0x3ff0000000000000, %rax
    // 	jmp	.+0x07
    // 	mov	$0x0, %eax
    // 	mov	%rax, destoff(%rbp)
    void equal_rbp(int leftoff, int rightoff, int destoff) {
        assert(leftoff <= 0);
        assert(rightoff <= 0);
        assert(destoff <= 0);
        push(0xf2);
        push(0x0f);
        push(0x10);
        push(0x85);
        push_int(leftoff);
        push(0x66);
        push(0x0f);
        push(0x2e);
        push(0x85);
        push_int(rightoff);
        push(0x75);
        push(0x0e);
        push(0x7a);
        push(0x0c);
        push(0x48);
        push(0xb8);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0xf0);
        push(0x3f);
        push(0xeb);
        push(0x05);
        push(0xb8);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x48);
        push(0x89);
        push(0x85);
        push_int(destoff);
        m_source += "movsd  -" + fkxtoa(-leftoff, 8) + "(%rbp), %xmm0\n";
        m_source += "ucomisd -" + fkxtoa(-rightoff, 8) + "(%rbp), %xmm0\n";
        m_source += "jne	.+0x10\n";
        m_source += "jp	 .+0x0e\n";
        m_source += "mov	$0x3ff0000000000000, %rax\n";
        m_source += "jmp	.+0x07	\n";
        m_source += "mov	$0x0, %eax\n";
        m_source += "mov	%rax, -" + fkxtoa(-destoff, 8) + "(%rbp)\n";
    }

    // LESS EQUAL运算
    //	movsd  leftoff(%rbp), %xmm1
    //	movsd  rightoff(%rbp), %xmm0
    //	ucomisd %xmm1, %xmm0
    //	setae  %al
    //	test   %al, %al
    //	je	 . + 0x0e
    //	mov	$0x3ff0000000000000, %rax
    //	jmp	. + 0x07
    //	mov	$0x0, %eax
    //	mov	%rax, destoff(%rbp)
    void less_equal_rbp(int leftoff, int rightoff, int destoff) {
        assert(leftoff <= 0);
        assert(rightoff <= 0);
        assert(destoff <= 0);
        push(0xf2);
        push(0x0f);
        push(0x10);
        push(0x8d);
        push_int(leftoff);
        push(0xf2);
        push(0x0f);
        push(0x10);
        push(0x85);
        push_int(rightoff);
        push(0x66);
        push(0x0f);
        push(0x2e);
        push(0xc1);
        push(0x0f);
        push(0x93);
        push(0xc0);
        push(0x84);
        push(0xc0);
        push(0x74);
        push(0x0c);
        push(0x48);
        push(0xb8);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0xf0);
        push(0x3f);
        push(0xeb);
        push(0x05);
        push(0xb8);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x48);
        push(0x89);
        push(0x85);
        push_int(destoff);
        m_source += "movsd  -" + fkxtoa(-leftoff, 8) + "(%rbp), %xmm1\n";
        m_source += "movsd  -" + fkxtoa(-rightoff, 8) + "(%rbp), %xmm0\n";
        m_source += "ucomisd %xmm1, %xmm0\n";
        m_source += "setae  %al\n";
        m_source += "test   %al, %al\n";
        m_source += "je	 . + 0x0e\n";
        m_source += "mov	$0x3ff0000000000000, %rax\n";
        m_source += "jmp	. + 0x07\n";
        m_source += "mov	$0x0, %eax\n";
        m_source += "mov	%rax, -" + fkxtoa(-destoff, 8) + "(%rbp)\n";
    }

    // MORE EQUAL运算
    void more_equal_rbp(int leftoff, int rightoff, int destoff) {
        // 交换律
        less_rbp(rightoff, leftoff, destoff);
    }

    // NOT EQUAL运算
    //	movsd  leftoff(%rbp), %xmm0
    //	ucomisd rightoff(%rbp), %xmm0
    //	jp	 . + 0x04
    //	je	 . + 0x0e
    //	mov	$0x3ff0000000000000, %rax
    //	jmp	. + 0x07
    //	mov	$0x0, %eax
    //	mov	%rax, destoff(%rbp)
    void not_equal_rbp(int leftoff, int rightoff, int destoff) {
        assert(leftoff <= 0);
        assert(rightoff <= 0);
        assert(destoff <= 0);
        push(0xf2);
        push(0x0f);
        push(0x10);
        push(0x85);
        push_int(leftoff);
        push(0x66);
        push(0x0f);
        push(0x2e);
        push(0x85);
        push_int(rightoff);
        push(0x7a);
        push(0x02);
        push(0x74);
        push(0x0c);
        push(0x48);
        push(0xb8);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0xf0);
        push(0x3f);
        push(0xeb);
        push(0x05);
        push(0xb8);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x00);
        push(0x48);
        push(0x89);
        push(0x85);
        push_int(destoff);
        m_source += "movsd  -" + fkxtoa(-leftoff, 8) + "(%rbp), %xmm0\n";
        m_source += "ucomisd -" + fkxtoa(-rightoff, 8) + "(%rbp), %xmm0\n";
        m_source += "jp	 . + 0x04\n";
        m_source += "je	 . + 0x0e\n";
        m_source += "mov	$0x3ff0000000000000, %rax\n";
        m_source += "jmp	. + 0x07\n";
        m_source += "mov	$0x0, %eax\n";
        m_source += "mov	%rax, -" + fkxtoa(-destoff, 8) + "(%rbp)\n";
    }

    // 取反运算
    // xorpd  %xmm0,%xmm0
    // ucomisd leftoff(%rbp),%xmm0
    // setne  %al
    // setp   %dl
    // or	 %edx,%eax
    // xor	$0x1,%eax
    // movzbl %al,%eax
    // cvtsi2sd %eax,%xmm0
    // movsd  %xmm0,destoff(%rbp)
    void not_rbp(int leftoff, int destoff) {
        assert(leftoff <= 0);
        assert(destoff <= 0);
        push(0x66);
        push(0x0f);
        push(0x57);
        push(0xc0);
        push(0x66);
        push(0x0f);
        push(0x2e);
        push(0x85);
        push_int(leftoff);
        push(0x0f);
        push(0x95);
        push(0xc0);
        push(0x0f);
        push(0x9a);
        push(0xc2);
        push(0x09);
        push(0xd0);
        push(0x83);
        push(0xf0);
        push(0x01);
        push(0x0f);
        push(0xb6);
        push(0xc0);
        push(0xf2);
        push(0x0f);
        push(0x2a);
        push(0xc0);
        push(0xf2);
        push(0x0f);
        push(0x11);
        push(0x85);
        push_int(destoff);
        m_source += "xorpd  %xmm0,%xmm0\n";
        m_source += "ucomisd -" + fkxtoa(-leftoff, 8) + "(%rbp),%xmm0\n";
        m_source += "setne  %al\n";
        m_source += "setp   %dl\n";
        m_source += "or	 %edx,%eax\n";
        m_source += "xor	$0x1,%eax\n";
        m_source += "movzbl %al,%eax\n";
        m_source += "cvtsi2sd %eax,%xmm0\n";
        m_source += "movsd  %xmm0,-" + fkxtoa(-destoff, 8) + "(%rbp)\n";
    }

    // offset的地方为0就jmp jumpoff
    // xorpd  %xmm0,%xmm0
    // ucomisd offset(%rbp),%xmm0
    // je	 .+jumpoff
    void jne_rbp(int offset, int jumpoff) {
        assert(offset <= 0);
        push(0x66);
        push(0x0f);
        push(0x57);
        push(0xc0);
        push(0x66);
        push(0x0f);
        push(0x2e);
        push(0x85);
        push_int(offset);
        push(0x0f);
        push(0x84);
        push_int(jumpoff);
        m_source += "xorpd  %xmm0,%xmm0\n";
        m_source += "ucomisd -" + fkxtoa(-offset, 8) + "(%rbp),%xmm0\n";
        m_source += "je	 .+" + fkxtoa(jumpoff, 8) + "\n";
    }

    // offset的地方不为0就jmp jumpoff
    // xorpd  %xmm0,%xmm0
    // ucomisd offset(%rbp),%xmm0
    // jne	 .+jumpoff
    void je_rbp(int offset, int jumpoff) {
        assert(offset <= 0);
        push(0x66);
        push(0x0f);
        push(0x57);
        push(0xc0);
        push(0x66);
        push(0x0f);
        push(0x2e);
        push(0x85);
        push_int(offset);
        push(0x0f);
        push(0x85);
        push_int(jumpoff);
        m_source += "xorpd  %xmm0,%xmm0\n";
        m_source += "ucomisd -" + fkxtoa(-offset, 8) + "(%rbp),%xmm0\n";
        m_source += "jne	 .+" + fkxtoa(jumpoff, 8) + "\n";
    }

    // 跳转jumpoff
    // jmp	 .+jumpoff
    void jmp(int jumpoff) {
        push(0xe9);
        push_int(jumpoff);
        m_source += "jmp	 .+" + fkxtoa(jumpoff, 8) + "\n";
    }

    // call rax
    void call_rax() {
        push(0xff);
        push(0xd0);
        m_source += "callq  *%rax\n";
    }

    // 取得offset的地址到rsi
    // lea	offset(%rbp),%rsi
    void lea_rbp_rsi(int offset) {
        push(0x48);
        push(0x8d);
        push(0xb5);
        push_int(offset);
        m_source += "lea	-" + fkxtoa(-offset, 8) + "(%rbp),%rsi\n";
    }

    // 取得offset的地址到rdx
    // lea	offset(%rbp),%rdx
    void lea_rbp_rdx(int offset) {
        push(0x48);
        push(0x8d);
        push(0x95);
        push_int(offset);
        m_source += "lea	-" + fkxtoa(-offset, 8) + "(%rbp),%rdx\n";
    }

    // 取得offset的地址到rcx
    // lea	offset(%rbp),%rcx
    void lea_rbp_rcx(int offset) {
        push(0x48);
        push(0x8d);
        push(0x8d);
        push_int(offset);
        m_source += "lea	-" + fkxtoa(-offset, 8) + "(%rbp),%rcx\n";
    }

    // 取得offset的地址到rdx
    // lea  offset(%rbp),%r8d
    void lea_rbp_r8(int offset) {
        push(0x4c);
        push(0x8d);
        push(0x85);
        push_int(offset);
        m_source += "lea	-" + fkxtoa(-offset, 8) + "(%rbp),%r8d\n";
    }

    // push参数rax:ps rdi:type rdx:data
    // mov	(%rax),%rbx
    // shl	$0x4,%rbx
    // mov	%edi,0x8(%rbx,%rax,1)
    // mov	(%rax),%rbx
    // shl	$0x4,%rbx
    // mov	%rdx,0x10(%rax,%rbx,1)
    // addq   $0x1,(%rax)
    void push_var() {
        push(0x48);
        push(0x8b);
        push(0x18);
        push(0x48);
        push(0xc1);
        push(0xe3);
        push(0x04);
        push(0x89);
        push(0x7c);
        push(0x03);
        push(0x08);
        push(0x48);
        push(0x8b);
        push(0x18);
        push(0x48);
        push(0xc1);
        push(0xe3);
        push(0x04);
        push(0x48);
        push(0x89);
        push(0x54);
        push(0x18);
        push(0x10);
        push(0x48);
        push(0x83);
        push(0x00);
        push(0x01);

        m_source += "mov	(%rax),%rbx\n";
        m_source += "shl	$0x4,%rbx\n";
        m_source += "mov	%edi,0x8(%rbx,%rax,1)\n";
        m_source += "mov	(%rax),%rbx\n";
        m_source += "shl	$0x4,%rbx\n";
        m_source += "mov	%rdx,0x10(%rax,%rbx,1)\n";
        m_source += "addq   $0x1,(%rax)\n";
    }

    // pop参数rax:ps rdi:type rdx:data
    // mov	(%rax),%rbx
    // shl	$0x4,%rbx
    // mov	-0x8(%rbx,%rax,1),%edi
    // mov	(%rax),%rbx
    // shl	$0x4,%rbx
    // mov	(%rax,%rbx,1),%rdx
    // subq   $0x1,(%rax)
    void pop_var() {
        push(0x48);
        push(0x8b);
        push(0x18);
        push(0x48);
        push(0xc1);
        push(0xe3);
        push(0x04);
        push(0x8b);
        push(0x7c);
        push(0x03);
        push(0xf8);
        push(0x48);
        push(0x8b);
        push(0x18);
        push(0x48);
        push(0xc1);
        push(0xe3);
        push(0x04);
        push(0x48);
        push(0x8b);
        push(0x14);
        push(0x18);
        push(0x48);
        push(0x83);
        push(0x28);
        push(0x01);

        m_source += "mov	(%rax),%rbx\n";
        m_source += "shl	$0x4,%rbx\n";
        m_source += "mov	-0x8(%rbx,%rax,1),%edi\n";
        m_source += "mov	(%rax),%rbx\n";
        m_source += "shl	$0x4,%rbx\n";
        m_source += "mov	(%rax,%rbx,1),%rdx	\n";
        m_source += "subq   $0x1,(%rax)\n";
    }

    // 把rax offset指向的赋给rdx
    // mov    offset(%rax),%rdx
    //
    void mov_raxv_rdx(int offset) {
        push(0x48);
        push(0x8b);
        push(0x90);
        push_int(offset);
        m_source += "mov   ";
        m_source += fkxtoa(offset, 8);
        m_source += "(%rax), %rdx\n";
    }

    // 把rdx赋给rax offset指向的
    // mov    %rdx,offset(%rax)
    //
    void mov_rdx_raxv(int offset) {
        push(0x48);
        push(0x89);
        push(0x90);
        push_int(offset);
        m_source += "mov   %rdx,";
        m_source += fkxtoa(offset, 8);
        m_source += "(%rax)\n";
    }

    // 返回
    // leaveq
    // retq
    void stop_func() {
        push(0xc9);
        push(0xc3);
        m_source += "leaveq\nretq\n";
    }

    void copy_param(size_t num);

    void copy_const(variant *p, size_t num, int start);

    void variant_assign(int leftpos, int rightpos);

    void variant_add(int destpos, int leftpos, int rightpos);

    void variant_sub(int destpos, int leftpos, int rightpos);

    void variant_mul(int destpos, int leftpos, int rightpos);

    void variant_div(int destpos, int leftpos, int rightpos);

    void variant_div_mod(int destpos, int leftpos, int rightpos);

    void variant_and(int destpos, int leftpos, int rightpos);

    void variant_or(int destpos, int leftpos, int rightpos);

    void variant_less(int destpos, int leftpos, int rightpos);

    void variant_more(int destpos, int leftpos, int rightpos);

    void variant_equal(int destpos, int leftpos, int rightpos);

    void variant_lessequal(int destpos, int leftpos, int rightpos);

    void variant_moreequal(int destpos, int leftpos, int rightpos);

    void variant_notequal(int destpos, int leftpos, int rightpos);

    void variant_not(int destpos, int leftpos);

    void variant_jne(int pos, int jmppos);

    void variant_je(int pos, int jmppos);

    void variant_jmp(int jmppos);

    void variant_push(int pos);

    void variant_pop(int pos);

    void variant_ps_clear();

    void call_func(void *func);

    void variant_from_rax(int destpos);

    void variant_to_rax(int srcpos);

    void call_func_param2(void *func, fake *fk, int pos2);

    void call_func_param3(void *func, fake *fk, int pos2, int pos3);

    void call_func_param4(void *func, fake *fk, int pos2, int pos3, int pos4);

    void call_func_param5(void *func, fake *fk, int pos2, int pos3, int pos4, int pos5);

    void output(const char *filename, const char *packagename, const char *name, func_native *nt);

    const String &source() const {
        return m_source;
    }

private:
    fake *m_fk;
    asm_code_list m_asm_code_list;
    String m_source;
};

