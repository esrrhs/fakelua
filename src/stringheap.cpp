#include "stringheap.h"
#include "fake.h"

stringheap::stringheap(fake * fk) : m_fk(fk), m_shh(fk)
{
	ARRAY_INI(m_todelete, m_fk);
}

stringheap::~stringheap()
{
	for (const fkhashmap<const char *, stringele>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
	{
		stringele * e = p->t;
		safe_fkfree(m_fk, e->s);
	}
	m_shh.clear();
	ARRAY_DELETE(m_todelete);
}

void stringheap::clear()
{
	ARRAY_CLEAR(m_todelete);
    for (const fkhashmap<const char *, stringele>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
    {
        stringele * e = p->t;
		if (!e->sysref)
		{
			safe_fkfree(m_fk, e->s);
			if (ARRAY_SIZE(m_todelete) >= ARRAY_MAX_SIZE(m_todelete))
			{
			    size_t newsize = ARRAY_MAX_SIZE(m_todelete) + 1 + ARRAY_MAX_SIZE(m_todelete) * (m_fk->cfg.array_grow_speed) / 100;
		        ARRAY_GROW(m_todelete, newsize, const char *);
		    }
			ARRAY_PUSH_BACK(m_todelete);
			ARRAY_BACK(m_todelete) = p->k;
		}
    }
    
	for (int i = 0; i < (int)ARRAY_SIZE(m_todelete); i++)
	{
		const char * str = ARRAY_GET(m_todelete, i);
		m_shh.del(str);
	}
	
	ARRAY_CLEAR(m_todelete);
}

stringele * stringheap::allocstring(const char * str)
{
	stringele * p = m_shh.get(str);
    if (p)
    {
		return p;
	}
	stringele e;
	e.update = 0;
	e.sz = strlen(str);
	e.s = stringdump(m_fk, str, e.sz);
	e.sysref = 0;
	return m_shh.add(str, e)->t;
}

variant stringheap::allocsysstr(const char * str)
{
	fake * fk = m_fk;
	variant v;
	V_SET_STRING(&v, str);
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

