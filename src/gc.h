#pragma once

#include "types.h"
#include "hashmap.h"
#include "array.h"

struct fake;

class gc
{
public:
	gc(fake * fk);
	~gc();

	void clear();

	void check();

	const char * dump();

private:
    void go();
	
private:
	fake * m_fk;
	String m_dumpstr;
};

