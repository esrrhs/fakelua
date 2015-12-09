#pragma once

#include "types.h"
#include "hashmap.h"
#include "array.h"

struct fake;

struct pointerele
{
	void * ptr;
	String type;
};

class pointerheap
{
public:
	pointerheap(fake * fk);
	~pointerheap();

	void clear();

	pointerele * allocpointer(void * ptr, const char * type);

	void checkgc();
	
private:
	fake * m_fk;
	array<pointerele*> m_pointers;
};

