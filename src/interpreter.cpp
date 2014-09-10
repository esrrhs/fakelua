#include "interpreter.h"
#include "fuck.h"
#include "binary.h"
#include "paramstack.h"

void interpreter::clear()
{
    m_isend = false;
    m_ret = variant(m_fk);
    m_stack_list.clear();
}

bool interpreter::isend() const
{
    return m_isend;
}

const variant & interpreter::getret() const
{
    return m_ret;
}

void interpreter::call(binary * bin, const String & func, paramstack * ps)
{
    const func_binary * fb = bin->get_func(func);
    if (!fb)
    {
        FKERR("fkrun bin %p no func %s fail", bin, func.c_str());
        m_fk->seterror(m_ei, efk_run_no_func_error, "fkrun bin %p no func %s fail", bin, func.c_str());
        m_isend = true;
        return;
    }

    // 压栈
    m_stack_list.push_back(stack(m_fk, m_ei, fb));
    stack & s = m_stack_list.back();

    // 分配栈空间
    for (int i = 0; i < (int)ps->size(); i++)
    {
        s.set_stack_variant((*ps)[i], i);
    }
}

int interpreter::run(int cmdnum)
{
    int num = 0;
    for (int i = 0; i < cmdnum; i++)
    {
        if (!next())
        {
            // 发生错误
            m_isend = true;
        }
        num++;
        if (isend())
        {
            break;
        }
    }

    return num;
}

bool interpreter::next()
{
    stack & s = m_stack_list.back();
    const func_binary & fb = *s.m_fb;
    int pos = s.m_pos;

    command cmd = fb.getcmd(pos);
    int type = COMMAND_TYPE(cmd);
    int code = COMMAND_CODE(cmd);

    FKLOG("next %d %d %s", type, code, OpCodeStr(code));
        
    assert (type == COMMAND_OPCODE);

    s.m_pos++;

    bool ret = false;
    switch (code)
    {
    case OPCODE_ASSIGN:
        ret = next_assign(s, fb);
        break;
    }

    // 当前函数走完
    if (s.m_pos >= (int)fb.cmdsize())
    {
        FKLOG("pop stack %s", s.m_fb->getname().c_str());
        // 出栈
        m_stack_list.pop_back();
    }

    // 所有都完
    if (m_stack_list.empty())
    {
        FKLOG("stack empty end");
        m_isend = true;
    }

    return ret;
}

#define GET_VARIANT(s, fb, v, pos) \
    command v##_cmd = fb.getcmd(pos);\
    assert (COMMAND_TYPE(v##_cmd) == COMMAND_ADDR);\
    int v##_addrtype = ADDR_TYPE(COMMAND_CODE(v##_cmd));\
    int v##_addrpos = ADDR_POS(COMMAND_CODE(v##_cmd));\
    assert (v##_addrtype == ADDR_STACK || v##_addrtype == ADDR_CONST);\
    if (v##_addrtype == ADDR_STACK)\
    {\
        s.get_stack_variant(v, v##_addrpos);\
    }\
    else if (v##_addrtype == ADDR_CONST)\
    {\
        fb.getconst(v, v##_addrpos);\
    }\
    else\
    {\
        assert(0);\
        FKERR("next_assign assignaddrtype cannot be %d %d", v##_addrtype, v##_addrpos);\
        return false;\
    }

#define LOG_VARIANT(s, fb, pos, prefix) \
    FKLOG(prefix " variant %d %d", \
        ADDR_TYPE(COMMAND_CODE(fb.getcmd(pos))),\
        ADDR_POS(COMMAND_CODE(fb.getcmd(pos))));

bool interpreter::next_assign(stack & s, const func_binary & fb)
{
    // 赋值dest，必须为栈上
    assert (ADDR_TYPE(COMMAND_CODE(fb.getcmd(s.m_pos))) == ADDR_STACK);
    LOG_VARIANT(s, fb, s.m_pos, "assign");
    int addrpos = ADDR_POS(COMMAND_CODE(fb.getcmd(s.m_pos)));
    s.m_pos++;

    // 赋值来源
    variant valuev(m_fk);
    LOG_VARIANT(s, fb, s.m_pos, "value");
    GET_VARIANT(s, fb, valuev, s.m_pos);
    s.m_pos++;

    // 赋值
    s.set_stack_variant(valuev, addrpos);

    FKLOG("assign %s to pos %d", ((String)valuev).c_str(), addrpos);
    
    return true;
}

