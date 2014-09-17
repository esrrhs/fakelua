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

