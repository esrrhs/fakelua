#include "interpreter.h"
#include "fuck.h"
#include "binary.h"
#include "paramstack.h"

void stack::grow(int pos)
{
	if (pos < m_fk->m_stack_ini_size)
	{
		pos = m_fk->m_stack_ini_size;
	}
	assert(m_fk);
	// 新空间
	int newsize = pos + 1 + pos * m_fk->m_stack_grow_speed / 100;
	assert(newsize > (int)m_stack_variant_list_num);
	variant * newbuff = (variant *)m_fk->m_fkmalloc(newsize * sizeof(variant));
	
	// 复制
	if (m_stack_variant_list)
	{
		memcpy(newbuff, m_stack_variant_list, m_stack_variant_list_num * sizeof(variant));
	}

	// 构造剩余的
	for (int i = m_stack_variant_list_num; i <newsize; i++)
	{
		new (&newbuff[i]) variant(m_fk);
	}

	// 删除
	if (m_stack_variant_list)
	{
		m_fk->m_fkfree(m_stack_variant_list);
	}

	m_stack_variant_list = newbuff;
	m_stack_variant_list_num = newsize;
}

//////////////////////////////////////////////////////////////////////////

void interpreter::grow()
{
	assert(m_fk);
	// 新空间
	int newsize = m_stack_list_max_num + 1 + m_stack_list_max_num * m_fk->m_stack_list_grow_speed / 100;
	assert(newsize > (int)m_stack_list_max_num);
	stack * newbuff = (stack *)m_fk->m_fkmalloc(newsize * sizeof(stack));

	// 复制
	if (m_stack_list)
	{
		memcpy(newbuff, m_stack_list, m_stack_list_max_num * sizeof(stack));
	}

	// 构造剩余的
	for (int i = m_stack_list_max_num; i < newsize; i++)
	{
		new (&newbuff[i]) stack();
	}

	// 删除
	if (m_stack_list)
	{
		m_fk->m_fkfree(m_stack_list);
	}

	m_stack_list = newbuff;
	m_stack_list_max_num = newsize;
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
	if (m_stack_list_num >= m_stack_list_max_num)
	{
		grow();
	}
	m_stack_list_num++;
	stack & s = m_stack_list[m_stack_list_num - 1];
	s.m_fk = m_fk;
	s.m_ei = m_ei;
	s.m_fb = fb;
	s.clear();

    // 分配栈空间
    for (int i = 0; i < (int)ps->size(); i++)
    {
		s.set_stack_variant((*ps)[i], i);
		FKLOG("call set %s to pos %d", ((String)((*ps)[i])).c_str(), i);
    }
}

#define GET_CMD(fb, pos) fb.m_buff[pos]

#define GET_CONST(v, fb, pos) \
    assert(pos >= 0 && pos < (int)fb.m_const_list_num);\
    v = &fb.m_const_list[pos];

#define GET_STACK(v, s, pos) \
	if (pos >= (int)s.m_stack_variant_list_num)\
	{\
		s.grow(pos);\
	}\
    v = &s.m_stack_variant_list[pos];

bool interpreter::next()
{
	stack & s = m_stack_list[m_stack_list_num - 1];
    const func_binary & fb = *s.m_fb;
    int pos = s.m_pos;

    command cmd = GET_CMD(fb, pos);
    int type = COMMAND_TYPE(cmd);
    int code = COMMAND_CODE(cmd);

    FKLOG("next %d %d %s", type, code, OpCodeStr(code));
        
    assert (type == COMMAND_OPCODE);

    s.m_pos++;

    bool ret = false;
    switch (code)
    {
    case OPCODE_ASSIGN:
        ret = next_assign(s, fb, code);
        break;
    case OPCODE_PLUS:
    case OPCODE_MINUS:
    case OPCODE_MULTIPLY:
    case OPCODE_DIVIDE:
    case OPCODE_DIVIDE_MOD:
    case OPCODE_AND:
    case OPCODE_OR:
    case OPCODE_LESS:
	case OPCODE_MORE:
	case OPCODE_EQUAL:
	case OPCODE_MOREEQUAL:
	case OPCODE_LESSEQUAL:
	case OPCODE_NOTEQUAL:
        ret = next_math(s, fb, code);
		break;
	case OPCODE_RETURN:
		ret = next_return(s, fb, code);
		break;
	case OPCODE_JNE:
		ret = next_jne(s, fb, code);
		break;
	case OPCODE_JMP:
		ret = next_jmp(s, fb, code);
		break;
    default:
        assert(0);
        FKERR("next err code %d %s", code, OpCodeStr(code));
        break;
    }

    // 当前函数走完
    if (s.m_pos >= (int)fb.cmdsize())
    {
        FKLOG("pop stack %s", s.m_fb->getname().c_str());
        // 出栈
		s.clear();
		m_stack_list_num--;
    }

    // 所有都完
	if (!m_stack_list_num)
    {
        FKLOG("stack empty end");
        m_isend = true;
    }

    return ret;
}

