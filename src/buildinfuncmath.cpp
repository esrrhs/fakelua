#include "buildinfuncmath.h"
#include "fake.h"
#include "fakescript.h"

// msrand
void buildin_msrand(fake * fk, interpreter * inter)
{
	int seed = fkpspop<int>(fk);
	fk->bif.setseed(seed);
	fkpspush<bool>(fk, true);
}

// mrand
void buildin_mrand(fake * fk, interpreter * inter)
{
	int s = (((fk->bif.getseed() * 214013L
		+ 2531011L) >> 16) & 0x7fff);
	fk->bif.setseed(s);
	fkpspush<int>(fk, s);
}

void buildinfuncmath::openmathfunc()
{
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("msrand"), buildin_msrand);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("mrand"), buildin_mrand);
}

void buildinfuncmath::setseed(int seed)
{
	m_seed = seed;
}

int buildinfuncmath::getseed() const
{
	return m_seed;
}

