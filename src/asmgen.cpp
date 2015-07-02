#include "asmgen.h"
#include "semantic.h"
#include "myflexer.h"
#include "fake.h"
#include "variant.h"
#ifdef WIN32
#include <Windows.h>
#else  
#include <sys/mman.h>
#endif

void asmgen::output(const char * filename, const char * packagename, const char * name, func_native * nt)
{
    nt->m_name = stringdump(m_fk, name, strlen(name));
    nt->m_filename = stringdump(m_fk, filename, strlen(filename));
    nt->m_packagename = stringdump(m_fk, packagename, strlen(packagename));

    nt->m_size = m_asm_code_list.size();
    
#ifdef WIN32
    nt->m_buff = (char*)VirtualAlloc(0, nt->m_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else  
    nt->m_buff = (char*)mmap(0, nt->m_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif

    memcpy(nt->m_buff, &m_asm_code_list[0], nt->m_size);
    
#ifdef WIN32
    DWORD dwOld; 
    VirtualProtect(nt->m_buff, nt->m_size, PAGE_EXECUTE_READ, &dwOld);
#else  
    mprotect(nt->m_buff, nt->m_size, PROT_READ | PROT_EXEC);
#endif

}

void asmgen::copy_param(size_t num)
{
}

void asmgen::copy_const(variant * p, size_t num, int start)
{
    for (int i = 0; i < (int)num; i++)
    {
        int typeoff = V_TYPE_OFF(start + i);
		int dataoff = V_DATA_OFF(start + i);
        mov_l_rbp(p[i].type, typeoff);
		mov_ll_rax(p[i].data.buf);
        mov_rax_rbp(dataoff);
    }
}

void asmgen::variant_assign(int leftpos, int rightpos)
{
    int lefttypeoff = V_TYPE_OFF(leftpos);
    int leftdataoff = V_DATA_OFF(leftpos);
    int rightpostypeoff = V_TYPE_OFF(rightpos);
    int rightposdataoff = V_DATA_OFF(rightpos);
    mov_rbp_rax(rightpostypeoff);
    mov_rax_rbp(lefttypeoff);
    mov_rbp_rax(rightposdataoff);
    mov_rax_rbp(leftdataoff);
}

void asmgen::variant_add(int destpos, int leftpos, int rightpos)
{
    int leftdataoff = V_DATA_OFF(leftpos);
    int rightdataoff = V_DATA_OFF(rightpos);
    int destdataoff = V_DATA_OFF(destpos);
    int desttypeoff = V_TYPE_OFF(destpos);
    movsd_rbp_xmm0(leftdataoff);
    addsd_rbp_xmm0(rightdataoff);
    movsd_xmm0_rbp(destdataoff);
    mov_l_rbp(variant::REAL, desttypeoff);
}

void asmgen::variant_sub(int destpos, int leftpos, int rightpos)
{
    int leftdataoff = V_DATA_OFF(leftpos);
    int rightdataoff = V_DATA_OFF(rightpos);
    int destdataoff = V_DATA_OFF(destpos);
    int desttypeoff = V_TYPE_OFF(destpos);
    movsd_rbp_xmm0(leftdataoff);
    subsd_rbp_xmm0(rightdataoff);
    movsd_xmm0_rbp(destdataoff);
    mov_l_rbp(variant::REAL, desttypeoff);
}

void asmgen::variant_mul(int destpos, int leftpos, int rightpos)
{
    int leftdataoff = V_DATA_OFF(leftpos);
    int rightdataoff = V_DATA_OFF(rightpos);
    int destdataoff = V_DATA_OFF(destpos);
    int desttypeoff = V_TYPE_OFF(destpos);
    movsd_rbp_xmm0(leftdataoff);
    mulsd_rbp_xmm0(rightdataoff);
    movsd_xmm0_rbp(destdataoff);
    mov_l_rbp(variant::REAL, desttypeoff);
}

void asmgen::variant_div(int destpos, int leftpos, int rightpos)
{
    int leftdataoff = V_DATA_OFF(leftpos);
    int rightdataoff = V_DATA_OFF(rightpos);
    int destdataoff = V_DATA_OFF(destpos);
    int desttypeoff = V_TYPE_OFF(destpos);
    movsd_rbp_xmm0(leftdataoff);
    divsd_rbp_xmm0(rightdataoff);
    movsd_xmm0_rbp(destdataoff);
    mov_l_rbp(variant::REAL, desttypeoff);
}

void asmgen::variant_div_mod(int destpos, int leftpos, int rightpos)
{
	int leftdataoff = V_DATA_OFF(leftpos);
	int rightdataoff = V_DATA_OFF(rightpos);
	int destdataoff = V_DATA_OFF(destpos);
    int desttypeoff = V_TYPE_OFF(destpos);
	divide_mod_rbp(leftdataoff, rightdataoff, destdataoff);
    mov_l_rbp(variant::REAL, desttypeoff);
}

void asmgen::variant_and(int destpos, int leftpos, int rightpos)
{
	int leftdataoff = V_DATA_OFF(leftpos);
	int rightdataoff = V_DATA_OFF(rightpos);
	int destdataoff = V_DATA_OFF(destpos);
    int desttypeoff = V_TYPE_OFF(destpos);
	and_rbp(leftdataoff, rightdataoff, destdataoff);
    mov_l_rbp(variant::REAL, desttypeoff);
}

void asmgen::variant_or(int destpos, int leftpos, int rightpos)
{
	int leftdataoff = V_DATA_OFF(leftpos);
	int rightdataoff = V_DATA_OFF(rightpos);
	int destdataoff = V_DATA_OFF(destpos);
    int desttypeoff = V_TYPE_OFF(destpos);
	or_rbp(leftdataoff, rightdataoff, destdataoff);
    mov_l_rbp(variant::REAL, desttypeoff);
}

void asmgen::variant_less(int destpos, int leftpos, int rightpos)
{
	int leftdataoff = V_DATA_OFF(leftpos);
	int rightdataoff = V_DATA_OFF(rightpos);
	int destdataoff = V_DATA_OFF(destpos);
    int desttypeoff = V_TYPE_OFF(destpos);
	less_rbp(leftdataoff, rightdataoff, destdataoff);
    mov_l_rbp(variant::REAL, desttypeoff);
}

void asmgen::variant_more(int destpos, int leftpos, int rightpos)
{
	int leftdataoff = V_DATA_OFF(leftpos);
	int rightdataoff = V_DATA_OFF(rightpos);
	int destdataoff = V_DATA_OFF(destpos);
    int desttypeoff = V_TYPE_OFF(destpos);
	more_rbp(leftdataoff, rightdataoff, destdataoff);
    mov_l_rbp(variant::REAL, desttypeoff);
}

void asmgen::variant_equal(int destpos, int leftpos, int rightpos)
{
	int leftdataoff = V_DATA_OFF(leftpos);
	int rightdataoff = V_DATA_OFF(rightpos);
	int destdataoff = V_DATA_OFF(destpos);
    int desttypeoff = V_TYPE_OFF(destpos);
	equal_rbp(leftdataoff, rightdataoff, destdataoff);
    mov_l_rbp(variant::REAL, desttypeoff);
}

void asmgen::variant_lessequal(int destpos, int leftpos, int rightpos)
{
	int leftdataoff = V_DATA_OFF(leftpos);
	int rightdataoff = V_DATA_OFF(rightpos);
	int destdataoff = V_DATA_OFF(destpos);
    int desttypeoff = V_TYPE_OFF(destpos);
	less_equal_rbp(leftdataoff, rightdataoff, destdataoff);
    mov_l_rbp(variant::REAL, desttypeoff);
}

void asmgen::variant_moreequal(int destpos, int leftpos, int rightpos)
{
	int leftdataoff = V_DATA_OFF(leftpos);
	int rightdataoff = V_DATA_OFF(rightpos);
	int destdataoff = V_DATA_OFF(destpos);
    int desttypeoff = V_TYPE_OFF(destpos);
	more_equal_rbp(leftdataoff, rightdataoff, destdataoff);
    mov_l_rbp(variant::REAL, desttypeoff);
}

void asmgen::variant_notequal(int destpos, int leftpos, int rightpos)
{
	int leftdataoff = V_DATA_OFF(leftpos);
	int rightdataoff = V_DATA_OFF(rightpos);
	int destdataoff = V_DATA_OFF(destpos);
    int desttypeoff = V_TYPE_OFF(destpos);
	not_equal_rbp(leftdataoff, rightdataoff, destdataoff);
    mov_l_rbp(variant::REAL, desttypeoff);
}

void asmgen::variant_not(int destpos, int leftpos)
{
	int leftdataoff = V_DATA_OFF(leftpos);
	int destdataoff = V_DATA_OFF(destpos);
    int desttypeoff = V_TYPE_OFF(destpos);
	not_rbp(leftdataoff, destdataoff);
    mov_l_rbp(variant::REAL, desttypeoff);
}

void asmgen::variant_jne(int pos, int jmppos)
{
	int dataoff = V_DATA_OFF(pos);
	jne_rbp(dataoff, jmppos);
}

void asmgen::variant_jmp(int jmppos)
{
	jmp(jmppos);
}

void asmgen::variant_push(int pos)
{
	int dataoff = V_DATA_OFF(pos);
    int typeoff = V_TYPE_OFF(pos);
    mov_ll_rax((int64_t)&m_fk->ps);
    mov_rbp_rdx(dataoff);
    mov_rbp_rdi(typeoff);
    push_var();
}

void asmgen::variant_pop(int pos)
{
	int dataoff = V_DATA_OFF(pos);
    int typeoff = V_TYPE_OFF(pos);
    mov_ll_rax((int64_t)&m_fk->ps);
    pop_var();
    mov_rdx_rbp(dataoff);
    mov_rdi_rbp(typeoff);
}

void asmgen::variant_ps_clear()
{
    mov_ll_rax((int64_t)&m_fk->ps);
	movq_p_rax(0);
}

void asmgen::call_func(void * func)
{
	mov_ll_rax((int64_t)func);
	call_rax();
}

