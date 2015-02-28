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
    printf("%s\n", str.c_str());
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
    
    // ret
    fkpspush<int>(fk, 1);
}

void buildinfunc::openbasefunc()
{
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("print"), buildin_print);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("log"), buildin_log);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("array"), buildin_array);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr(MAP_FUNC_NAME), buildin_map);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("size"), buildin_size);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("debug"), buildin_debug);
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

