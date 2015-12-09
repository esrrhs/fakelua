#include "pointerheap.h"
#include "fake.h"

pointerheap::pointerheap(fake * fk) : m_fk(fk), m_shh(fk)
{
}

pointerheap::~pointerheap()
{
	clear();
}

void pointerheap::clear()
{
	for (const fkhashmap<void *, pointerele>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
	{
		pointerele * e = p->t;
		safe_fkfree(m_fk, e->type);
	}
	m_shh.clear();
	m_dumpstr.clear();
}

pointerele * pointerheap::allocpointer(void * ptr, const char * type)
{
	pointerele * p = m_shh.get(ptr);
	if (LIKE(p != 0))
	{
		if (UNLIKE(strcmp(p->type, type) != 0))
		{
			safe_fkfree(m_fk, p->type);
			p->typesz = strlen(type);
			p->type = stringdump(m_fk, type, p->typesz);
		}
		return p;
	}
	
	pointerele e;
	e.ptr = ptr;
	e.typesz = strlen(type);
	e.type = stringdump(m_fk, type, e.typesz);
	
	return m_shh.add(ptr, e)->t;
}

void pointerheap::checkgc()
{
	if (UNLIKE((int)m_shh.size() > m_fk->cfg.pointer_heap_num))
	{
		gc();
	}
}

void pointerheap::gc()
{
	clear();
}

const char * pointerheap::dump()
{
	m_dumpstr.clear();
	for (const fkhashmap<void *, pointerele>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
	{
		pointerele * e = p->t;
		m_dumpstr += fkptoa(e->ptr);
		m_dumpstr += "(";
		m_dumpstr += e->type;
		m_dumpstr += ")\n";
	}
	return m_dumpstr.c_str();
}

