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

	void clear()
	{
		// 为了效率，保留脏数据
		m_variant_list_num = 0;
	}

	fuck * getfuck()
	{
		return m_fk;
	}

    void push(const variant & v);
    void pop(variant & v);

	variant operator[](int i)
	{
		assert(i >= 0 && i < REAL_MAX_FUCK_PARAM_NUM);
		return m_variant_list[i];
	}

	size_t size() const
	{
		return m_variant_list_num;
	}

private:
	fuck * m_fk;
	variant m_variant_list[REAL_MAX_FUCK_PARAM_NUM];
	size_t m_variant_list_num;
};

