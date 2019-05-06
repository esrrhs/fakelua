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

	void check(bool force);

	const char * dump();

    array<stringele *> get_used_stringele();
	
private:
	fake * m_fk;
	String m_dumpstr;
};

