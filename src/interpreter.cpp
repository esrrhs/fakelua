#include "interpreter.h"
#include "fuck.h"
#include "binary.h"
#include "paramstack.h"

void stack::grow(size_t size)
{
	assert(m_fk);
	// 新空间
	int newsize = size;
	assert(newsize > (int)m_stack_variant_list_num);
	variant * newbuff = (variant *)safe_fkmalloc(m_fk, (newsize * sizeof(variant)));
	
	// 复制
	if (m_stack_variant_list)
	{
		memcpy(newbuff, m_stack_variant_list, m_stack_variant_list_num * sizeof(variant));
	}

	// 构造剩余的
	memset(&newbuff[m_stack_variant_list_num], 0, (newsize - m_stack_variant_list_num) * sizeof(variant));

	// 删除
	safe_fkfree(m_fk, m_stack_variant_list);

	m_stack_variant_list = newbuff;
	m_stack_variant_list_num = newsize;
}

//////////////////////////////////////////////////////////////////////////

void interpreter::grow()
{
	assert(m_fk);
	// 新空间
	int newsize = m_stack_list_max_num + 1 + m_stack_list_max_num * m_fk->cfg.stack_list_grow_speed / 100;
	assert(newsize > (int)m_stack_list_max_num);
	stack * newbuff = (stack *)safe_fkmalloc(m_fk, (newsize * sizeof(stack)));

	// 复制
	if (m_stack_list)
	{
		memcpy(newbuff, m_stack_list, m_stack_list_max_num * sizeof(stack));
	}

	// 构造剩余的
	memset(&newbuff[m_stack_list_max_num], 0, (newsize - m_stack_list_max_num) * sizeof(stack));

	// 删除
	safe_fkfree(m_fk, m_stack_list);

	m_stack_list = newbuff;
	m_stack_list_max_num = newsize;
}

