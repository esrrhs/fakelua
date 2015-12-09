#include "pointerheap.h"
#include "fake.h"

pointerheap::pointerheap(fake * fk) : m_fk(fk)
{
	ARRAY_INI(m_pointers, m_fk);
}

pointerheap::~pointerheap()
{
	ARRAY_DELETE(m_pointers);
}

void pointerheap::clear()
{
	for (int i = 0; i < (int)ARRAY_SIZE(m_pointers); i++)
	{
		pointerele * ptr = ARRAY_GET(m_pointers, i);
		fkdelete<pointerele>(m_fk, ptr);
	}
	
	ARRAY_CLEAR(m_pointers);
}

pointerele * pointerheap::allocpointer(void * ptr, const char * type)
{
	pointerele * pointer = fknew<pointerele>(m_fk);
	pointer->ptr = ptr;
	pointer->type = type;
	
	if (ARRAY_SIZE(m_pointers) >= ARRAY_MAX_SIZE(m_pointers))
	{
		size_t newsize = ARRAY_MAX_SIZE(m_pointers) + 1 + ARRAY_MAX_SIZE(m_pointers) * (m_fk->cfg.array_grow_speed) / 100;
		ARRAY_GROW(m_pointers, newsize, pointerele *);
	}
	ARRAY_PUSH_BACK(m_pointers);
	ARRAY_BACK(m_pointers) = pointer;

	return pointer;
}

void pointerheap::checkgc()
{
	if ((int)ARRAY_SIZE(m_pointers) > m_fk->cfg.pointer_heap_num)
	{
		clear();
	}
}

