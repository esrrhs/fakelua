#pragma once

#include "types.h"
#include "hashmap.h"
#include "array.h"

struct fake;

struct stringele
{
    // 更新计数
    uint32_t update;
	// string
    char * s;
    size_t sz;
	// 系统引用
	uint32_t sysref;
};

class stringheap
{
public:
    stringheap(fake * fk);
	~stringheap();

	void clear();

	stringele * allocstring(const char * str);

	variant allocsysstr(const char * str);

	void checkgc();
	
private:
	void gc();

private:
	fake * m_fk;
	fkhashmap<const char *, stringele> m_shh;
	array<const char *> m_todelete;
};

