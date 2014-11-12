#pragma once

#include "types.h"
#include "hashmap.h"

struct fake;

struct stringele
{
    // 更新计数
    uint32_t update;
	// string
    char * s;
    size_t sz;
};

typedef std::vector<stringele> stringelelist;

class stringheap
{
public:
    stringheap(fake * fk);
	~stringheap();

	void clear();

	stringele * allocstring(const char * str);

private:
	void gc();

private:
    fake * m_fk;    
    stringelelist m_stringelelist;
    stringhashmap<int> m_shh;
};

