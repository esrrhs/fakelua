#include "bindfunc.h"
#include "fake.h"
#include "fakescript.h"
#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

// print, very slow
void buildin_print(fake * fk, interpreter * inter)
{
	String str;
	for (int i = 0; i < (int)fk->ps.m_variant_list_num; i++)
	{
		str += vartostring(&fk->ps.m_variant_list[i]);
	}
	// printf
	if (fk->bif.get_print_func())
	{
		fk->bif.get_print_func()(fk, str.c_str());
	}
	else
	{
		printf("%s\n", str.c_str());
	}
	PS_CLEAR(fk->ps);
	// ret
	fkpspush<int>(fk, (int)str.size());
}

// log, very slow
void buildin_log(fake * fk, interpreter * inter)
{
	String str;
	for (int i = 0; i < (int)fk->ps.m_variant_list_num; i++)
	{
		str += vartostring(&fk->ps.m_variant_list[i]);
	}
	
	// log
	FILE * pLog = fopen("fk.log", "a+");
	if (pLog)
	{
		time_t clock1 = time(0);
		struct tm * tptr = localtime(&clock1);
		const char * func = inter->get_running_func_name();
		
		fprintf(pLog, "[%d.%d.%d, %d.%d.%d][%s] : %s\n", 
			tptr->tm_year + 1990, tptr->tm_mon + 1, tptr->tm_mday, tptr->tm_hour, tptr->tm_min, tptr->tm_sec,
			func, str.c_str());

		fclose(pLog);
	}

	PS_CLEAR(fk->ps);

	// ret
	fkpspush<int>(fk, (int)str.size());
}

// array
void buildin_array(fake * fk, interpreter * inter)
{
	bool err = false;
	variant_array * a = fk->con.newarray();
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_ARRAY(v, a);
}

// map
void buildin_map(fake * fk, interpreter * inter)
{
	bool err = false;
	variant_map * m = fk->con.newmap();
	variant * v = 0;
	PS_PUSH_AND_GET(fk->ps, v);
	V_SET_MAP(v, m);
}

// size
void buildin_size(fake * fk, interpreter * inter)
{
	bool err = false;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	int len = 0;
	if (v->type == variant::STRING)
	{
		len = v->data.str->sz;
	}
	else if (v->type == variant::ARRAY)
	{
		len = ARRAY_SIZE(v->data.va->va);
	}
	else if (v->type == variant::MAP)
	{
		len = v->data.vm->vm.size();
	}
	fkpspush<int>(fk, len);
}

// debug
void buildin_debug(fake * fk, interpreter * inter)
{
	for (int i = 0; i < (int)fk->ps.m_variant_list_num; i++)
	{
		variant * v = 0;
		v = (&fk->ps.m_variant_list[i]);
		USE(v);
	}
	
	PS_CLEAR(fk->ps);
	
	// ret
	fkpspush<int>(fk, 1);
}

// typeof
void buildin_typeof(fake * fk, interpreter * inter)
{
	bool err = false;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);
	const char * name = vartypetostring(v->type);
	fkpspush<const char *>(fk, name);
}

// range
void buildin_range(fake * fk, interpreter * inter)
{
	// pos
	int pos = fkpspop<int>(fk);

	// container
	bool err = false;
	variant * v = 0;
	PS_POP_AND_GET(fk->ps, v);

	if (v->type == variant::STRING)
	{
		if (pos >= 0 && pos < (int)v->data.str->sz)
		{
			char data[2];
			data[0] = v->data.str->s[pos];
			data[1] = 0;
			fkpspush<const char *>(fk, data);
		}
		else
		{
			fkpspush<const char *>(fk, "");
		}
	}
	else if (v->type == variant::ARRAY)
	{
		if (pos >= 0 && pos < (int)ARRAY_SIZE(v->data.va->va) && ARRAY_GET(v->data.va->va, pos))
		{
			variant * ret = 0;
			PS_PUSH_AND_GET(fk->ps, ret);
			*ret = ARRAY_GET(v->data.va->va, pos)->t;
		}
		else
		{
			fkpspush<bool>(fk, false);
		}
	}
	else if (v->type == variant::MAP)
	{
		if (pos >= 0 && pos < (int)v->data.vm->vm.size())
		{
			variant * key = 0;
			PS_PUSH_AND_GET(fk->ps, key);
			
			variant * value = 0;
			PS_PUSH_AND_GET(fk->ps, value);
			
			const fkhashmap<variant, pool<variant>::node *>::ele * e = v->data.vm->vm.at(pos);
			*key = e->k;
			*value = (*e->t)->t;
		}
		else
		{
			fkpspush<bool>(fk, false);
			fkpspush<bool>(fk, false);
		}
	}
	else
	{
		fkpspush<bool>(fk, false);
	}
}

