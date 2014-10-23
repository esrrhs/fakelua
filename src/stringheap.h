#pragma once

#include "types.h"

struct fuck;

struct stringele
{
    stringele() : update(0), hash(0) {}
	stringele(const char * _str) : update(0), hash(0), str(_str) {}
    // 更新计数
    uint32_t update;
    // hash值
    uint32_t hash;
	// string
    String str;
};

typedef std::vector<stringele*> stringelelist;

class stringheap
{
public:
    force_inline stringheap(fuck * fk) : m_fk(fk)
    {
    }
	~stringheap();

	void clear();

	stringele * allocstring(const char * str);

private:
	void gc();

private:
    fuck * m_fk;    
    stringelelist m_stringelelist;
};

