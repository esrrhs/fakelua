#include "assembler.h"
#include "semantic.h"
#include "myflexer.h"
#include "fuck.h"
#include "binary.h"
#include "asmgen.h"

void assembler::clear()
{
}

bool assembler::compile(binary * bin)
{
    FKLOG("[assembler] compile binary %p", bin);

    for (int i = 0; i < (int)bin->m_func_list.size(); i++)
    {
        func_binary & fb = bin->m_func_list[i];
        if (!compile_func(&fb))
        {
            FKERR("[assembler] compile compile_func %s fail", fb.getname().c_str());
            return false;
        }
    }
    
    String str = m_native->dump();
    FKLOG("[assembler] compile binary %d ok \n%s", bin, str.c_str());

    return true;
}

bool assembler::compile_func(func_binary * fb)
{
    FKLOG("[assembler] compile_func func_binary %p", fb);

    asmgen asg(m_fk);

    asg.start_func();
    
    asg.alloc_stack((fb->m_maxstack + fb->m_const_list_num) * variant_size);

    asg.copy_const(fb->m_const_list, fb->m_const_list_num);

    for (int i = 0; i < (int)fb->m_size; i++)
    {
        command cmd = fb->m_buff[i];
        if (!compile_command(cmd))
        {
            FKERR("[assembler] compile_func compile_command %s fail", fkxtoa(cmd).c_str());
            return false;
        }
    }
    
    asg.stop_func();
    func_native nt(m_fk);
    asg.output(fb->getname(), &nt);
    m_native->add_func(nt);
    
    FKLOG("[assembler] compile_func binary %p ok", fb);

    return true;
}

bool assembler::compile_command(command cmd)
{
    
    return true;
}

