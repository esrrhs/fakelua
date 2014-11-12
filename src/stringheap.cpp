#include "stringheap.h"
#include "fake.h"

stringheap::stringheap(fake * fk) : m_fk(fk), m_shh(fk)
{
}

stringheap::~stringheap()
{
	clear();
}

void stringheap::clear()
{
    for (int i = 0; i < (int)m_stringelelist.size(); i++)
    {
        safe_fkfree(m_fk, m_stringelelist[i].s);
    }
    m_stringelelist.clear();
	m_shh.clear();
}

stringele * stringheap::allocstring(const char * str)
{
    int * p = m_shh.get(str);
    if (p)
    {
		return &m_stringelelist[*p];
	}
	int pos = m_stringelelist.size();
	stringele e;
	e.update = 0;
	e.sz = strlen(str);
	e.s = (char*)safe_fkmalloc(m_fk, e.sz + 1);
	memcpy(e.s, str, e.sz);
	e.s[e.sz] = 0;
	m_stringelelist.push_back(e);
	m_shh.add(str, pos);
	return &m_stringelelist[pos];
}

void stringheap::gc()
{
	// TODO
}

