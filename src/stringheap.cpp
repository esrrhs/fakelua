#include "stringheap.h"
#include "fake.h"

stringheap::stringheap(fake * fk) : m_fk(fk), m_shh(fk)
{
	ARRAY_INI(m_todelete, m_fk);
}

stringheap::~stringheap()
{
	for (const fkhashset<stringele *>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
	{
		stringele * e = p->k;
		safe_fkfree(m_fk, e->s);
		safe_fkfree(m_fk, e);
	}
	ARRAY_DELETE(m_todelete);
}

void stringheap::clear()
{
	ARRAY_CLEAR(m_todelete);
	for (const fkhashset<stringele *>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
	{
		stringele * e = p->k;
		if (!e->sysref)
		{
			if (ARRAY_SIZE(m_todelete) >= ARRAY_MAX_SIZE(m_todelete))
			{
				size_t newsize = ARRAY_MAX_SIZE(m_todelete) + 1 + ARRAY_MAX_SIZE(m_todelete) * (m_fk->cfg.array_grow_speed) / 100;
				ARRAY_GROW(m_todelete, newsize, stringele *);
			}
			ARRAY_PUSH_BACK(m_todelete);
			ARRAY_BACK(m_todelete) = e;
		}
	}
	
	for (int i = 0; i < (int)ARRAY_SIZE(m_todelete); i++)
	{
		stringele * e = ARRAY_GET(m_todelete, i);
		m_shh.del(e);
		safe_fkfree(m_fk, e->s);
		safe_fkfree(m_fk, e);
	}
	
	ARRAY_CLEAR(m_todelete);
	m_dumpstr.clear();
}

stringele * stringheap::allocstring(const char * str)
{
	stringele tmp;
	tmp.s = (char*)str;
	fkhashset<stringele *>::ele * p = m_shh.get(&tmp);
	if (LIKE(p != 0))
	{
		return p->k;
	}
	stringele * e = (stringele *)safe_fkmalloc(m_fk, sizeof(stringele), emt_stringele);
	e->sz = strlen(str);
	e->s = stringdump(m_fk, str, e->sz, emt_stringheap);
	e->sysref = 0;
	return m_shh.add(e)->k;
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
	if (UNLIKE((int)m_shh.size() > m_fk->cfg.string_heap_num))
	{
		gc();
	}
}

void stringheap::gc()
{
	clear();
}

const char * stringheap::dump()
{
	m_dumpstr.clear();
	for (const fkhashset<stringele *>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
	{
		stringele * e = p->k;
		m_dumpstr += e->s;
		if (e->sysref)
		{
			m_dumpstr += "(system)";
		}
		m_dumpstr += "\n";
	}
	return m_dumpstr.c_str();
}

size_t stringheap::sys_size() const
{
	size_t ret = 0;
	for (const fkhashset<stringele *>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
	{
		stringele * e = p->k;
		if (e->sysref)
		{
			ret++;
		}
	}
	return ret;
}

size_t stringheap::bytesize() const
{
	size_t ret = 0;
	for (const fkhashset<stringele *>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
	{
		stringele * e = p->k;
		ret += e->sz;
	}
	return ret;
}

size_t stringheap::sys_bytesize() const
{
	size_t ret = 0;
	for (const fkhashset<stringele *>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
	{
		stringele * e = p->k;
		if (e->sysref)
		{
			ret += e->sz;
		}
	}
	return ret;
}

