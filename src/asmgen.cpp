#include "asmgen.h"
#include "semantic.h"
#include "myflexer.h"
#include "fuck.h"
#include "variant.h"
#ifdef WIN32
#include <Windows.h>
#else  
#include <sys/mman.h>
#endif

void asmgen::output(const String & name, func_native * nt)
{
    nt->m_name = name;

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
    for (int i = 0; i < (int)num; i++)
    {
        int typeoff = V_TYPE_OFF(i);
        int dataoff = V_DATA_OFF(i);
        mov_rax_rbp(typeoff);
        pop_rax();
        mov_rbp_rax(dataoff);
        pop_rdx();
    }
}

void asmgen::copy_const(variant * p, size_t num, int start)
{
    for (int i = 0; i < (int)num; i++)
    {
        int typeoff = V_TYPE_OFF(start);
        int dataoff = V_DATA_OFF(start);
        mov_l_rbp(p[i].type, typeoff);
        mov_ll_rax(*(int64_t*)&p[i].data);
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

