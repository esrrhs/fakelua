#include "assembler.h"
#include "semantic.h"
#include "myflexer.h"
#include "fuck.h"
#include "binary.h"
#include "asmgen.h"

void assembler::clear()
{
    m_pos = 0;
}

bool assembler::compile(binary * bin)
{
    FKLOG("[assembler] compile binary %p", bin);

    for (int i = 0; i < (int)bin->m_func_list.size(); i++)
    {
        const func_binary & fb = bin->m_func_list[i];
        if (!compile_func(fb))
        {
            FKERR("[assembler] compile compile_func %s fail", fb.getname().c_str());
            return false;
        }
    }
    
    String str = m_native->dump();
    FKLOG("[assembler] compile binary %d ok \n%s", bin, str.c_str());

    return true;
}

bool assembler::compile_func(const func_binary & fb)
{
    FKLOG("[assembler] compile_func func_binary %p", &fb);

    asmgen asg(m_fk);

    asg.start_func();

    int stacksize = (fb.m_maxstack + fb.m_const_list_num) * variant_size;
    FKLOG("[assembler] compile_func stack size %d", stacksize);
    asg.alloc_stack(stacksize);

    asg.copy_param(fb.paramnum());

    asg.copy_const(fb.m_const_list, fb.m_const_list_num, fb.m_maxstack);
   
    m_pos = 0;
    while (m_pos < (int)fb.m_size)
    {
        if (!compile_next(asg, fb))
        {
            FKERR("[assembler] compile_func compile_next %d fail", m_pos);
            return false;
        }
    }
    
    asg.stop_func();
    func_native nt(m_fk);
    asg.output(fb.getname(), &nt);
    m_native->add_func(nt);
    
    String str = asg.source();

    FKLOG("[assembler] compile_func binary %p ok\n%s", &fb, str.c_str());

    return true;
}

bool assembler::compile_next(asmgen & asg, const func_binary & fb)
{
    command cmd = GET_CMD(fb, m_pos);
    int type = COMMAND_TYPE(cmd);
    int code = COMMAND_CODE(cmd);

    FKLOG("[assembler] compile_next cmd %d %d %s", type, code, OpCodeStr(code));
        
    assert (type == COMMAND_OPCODE);

    m_pos++;

    bool ret = false;

    // 执行对应命令，放一起switch效率更高，cpu有缓存
    switch (code)
    {
    case OPCODE_ASSIGN:
        {
            ret = compile_assign(asg, fb);
        }
        break;
    default:
        assert(0);
        FKERR("[assembler] compile_next err code %d %s", code, OpCodeStr(code));
        break;
    }
    return true;
}

#define GET_VARIANT_POS(fb, v, pos) \
    command v##_cmd = GET_CMD(fb, pos);\
    assert (COMMAND_TYPE(v##_cmd) == COMMAND_ADDR);\
    int v##_addrtype = ADDR_TYPE(COMMAND_CODE(v##_cmd));\
    int v##_addrpos = ADDR_POS(COMMAND_CODE(v##_cmd));\
    assert (v##_addrtype == ADDR_STACK || v##_addrtype == ADDR_CONST);\
    if (v##_addrtype == ADDR_STACK)\
    {\
        v = (v##_addrpos);\
    }\
    else if (v##_addrtype == ADDR_CONST)\
    {\
		v = (v##_addrpos) + fb.maxstack(); \
    }\
    else\
    {\
		v = 0;\
        assert(0);\
        FKERR("next_assign assignaddrtype cannot be %d %d", v##_addrtype, v##_addrpos);\
        return false;\
    }


bool assembler::compile_assign(asmgen & asg, const func_binary & fb)
{
    assert (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_pos))) == ADDR_STACK);
    int var = 0;
    GET_VARIANT_POS(fb, var, m_pos);
    m_pos++;
    
    int value = 0;
    GET_VARIANT_POS(fb, value, m_pos);
    m_pos++;

    asg.variant_assign(var, value);

    FKLOG("assign from %d to pos %d", var, value);
    
    return true;
}

