#pragma once

#include "types.h"
#include "hashmap.h"
#include "buildinfuncfile.h"
#include "buildinfuncnet.h"
#include "buildinfuncos.h"
#include "buildinfuncstring.h"
#include "buildinfuncmath.h"

struct interpreter;
struct funcunion;

typedef void (*bifunc)(fake * fk, interpreter * inter);

class buildinfunc
{
public:
	force_inline buildinfunc(fake * fk) : m_fk(fk), m_bifile(fk), m_bifnet(fk), m_bifos(fk), m_bifstring(fk), m_bifmath(fk)
	{
	}
	force_inline ~buildinfunc()
	{
		clear();
	}

	force_inline fake * getfake()
	{
		return m_fk;
	}

	force_inline void clear()
	{
		m_bifile.clear();
		m_bifnet.clear();
		m_bifos.clear();
		m_bifstring.clear();
	}

	void openbasefunc();
	void openfilefunc();
	void opennetfunc();
	void openosfunc();
	void openstringfunc();
	void openmathfunc();

	buffer * newbuffer(int size);
	selector * newselector();

	void setargv(int argc, const char *argv[]);
	int get_argc() const;
	const char * get_argv(int pos) const;

	void setseed(int seed);
	int getseed() const;

private:
	fake * m_fk;
	buildinfuncfile m_bifile;
	buildinfuncnet m_bifnet;
	buildinfuncos m_bifos;
	buildinfuncstring m_bifstring;
	buildinfuncmath m_bifmath;
};

// 参数和返回值都在m_fk->ps里
#define BUILDIN_FUNC_CALL(f, inter) \
	assert((f)->havebif);\
	assert((f)->bif); \
	((f)->bif)(m_fk, (inter))
