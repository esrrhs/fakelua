#pragma once

#include "types.h"
#include "variant.h"
#include "binary.h"
#include "paramstack.h"

struct fuck;
struct fkerrorinfo;

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
	LOG_VARIANT(s, fb, s.m_pos, "left");\
    GET_VARIANT(s, fb, left, s.m_pos);\
    s.m_pos++;\
    \
	const variant * right = 0;\
	LOG_VARIANT(s, fb, s.m_pos, "right");\
    GET_VARIANT(s, fb, right, s.m_pos);\
    s.m_pos++;\
    \
	variant * dest;\
    assert (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, s.m_pos))) == ADDR_STACK);\
    LOG_VARIANT(s, fb, s.m_pos, "dest");\
	GET_VARIANT(s, fb, dest, s.m_pos);\
    s.m_pos++;\
    \
	FKLOG("math left %s right %s", ((String)*left).c_str(), ((String)*right).c_str());\
    \
    dest->oper(*left, *right);\
    \
    FKLOG("math %s %s", OpCodeStr(code), ((String)*dest).c_str());
 
#define MATH_ASSIGN_OPER(s, fb, oper) \
	variant * var = 0;\
    assert (ADDR_TYPE(COMMAND_CODE(GET_CMD(fb, s.m_pos))) == ADDR_STACK);\
	LOG_VARIANT(s, fb, s.m_pos, "var");\
    GET_VARIANT(s, fb, var, s.m_pos);\
    s.m_pos++;\
    \
	const variant * value = 0;\
	LOG_VARIANT(s, fb, s.m_pos, "value");\
    GET_VARIANT(s, fb, value, s.m_pos);\
    s.m_pos++;\
    \
	FKLOG("math var %s value %s", ((String)*var).c_str(), ((String)*value).c_str());\
    \
    var->oper(*var, *value);\
    \
    FKLOG("math %s %s", OpCodeStr(code), ((String)*var).c_str());

struct stack
{
	force_inline stack() : m_fk(0), m_ei(0), m_fb(0), m_pos(0), m_stack_variant_list(0), m_stack_variant_list_num(0)
    {
    }
	force_inline stack(fuck * fk, fkerrorinfo * ei, const  func_binary * fb) : m_fk(fk), m_ei(ei), m_fb(fb), m_pos(0), m_stack_variant_list(0), m_stack_variant_list_num(0)
    {
    	grow(m_fk->m_stack_ini_size);
    }
	force_inline ~stack()
    {
    	if (m_stack_variant_list)
    	{
    		assert(m_fk);
    		for (int i = 0; i < (int)m_stack_variant_list_num; i++)
    		{
    			m_stack_variant_list[i].~variant();
    		}
    		m_fk->m_fkfree(m_stack_variant_list);
    	}
    }

	void grow(int pos);

	force_inline void clear()
    {
    	m_pos = 0;
    	// 为了效率，保留脏数据
    }

	force_inline void set_stack_variant(const variant & v, int pos)
    {
		if (pos >= (int)m_stack_variant_list_num)
		{
			grow(pos);
		}
        m_stack_variant_list[pos] = v;
    }
	force_inline const variant * get_stack_variant(int pos)
	{
		if (pos >= (int)m_stack_variant_list_num)
		{
			grow(pos);
		}
        return &m_stack_variant_list[pos];
    }
    
