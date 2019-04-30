#include "gc.h"
#include "fake.h"

gc::gc(fake * fk) : m_fk(fk)
{
}

gc::~gc()
{
	clear();
}

void gc::clear()
{
}

void gc::check()
{
}

void gc::go()
{
}

const char * gc::dump()
{
	m_dumpstr.clear();
	return m_dumpstr.c_str();
}

