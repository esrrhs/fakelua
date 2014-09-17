#pragma once

#include "types.h"
#include "variant.h"

struct fuck;
struct fkerrorinfo;
struct binary;
struct paramstack;
struct func_binary;

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
    
    void call(binary * bin, const String & func, paramstack * ps);
    force_inline const variant & getret() const
    {
        return m_ret;
    }

    force_inline int run(int cmdnum)
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
    
private:
    bool next();
	bool next_assign(stack & s, const func_binary & fb, int code);
	bool next_math(stack & s, const func_binary & fb, int code);
	bool next_return(stack & s, const func_binary & fb, int code);
	bool next_jne(stack & s, const func_binary & fb, int code);
	bool next_jmp(stack & s, const func_binary & fb, int code);
private:
    fuck * m_fk;
    fkerrorinfo * m_ei;
    bool m_isend;
    variant m_ret;
	stack * m_stack_list;
	size_t m_stack_list_num;
	size_t m_stack_list_max_num;
};

