#pragma once

#include "types.h"
#include "hashmap.h"
#include "buildinfuncfile.h"
#include "buildinfuncos.h"
#include "buildinfuncstring.h"
#include "buildinfuncmath.h"

struct interpreter;
struct funcunion;

typedef void (*bifunc)(fake * fk, interpreter * inter);

#define BIF_CHECK_ARG_NUM(n)  \
{ \
	if (UNLIKE((int)fk->ps.m_variant_list_num != n)) \
	{\
		FKERR("buildin func param not match, give %d need %d", (int)fk->ps.m_variant_list_num, n);\
		seterror(fk, efk_run_param_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk), "buildin func param not match, give %d need %d", (int)fk->ps.m_variant_list_num, n);\
		return;\
	} \
}

class buildinfunc
{
public:
	force_inline buildinfunc(fake * fk) : m_fk(fk), m_bifile(fk), m_bifos(fk), m_bifstring(fk), m_bifmath(fk)
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
		m_bifos.clear();
		m_bifstring.clear();
	}

	void openbasefunc();
	void openfilefunc();
	void openosfunc();
	void openstringfunc();
	void openmathfunc();

	void setargv(int argc, const char *argv[]);
	int get_argc() const;
	const char * get_argv(int pos) const;

	void setseed(int seed);
	int getseed() const;

	void set_print_func(fkprint func);
	fkprint get_print_func() const;

private:
	fake * m_fk;
	fkprint m_fkprint;
	buildinfuncfile m_bifile;
	buildinfuncos m_bifos;
	buildinfuncstring m_bifstring;
	buildinfuncmath m_bifmath;
};

// 参数和返回值都在m_fk->ps里
#define BUILDIN_FUNC_CALL(f, inter) \
	assert((f)->havebif);\
	assert((f)->bif); \
	((f)->bif)(m_fk, (inter))
