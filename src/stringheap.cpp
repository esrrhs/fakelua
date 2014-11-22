#include "stringheap.h"
#include "fake.h"

stringheap::stringheap(fake * fk) : m_fk(fk), m_shh(fk)
{
	ARRAY_INI(m_sysstr_list, fk);
}

stringheap::~stringheap()
{
	for (const stringhashmap<stringele*>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
	{
		stringele * e = p->t;
		safe_fkfree(m_fk, e->s);
		safe_fkfree(m_fk, e);
	}
	m_shh.clear();
	ARRAY_DELETE(m_sysstr_list);
}

void stringheap::clear()
{
    for (const stringhashmap<stringele*>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
    {
        stringele * e = p->t;
		if (!e->sysref)
		{
			safe_fkfree(m_fk, e->s);
			safe_fkfree(m_fk, e);
		}
    }
	m_shh.clear();
	for (int i = 0; i < (int)ARRAY_SIZE(m_sysstr_list); i++)
	{
		stringele * e = ARRAY_GET(m_sysstr_list, i);
		m_shh.add(e->s, e);
	}
}

stringele * stringheap::allocstring(const char * str)
{
	stringele ** p = m_shh.get(str);
    if (p)
    {
		return *p;
	}
	stringele * pe = (stringele*)safe_fkmalloc(m_fk, sizeof(stringele));
	stringele & e = *pe;
	e.update = 0;
	e.sz = strlen(str);
	e.s = stringdump(m_fk, str, e.sz);
	e.sysref = 0;
	m_shh.add(str, pe);
	return pe;
}

variant stringheap::allocsysstr(const char * str)
{
	fake * fk = m_fk;
	variant v;
	V_SET_STRING(&v, str);
	// 存起来，gc完再加回去
	if (!v.data.str->sysref)
	{
		if (ARRAY_SIZE(m_sysstr_list) >= ARRAY_MAX_SIZE(m_sysstr_list))
		{
			size_t newsize = ARRAY_MAX_SIZE(m_sysstr_list) + 1 + ARRAY_MAX_SIZE(m_sysstr_list) * (m_fk->cfg.array_grow_speed) / 100;
			ARRAY_GROW(m_sysstr_list, newsize, stringele*);
		}
		ARRAY_PUSH_BACK(m_sysstr_list);
		ARRAY_BACK(m_sysstr_list) = v.data.str;
	}
	v.data.str->sysref++;
	return v;
}

void stringheap::checkgc()
{
	if ((int)m_shh.size() > m_fk->cfg.string_heap_num)
	{
	    gc();
	}
}

void stringheap::gc()
{
	clear();
}
