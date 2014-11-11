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
	m_shh.clear();
}

stringele * stringheap::allocstring(const char * str)
{
    stringhashmap<stringele>::ele * p = m_shh.get_ele(str);
    if (p)
    {
        p->t.update++;
		return &p->t;
	}
	stringele e;
	e.update = 0;
	p = m_shh.add(str, e);
	p->t.s = p->s;
	p->t.sz = p->sz;
	return &p->t;
}

void stringheap::gc()
{
	// TODO
}

