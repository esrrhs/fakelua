#include "paramstack.h"
#include "fuck.h"

void paramstack::push(const variant & v)
{
	assert(m_variant_list_num < REAL_MAX_FUCK_PARAM_NUM);
	m_variant_list[m_variant_list_num] = v;
	m_variant_list_num++;
}

void paramstack::pop(variant & v)
{
	assert(m_variant_list_num > 0);
	v = m_variant_list[m_variant_list_num - 1];
	m_variant_list_num--;
}
