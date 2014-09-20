#include "machine.h"
#include "fuck.h"
#include "binary.h"
#include "paramstack.h"

void machine::call(native * nt, const char * func, paramstack * ps)
{
    const func_native * fn = nt->get_func(func);
    if (!fn)
    {
        FKERR("fkrun native %p no func %s fail", nt, func);
        seterror(m_fk, efk_run_no_func_error, "fkrun native %p no func %s fail", nt, func);
        m_isend = true;
        return;
    }

    // push参数
    for (int i = (int)ps->m_variant_list_num - 1; i >= 0; i--)
    {
        int type = ps->m_variant_list[i].type;
        double data = ps->m_variant_list[i].data.real;
        asm(
            "mov    %0,%%eax    \n\t"  
            "pushq  %%rax        \n\t"      
            "movq   %1,%%rdx    \n\t"
            "pushq  %%rdx        \n\t"
            :
            :"r"(type),"r"(data)
            :"%rax","rdi"
            );
    }

    // call
    typedef void (*macfunc) ();
    macfunc f = (macfunc)fn->m_buff;
    f();

    // 获得返回值
    asm(
        "mov    %%eax,%0    \n\t"  
        "popq   %%rax       \n\t"      
        "movq   %%rdx,%1    \n\t"
        "popq   %%rdx       \n\t"
        :"=r"(m_ret.type),"=r"(m_ret.data.real)
        :
        :"%rax","rdi"
        );
    
}

