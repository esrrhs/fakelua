#include "buildinfuncmath.h"
#include "fake.h"
#include "fakescript.h"

// msrand
void buildin_srand(fake * fk, interpreter * inter)
{
	BIF_CHECK_ARG_NUM(1);

	int seed = fkpspop<int>(fk);
	fk->bif.setseed(seed);
	fkpspush<bool>(fk, true);
}

// mrand
void buildin_rand(fake * fk, interpreter * inter)
{
	int s = (((fk->bif.getseed() * 214013L
		+ 2531011L) >> 16) & 0x7fff);
	fk->bif.setseed(s);
	fkpspush<int>(fk, s);
}

// ceil
void buildin_ceil(fake * fk, interpreter * inter)
{
	BIF_CHECK_ARG_NUM(1);

	double data = fkpspop<double>(fk);
	int ret = (int)ceil(data);
	fkpspush<int>(fk, ret);
}

// floor
void buildin_floor(fake * fk, interpreter * inter)
{
	BIF_CHECK_ARG_NUM(1);

	double data = fkpspop<double>(fk);
	int ret = (int)floor(data);
	fkpspush<int>(fk, ret);
}

void buildinfuncmath::openmathfunc()
{
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("srand"), buildin_srand);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("rand"), buildin_rand);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("ceil"), buildin_ceil);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("floor"), buildin_floor);
}

void buildinfuncmath::setseed(int seed)
{
	m_seed = seed;
}

int buildinfuncmath::getseed() const
{
	return m_seed;
}

