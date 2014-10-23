#include "stringheap.h"
#include "fuck.h"

stringheap::~stringheap()
{
	clear();
}

void stringheap::clear()
{
	for (int i = 0; i < (int)m_stringelelist.size(); i++)
	{
		fkdelete<stringele>(m_fk, m_stringelelist[i]);
	}
	m_stringelelist.clear();
}

stringele * stringheap::allocstring(const char * str)
{
	// TODO 做hash和gc，先直接缓存起来
	for (int i = 0; i < (int)m_stringelelist.size(); i++)
	{
		if (m_stringelelist[i]->str == str)
		{
			return m_stringelelist[i];
		}
	}
	stringele * p = fknew<stringele>(m_fk, str);
	m_stringelelist.push_back(p);
	return p;
}

void stringheap::gc()
{
	// TODO
}
