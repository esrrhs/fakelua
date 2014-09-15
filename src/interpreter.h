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
	stack();
	stack(fuck * fk, fkerrorinfo * ei, const  func_binary * fb);
	~stack();

	void grow(int pos);

	void clear();

    void set_stack_variant(const variant & v, int pos)
    {
		if (pos >= (int)m_stack_variant_list_num)
		{
			grow(pos);
		}
        m_stack_variant_list[pos] = v;
    }
	const variant * get_stack_variant(int pos)
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
	interpreter(fuck * fk, fkerrorinfo * ei) : m_fk(fk), m_ei(ei), 
		m_stack_list(0), m_stack_list_num(0), m_stack_list_max_num(0)
    {
        clear();
    }
    ~interpreter();

	void grow();
    void clear();
    
    bool isend() const;
    
    void call(binary * bin, const String & func, paramstack * ps);
    const variant & getret() const;

    int run(int cmdnum);
    
private:
    bool next();
	bool next_assign(stack & s, const func_binary & fb, int code);
	bool next_math(stack & s, const func_binary & fb, int code);
	bool next_return(stack & s, const func_binary & fb, int code);
private:
    fuck * m_fk;
    fkerrorinfo * m_ei;
    bool m_isend;
    variant m_ret;
	stack * m_stack_list;
	size_t m_stack_list_num;
	size_t m_stack_list_max_num;
};