#define GET_VARIANT(s, fb, v, pos) \
    command v##_cmd = GET_CMD(fb, pos);\
    assert (COMMAND_TYPE(v##_cmd) == COMMAND_ADDR);\
    int v##_addrtype = ADDR_TYPE(COMMAND_CODE(v##_cmd));\
    int v##_addrpos = ADDR_POS(COMMAND_CODE(v##_cmd));\
    assert (v##_addrtype == ADDR_STACK || v##_addrtype == ADDR_CONST);\
    if (v##_addrtype == ADDR_STACK)\
    {\
        GET_STACK(v, s, (v##_addrpos));\
    }\
    else if (v##_addrtype == ADDR_CONST)\
    {\
		GET_CONST(v, fb, (v##_addrpos)); \
    }\
    else\
    {\
		v = 0;\
        assert(0);\
        FKERR("next_assign assignaddrtype cannot be %d %d", v##_addrtype, v##_addrpos);\
        return false;\
    }

#define LOG_VARIANT(s, fb, pos, prefix) \
    FKLOG(prefix " variant %d %d", \
        ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, pos))),\
        ADDR_POS(COMMAND_CODE(GET_CMD(fb, pos))));

bool interpreter::next_assign(stack & s, const func_binary & fb, int code)
{
    // 赋值dest，必须为栈上
    assert (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, s.m_pos))) == ADDR_STACK);
    LOG_VARIANT(s, fb, s.m_pos, "assign");
    int addrpos = ADDR_POS(COMMAND_CODE(GET_CMD(fb, s.m_pos)));
    s.m_pos++;

    // 赋值来源
    const variant * valuev = 0;
    LOG_VARIANT(s, fb, s.m_pos, "value");
    GET_VARIANT(s, fb, valuev, s.m_pos);
    s.m_pos++;

    // 赋值
    s.set_stack_variant(*valuev, addrpos);

	FKLOG("assign %s to pos %d", ((String)*valuev).c_str(), addrpos);
    
    return true;
}

bool interpreter::next_math(stack & s, const func_binary & fb, int code)
{
	const variant * left = 0;
	LOG_VARIANT(s, fb, s.m_pos, "left");
    GET_VARIANT(s, fb, left, s.m_pos);
    s.m_pos++;
    
	const variant * right = 0;
	LOG_VARIANT(s, fb, s.m_pos, "right");
    GET_VARIANT(s, fb, right, s.m_pos);
    s.m_pos++;
    
	variant * dest;
    assert (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, s.m_pos))) == ADDR_STACK);
    LOG_VARIANT(s, fb, s.m_pos, "dest");
	GET_VARIANT(s, fb, dest, s.m_pos);
    s.m_pos++;

	FKLOG("math left %s right %s", ((String)*left).c_str(), ((String)*right).c_str());

    switch (code)
    {
    case OPCODE_PLUS:
		dest->plus(*left, *right);
        break;
    case OPCODE_MINUS:
		dest->minus(*left, *right);
        break;
    case OPCODE_MULTIPLY:
		dest->multiply(*left, *right);
        break;
    case OPCODE_DIVIDE:
		dest->divide(*left, *right);
        break;
    case OPCODE_DIVIDE_MOD:
		dest->divide_mode(*left, *right);
        break;
    case OPCODE_AND:
		dest->band(*left, *right);
        break;
    case OPCODE_OR:
		dest->bor(*left, *right);
        break;
    case OPCODE_LESS:
		dest->less(*left, *right);
        break;
	case OPCODE_MORE:
		dest->more(*left, *right);
        break;
	case OPCODE_EQUAL:
		dest->equal(*left, *right);
        break;
	case OPCODE_MOREEQUAL:
		dest->moreequal(*left, *right);
        break;
	case OPCODE_LESSEQUAL:
		dest->lessequal(*left, *right);
        break;
	case OPCODE_NOTEQUAL:
		dest->notequal(*left, *right);
        break;
    default:
        assert(0);
        FKERR("math err code %d %s", code, OpCodeStr(code));
        break;
    }

	FKLOG("math %s %s", OpCodeStr(code), ((String)*dest).c_str());
    
    return true;
}

bool interpreter::next_return(stack & s, const func_binary & fb, int code)
{
	if (GET_CMD(fb, s.m_pos) == EMPTY_CMD)
	{
		FKLOG("return empty");
		return true;
	}

	// 塞给ret
	const variant * ret = 0;
	LOG_VARIANT(s, fb, s.m_pos, "ret");
	GET_VARIANT(s, fb, ret, s.m_pos);
	s.m_pos++;

	m_ret = *ret;

	FKLOG("return %s", ((String)m_ret).c_str());

	return true;
}

bool interpreter::next_jne(stack & s, const func_binary & fb, int code)
{
	const variant * cmp = 0;
	LOG_VARIANT(s, fb, s.m_pos, "cmp");
	GET_VARIANT(s, fb, cmp, s.m_pos);
	s.m_pos++;

    int pos = COMMAND_CODE(GET_CMD(fb, s.m_pos));
	s.m_pos++;
	
    if (!((bool)(*cmp)))
    {
	    FKLOG("jne %d", pos);
        
        s.m_pos = pos;
    }
    else
    {
	    FKLOG("not jne %d", pos);
    }
    
	return true;
}

bool interpreter::next_jmp(stack & s, const func_binary & fb, int code)
{
    int pos = COMMAND_CODE(GET_CMD(fb, s.m_pos));
	s.m_pos++;
	
	FKLOG("jmp %d", pos);

    s.m_pos = pos;
	
	return true;
}

