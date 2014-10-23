#pragma once

#include "types.h"
#include "variant.h"
#include "binary.h"
#include "paramstack.h"

struct fuck;

#define GET_CMD(fb, pos) fb.m_buff[pos]

#define GET_CONST(v, fb, pos) \
    assert(pos >= 0 && pos < (int)fb.m_const_list_num);\
    v = &fb.m_const_list[pos];

#define GET_STACK(v, s, pos) \
	assert(pos >= 0 && pos < (int)(s).m_stack_variant_list_num);\
    v = &(s).m_stack_variant_list[pos];

#define SET_STACK(v, s, pos) \
	assert(pos >= 0 && pos < (int)(s).m_stack_variant_list_num);\
    (s).m_stack_variant_list[pos] = *v;
    
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
        err = true;\
        break;\
    }

#define LOG_VARIANT(s, fb, pos, prefix) \
    FKLOG(prefix " variant %d %d", \
        ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, pos))),\
        ADDR_POS(COMMAND_CODE(GET_CMD(fb, pos))));

#define MATH_OPER(s, fb, oper) \
	const variant * left = 0;\
	LOG_VARIANT(s, fb, (s).m_pos, "left");\
    GET_VARIANT(s, fb, left, (s).m_pos);\
    (s).m_pos++;\
    \
	const variant * right = 0;\
	LOG_VARIANT(s, fb, (s).m_pos, "right");\
    GET_VARIANT(s, fb, right, (s).m_pos);\
    (s).m_pos++;\
    \
	variant * dest;\
    assert (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, (s).m_pos))) == ADDR_STACK);\
    LOG_VARIANT(s, fb, (s).m_pos, "dest");\
	GET_VARIANT(s, fb, dest, (s).m_pos);\
    (s).m_pos++;\
    \
	FKLOG("math left %s right %s", (vartostring(left)).c_str(), (vartostring(right)).c_str());\
    \
    V_##oper(dest, left, right);\
    \
    FKLOG("math %s %s", OpCodeStr(code), (vartostring(dest)).c_str());
 
#define MATH_ASSIGN_OPER(s, fb, oper) \
	variant * var = 0;\
    assert (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, (s).m_pos))) == ADDR_STACK);\
	LOG_VARIANT(s, fb, (s).m_pos, "var");\
    GET_VARIANT(s, fb, var, (s).m_pos);\
    (s).m_pos++;\
    \
	const variant * value = 0;\
	LOG_VARIANT(s, fb, (s).m_pos, "value");\
    GET_VARIANT(s, fb, value, (s).m_pos);\
    (s).m_pos++;\
    \
	FKLOG("math var %s value %s", (vartostring(var)).c_str(), (vartostring(value)).c_str());\
    \
    V_##oper(var, var, value);\
    \
    FKLOG("math %s %s", OpCodeStr(code), (vartostring(var)).c_str());

struct stack
{
	force_inline stack() : m_fk(0), m_fb(0), m_pos(0), m_stack_variant_list(0), m_stack_variant_list_num(0)
    {
    }
	force_inline stack(fuck * fk, const  func_binary * fb) : m_fk(fk), m_fb(fb), m_pos(0), m_stack_variant_list(0), m_stack_variant_list_num(0)
    {
    }
	force_inline ~stack()
    {
        safe_fkfree(m_fk, m_stack_variant_list);
    }

	void grow(size_t size);

	force_inline void clear()
    {
    	m_pos = 0;
    	// 为了效率，保留脏数据
    }

    fuck * m_fk;
    // 函数二进制
    const func_binary * m_fb;
    // 当前执行位置
    int m_pos;
    // 当前栈上的变量
	variant * m_stack_variant_list;
	size_t m_stack_variant_list_num;
};

class interpreter
{
public:
	force_inline interpreter(fuck * fk) : m_fk(fk), m_isend(false), m_retvpos(-1), m_cur_stack(0),
		m_stack_list(0), m_stack_list_num(0), m_stack_list_max_num(0)
    {
    }
    force_inline ~interpreter()
    {
    	if (m_stack_list)
    	{
    		assert(m_fk);
    		for (int i = 0; i < (int)m_stack_list_max_num; i++)
    		{
    			m_stack_list[i].~stack();
    		}
    		safe_fkfree(m_fk, m_stack_list);
    	}
    }

