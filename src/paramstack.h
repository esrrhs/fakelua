#pragma once

#include "types.h"
#include "variant.h"

#define REAL_MAX_FUCK_PARAM_NUM (MAX_FUCK_PARAM_NUM + MAX_FUCK_RETURN_NUM)

struct fuck;
class paramstack
{
public:
	paramstack(fuck * fk) : m_fk(fk), m_variant_list_num(0)
	{

	}
	~paramstack()
	{

	}

	force_inline fuck * getfuck()
	{
		return m_fk;
	}

    force_inline void push(const variant & v)
    {
    	assert(m_variant_list_num < REAL_MAX_FUCK_PARAM_NUM);
    	m_variant_list[m_variant_list_num] = v;
    	m_variant_list_num++;
    }

    force_inline void pop(variant & v)
    {
    	assert(m_variant_list_num > 0);
    	v = m_variant_list[m_variant_list_num - 1];
    	m_variant_list_num--;
    }

	force_inline const variant & operator[](int i)
	{
		assert(i >= 0 && i < REAL_MAX_FUCK_PARAM_NUM);
		return m_variant_list[i];
	}

	force_inline size_t size() const
	{
		return m_variant_list_num;
	}

private:
	fuck * m_fk;
	variant m_variant_list[REAL_MAX_FUCK_PARAM_NUM];
	size_t m_variant_list_num;
};