// dumpallfunc
void buildin_dumpallfunc(fake * fk, interpreter * inter)
{
	const char * str = fk->bin.dump().c_str();
	fkpspush<const char *>(fk, str);
}

// dumpfunc
void buildin_dumpfunc(fake * fk, interpreter * inter)
{
	const char * func = fkpspopcstrptr(fk);
	const char * str = fk->bin.dump(func).c_str();
	fkpspush<const char *>(fk, str);
}

// dumpfuncmap
void buildin_dumpfuncmap(fake * fk, interpreter * inter)
{
	const char * str = fk->fm.dump().c_str();
	fkpspush<const char *>(fk, str);
}

// dofile
void buildin_dofile(fake * fk, interpreter * inter)
{
	const char * file = fkpspopcstrptr(fk);
	bool ret = fkparse(fk, file);
	fkpspush<bool>(fk, ret);
}

// dostring
void buildin_dostring(fake * fk, interpreter * inter)
{
	const char * str = fkpspopcstrptr(fk);
	bool ret = fkparsestr(fk, str);
	fkpspush<bool>(fk, ret);
}

// getcurfile
void buildin_getcurfile(fake * fk, interpreter * inter)
{
	const char * str = fkgetcurfile(fk);
	fkpspush<const char *>(fk, str);
}

// getcurline
void buildin_getcurline(fake * fk, interpreter * inter)
{
	int line = fkgetcurline(fk);
	fkpspush<int>(fk, line);
}

// getcurfunc
void buildin_getcurfunc(fake * fk, interpreter * inter)
{
	const char * str = fkgetcurfunc(fk);
	fkpspush<const char *>(fk, str);
}

// getcurcallstack
void buildin_getcurcallstack(fake * fk, interpreter * inter)
{
	const char * str = fkgetcurcallstack(fk);
	fkpspush<const char *>(fk, str);
}

// geterror
void buildin_geterror(fake * fk, interpreter * inter)
{
	const char * str = fkerrorstr(fk);
	fkpspush<const char *>(fk, str);
}

// isfunc
void buildin_isfunc(fake * fk, interpreter * inter)
{
	const char * str = fkpspopcstrptr(fk);
	bool ret = fkisfunc(fk, str);
	fkpspush<bool>(fk, ret);
}

// tonumber
void buildin_tonumber(fake * fk, interpreter * inter)
{
	const char * str = fkpspopcstrptr(fk);
	int ret = atoi(str);
	fkpspush<int>(fk, ret);
}

void buildinfunc::openbasefunc()
{
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("print"), buildin_print);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("log"), buildin_log);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("array"), buildin_array);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr(MAP_FUNC_NAME), buildin_map);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("size"), buildin_size);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("range"), buildin_range);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("debug"), buildin_debug);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("typeof"), buildin_typeof);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("dumpallfunc"), buildin_dumpallfunc);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("dumpfunc"), buildin_dumpfunc);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("dumpfuncmap"), buildin_dumpfuncmap);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("dofile"), buildin_dofile);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("dostring"), buildin_dostring);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("getcurfile"), buildin_getcurfile);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("getcurline"), buildin_getcurline);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("getcurfunc"), buildin_getcurfunc);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("getcurcallstack"), buildin_getcurcallstack);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("geterror"), buildin_geterror);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("isfunc"), buildin_isfunc);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("tonumber"), buildin_tonumber);
}

void buildinfunc::openfilefunc()
{
	m_bifile.openfilefunc();
}

void buildinfunc::opennetfunc()
{
	m_bifnet.opennetfunc();
}

void buildinfunc::openosfunc()
{
	m_bifos.openosfunc();
}

void buildinfunc::openstringfunc()
{
	m_bifstring.openstringfunc();
}

void buildinfunc::openmathfunc()
{
	m_bifmath.openmathfunc();
}

buffer * buildinfunc::newbuffer(int size)
{
	return m_bifnet.newbuffer(size);
}
selector * buildinfunc::newselector()
{
	return m_bifnet.newselector();
}

void buildinfunc::setargv(int argc, const char *argv[])
{
	m_bifos.setargv(argc, argv);
}

int buildinfunc::get_argc() const
{
	return m_bifos.get_argc();
}

const char * buildinfunc::get_argv(int pos) const
{
	return m_bifos.get_argv(pos);
}

void buildinfunc::setseed(int seed)
{
	m_bifmath.setseed(seed);
}

int buildinfunc::getseed() const
{
	return m_bifmath.getseed();
}

void buildinfunc::set_print_func(fkprint func)
{
	m_fkprint = func;
}

fkprint buildinfunc::get_print_func() const
{
	return m_fkprint;
}