    void clear()
    {
        m_isend = false;
        m_retvpos = -1;
        m_cur_stack = 0;
        m_stack_list_num = 0;
        for (int i = 0; i < (int)m_stack_list_max_num; i++)
		{
			m_stack_list[i].clear();
		}
    }
    
	void grow();
    
    bool isend() const
    {
        return m_isend;
    }
    
    void call(binary * bin, const char * func, paramstack * ps);

    force_inline const variant & getret() const
    {
        return m_ret;
    }

    force_inline int run(int cmdnum)
    {
        int num = 0;
        for (int i = 0; i < cmdnum; i++)
        {
            bool err = false;

            // next
            const func_binary & fb = *m_cur_stack->m_fb;
            int pos = m_cur_stack->m_pos;

            command cmd = GET_CMD(fb, pos);
            int type = COMMAND_TYPE(cmd);
            int code = COMMAND_CODE(cmd);

            FKLOG("next %d %d %s", type, code, OpCodeStr(code));
                
            assert (type == COMMAND_OPCODE);

            m_cur_stack->m_pos++;

            // 执行对应命令，放一起switch效率更高，cpu有缓存
            switch (code)
            {
            case OPCODE_ASSIGN:
                {
                    // 赋值dest，必须为栈上
                    assert (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos))) == ADDR_STACK);
                    LOG_VARIANT(*m_cur_stack, fb, m_cur_stack->m_pos, "assign");
                    int addrpos = ADDR_POS(COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos)));
                    m_cur_stack->m_pos++;

                    // 赋值来源
                    const variant * valuev = 0;
                    LOG_VARIANT(*m_cur_stack, fb, m_cur_stack->m_pos, "value");
                    GET_VARIANT(*m_cur_stack, fb, valuev, m_cur_stack->m_pos);
                    m_cur_stack->m_pos++;

                    // 赋值
                    SET_STACK(valuev, *m_cur_stack, addrpos);

                	FKLOG("assign %s to pos %d", (vartostring(valuev)).c_str(), addrpos);
                }
                break;
            case OPCODE_PLUS:
                {
        		    MATH_OPER(*m_cur_stack, fb, PLUS);
        		}
                break;
            case OPCODE_MINUS:
                {
        		    MATH_OPER(*m_cur_stack, fb, MINUS);
        		}
                break;
            case OPCODE_MULTIPLY:
        		{
        		    MATH_OPER(*m_cur_stack, fb, MULTIPLY);
        		}
                break;
            case OPCODE_DIVIDE:
        		{
        		    MATH_OPER(*m_cur_stack, fb, DIVIDE);
        		}
                break;
            case OPCODE_DIVIDE_MOD:
        		{
        		    MATH_OPER(*m_cur_stack, fb, DIVIDE_MOD);
        		}
                break;
            case OPCODE_AND:
        		{
        		    MATH_OPER(*m_cur_stack, fb, AND);
        		}
                break;
            case OPCODE_OR:
        		{
        		    MATH_OPER(*m_cur_stack, fb, OR);
        		}
                break;
            case OPCODE_LESS:
        		{
        		    MATH_OPER(*m_cur_stack, fb, LESS);
        		}
                break;
        	case OPCODE_MORE:
        		{
        		    MATH_OPER(*m_cur_stack, fb, MORE);
        		}
                break;
        	case OPCODE_EQUAL:
        		{
        		    MATH_OPER(*m_cur_stack, fb, EQUAL);
        		}
                break;
        	case OPCODE_MOREEQUAL:
        		{
        		    MATH_OPER(*m_cur_stack, fb, MOREEQUAL);
        		}
                break;
        	case OPCODE_LESSEQUAL:
        		{
        		    MATH_OPER(*m_cur_stack, fb, LESSEQUAL);
        		}
                break;
        	case OPCODE_NOTEQUAL:
        		{
        		    MATH_OPER(*m_cur_stack, fb, NOTEQUAL);
        		}
                break;
        	case OPCODE_RETURN:
        	    {
                	if (GET_CMD(fb, m_cur_stack->m_pos) == EMPTY_CMD)
                	{
                		FKLOG("return empty");
		                m_cur_stack->m_pos = fb.m_size;
                		break;
                	}

                	// 塞给ret
                	const variant * ret = 0;
                	LOG_VARIANT(*m_cur_stack, fb, m_cur_stack->m_pos, "ret");
                	GET_VARIANT(*m_cur_stack, fb, ret, m_cur_stack->m_pos);
                	m_cur_stack->m_pos++;

                	m_ret = *ret;

                	FKLOG("return %s", (vartostring(&m_ret)).c_str());

        	    }
        		break;
        	case OPCODE_JNE:
        		{
                	const variant * cmp = 0;
                	LOG_VARIANT(*m_cur_stack, fb, m_cur_stack->m_pos, "cmp");
                	GET_VARIANT(*m_cur_stack, fb, cmp, m_cur_stack->m_pos);
                	m_cur_stack->m_pos++;

                    int pos = COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos));
                	m_cur_stack->m_pos++;
                	
                    if (!(V_BOOL(cmp)))
                    {
                	    FKLOG("jne %d", pos);
                        
                        m_cur_stack->m_pos = pos;
                    }
                    else
                    {
                	    FKLOG("not jne %d", pos);
                    }
        		}
        		break;
        	case OPCODE_JMP:
        		{
                    int pos = COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos));
                	m_cur_stack->m_pos++;
                	
                	FKLOG("jmp %d", pos);

                    m_cur_stack->m_pos = pos;
        		}
        		break;
            
            case OPCODE_PLUS_ASSIGN:
                {
                    MATH_ASSIGN_OPER(*m_cur_stack, fb, PLUS);
                }
                break;
            case OPCODE_MINUS_ASSIGN:
                {
                    MATH_ASSIGN_OPER(*m_cur_stack, fb, MINUS);
                }
                break;
            case OPCODE_MULTIPLY_ASSIGN:
                {
                    MATH_ASSIGN_OPER(*m_cur_stack, fb, MULTIPLY);
                }
                break;
        	case OPCODE_DIVIDE_ASSIGN:
                {
                    MATH_ASSIGN_OPER(*m_cur_stack, fb, DIVIDE);
                }
                break;
        	case OPCODE_DIVIDE_MOD_ASSIGN:
                {
                    MATH_ASSIGN_OPER(*m_cur_stack, fb, DIVIDE_MOD);
                }
                break;
            case OPCODE_CALL:
                {
                	const variant * callpos = 0;
                	LOG_VARIANT(*m_cur_stack, fb, m_cur_stack->m_pos, "callpos");
                	GET_VARIANT(*m_cur_stack, fb, callpos, m_cur_stack->m_pos);
                	m_cur_stack->m_pos++;

                    assert (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos))) == ADDR_STACK);
                    m_retvpos = m_cur_stack->m_pos;
                	m_cur_stack->m_pos++;
                    
                    int argnum = COMMAND_CODE(GET_CMD(fb, m_cur_stack->m_pos));
                	m_cur_stack->m_pos++;

                	paramstack ps;
                	ps.clear();
                	for (int i = 0; i < argnum; i++)
                	{
                    	variant * arg = 0;
                    	LOG_VARIANT(*m_cur_stack, fb, m_cur_stack->m_pos, "arg");
                    	GET_VARIANT(*m_cur_stack, fb, arg, m_cur_stack->m_pos);
                    	m_cur_stack->m_pos++;

                        variant * argdest = 0;
                    	PS_PUSH_AND_GET(ps, argdest);
                    	*argdest = *arg;
                	}
                	
                    call(m_fk, callpos, &ps);
                }
                break;
            default:
                assert(0);
                FKERR("next err code %d %s", code, OpCodeStr(code));
                break;
            }

            // 当前函数走完
            if (m_cur_stack->m_pos >= (int)fb.cmdsize())
            {
                FKLOG("pop stack %s", m_cur_stack->m_fb->getname().c_str());
                // 出栈
        		m_cur_stack->clear();
        		m_stack_list_num--;
                // 所有都完
            	if (!m_stack_list_num)
                {
                    FKLOG("stack empty end");
                    m_isend = true;
                }
            	// 塞返回值
                else
                {
        		    m_cur_stack = &m_stack_list[m_stack_list_num - 1];
            		variant * ret;
                	GET_VARIANT(*m_cur_stack, fb, ret, m_retvpos);
                    *ret = m_ret;
                }
            }

            if (err)
            {
                // 发生错误
                m_isend = true;
            }
            num++;
            if (m_isend)
            {
                break;
            }
        }

        return num;
    }

private:
    void call(fuck * fk, const variant * callpos, paramstack * ps);
    
private:
    fuck * m_fk;
    bool m_isend;
    variant m_ret;
    int m_retvpos;
	stack * m_cur_stack;
	stack * m_stack_list;
	size_t m_stack_list_num;
	size_t m_stack_list_max_num;
};

