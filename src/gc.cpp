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
    m_fk->sh.checkgc();

    // TODO
}

const char * gc::dump()
{
	m_dumpstr.clear();
	return m_dumpstr.c_str();
}

array<stringele *> gc::get_used_stringele()
{
    array<stringele *> ret;

    // TODO

    return ret;
}