    fuck * m_fk;
    fkerrorinfo * m_ei;
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
	force_inline interpreter(fuck * fk, fkerrorinfo * ei) : m_fk(fk), m_ei(ei), m_isend(false), 
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
    		m_fk->m_fkfree(m_stack_list);
    	}
    }

	void grow();
    
    bool isend() const
    {
        return m_isend;
    }
    
    void call(binary * bin, const char * func, paramstack * ps)
    {
        const func_binary * fb = bin->get_func(func);
        if (!fb)
        {
            FKERR("fkrun bin %p no func %s fail", bin, func.c_str());
            m_fk->seterror(m_ei, efk_run_no_func_error, "fkrun bin %p no func %s fail", bin, func);
            m_isend = true;
            return;
        }

        // 空函数处理
        if (!fb->cmdsize())
        {
            // 所有都完
        	if (!m_stack_list_num)
            {
                FKLOG("call stack empty end");
                m_isend = true;
            }
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
        	stack & s = m_stack_list[m_stack_list_num - 1];
            const func_binary & fb = *s.m_fb;
            int pos = s.m_pos;

            command cmd = GET_CMD(fb, pos);
            int type = COMMAND_TYPE(cmd);
            int code = COMMAND_CODE(cmd);

            FKLOG("next %d %d %s", type, code, OpCodeStr(code));
                
            assert (type == COMMAND_OPCODE);

            s.m_pos++;

            // 执行对应命令，放一起switch效率更高，cpu有缓存
            switch (code)
            {
            case OPCODE_ASSIGN:
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
                }
                break;
            case OPCODE_PLUS:
                {
        		    MATH_OPER(s, fb, plus);
        		}
                break;
            case OPCODE_MINUS:
                {
        		    MATH_OPER(s, fb, minus);
        		}
                break;
            case OPCODE_MULTIPLY:
        		{
        		    MATH_OPER(s, fb, multiply);
        		}
                break;
            case OPCODE_DIVIDE:
        		{
        		    MATH_OPER(s, fb, divide);
        		}
                break;
            case OPCODE_DIVIDE_MOD:
        		{
        		    MATH_OPER(s, fb, divide_mode);
        		}
                break;
            case OPCODE_AND:
        		{
        		    MATH_OPER(s, fb, band);
        		}
                break;
            case OPCODE_OR:
        		{
        		    MATH_OPER(s, fb, bor);
        		}
                break;
            case OPCODE_LESS:
        		{
        		    MATH_OPER(s, fb, less);
        		}
                break;
        	case OPCODE_MORE:
        		{
        		    MATH_OPER(s, fb, more);
        		}
                break;
        	case OPCODE_EQUAL:
        		{
        		    MATH_OPER(s, fb, equal);
        		}
                break;
        	case OPCODE_MOREEQUAL:
        		{
        		    MATH_OPER(s, fb, moreequal);
        		}
                break;
        	case OPCODE_LESSEQUAL:
        		{
        		    MATH_OPER(s, fb, lessequal);
        		}
                break;
        	case OPCODE_NOTEQUAL:
        		{
        		    MATH_OPER(s, fb, notequal);
        		}
                break;
        	case OPCODE_RETURN:
        	    {
                	if (GET_CMD(fb, s.m_pos) == EMPTY_CMD)
                	{
                		FKLOG("return empty");
                		break;
                	}

                	// 塞给ret
                	const variant * ret = 0;
                	LOG_VARIANT(s, fb, s.m_pos, "ret");
                	GET_VARIANT(s, fb, ret, s.m_pos);
                	s.m_pos++;

                	m_ret = *ret;

                	FKLOG("return %s", ((String)m_ret).c_str());

        	    }
        		break;
        	case OPCODE_JNE:
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
        		}
        		break;
        	case OPCODE_JMP:
        		{
                    int pos = COMMAND_CODE(GET_CMD(fb, s.m_pos));
                	s.m_pos++;
                	
                	FKLOG("jmp %d", pos);

                    s.m_pos = pos;
        		}
        		break;
            
            case OPCODE_PLUS_ASSIGN:
                {
                    MATH_ASSIGN_OPER(s, fb, plus);
                }
                break;
            case OPCODE_MINUS_ASSIGN:
                {
                    MATH_ASSIGN_OPER(s, fb, minus);
                }
                break;
            case OPCODE_MULTIPLY_ASSIGN:
                {
                    MATH_ASSIGN_OPER(s, fb, multiply);
                }
                break;
        	case OPCODE_DIVIDE_ASSIGN:
                {
                    MATH_ASSIGN_OPER(s, fb, divide);
                }
                break;
        	case OPCODE_DIVIDE_MOD_ASSIGN:
                {
                    MATH_ASSIGN_OPER(s, fb, divide_mode);
                }
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

            if (err)
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

private:
    fuck * m_fk;
    fkerrorinfo * m_ei;
    bool m_isend;
    variant m_ret;
	stack * m_stack_list;
	size_t m_stack_list_num;
	size_t m_stack_list_max_num;
};

