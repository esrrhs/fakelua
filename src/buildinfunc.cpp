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

// sleep
void buildin_sleep(fake * fk, interpreter * inter)
{
    int millionseconds = fkpspop<int>(fk);
#if defined(WIN32)
	Sleep(millionseconds);
#else
	usleep(millionseconds * 1000);
#endif
    fkpspush<int>(fk, 0);
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

// global map
void buildin_globalmap(fake * fk, interpreter * inter)
{
	bool err = false;
    variant_map * m = fk->con.get_gmap();
    variant * v = 0;
    PS_PUSH_AND_GET(fk->ps, v);
    V_SET_MAP(v, m);
}

// global map
void buildin_clearglobalmap(fake * fk, interpreter * inter)
{
    variant_map * m = fk->con.get_gmap();
    m->vm.clear();
    fkpspush<int>(fk, 0);
}

void buildinfunc::openbasefunc()
{
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("print"), buildin_print);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("log"), buildin_log);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("sleep"), buildin_sleep);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("array"), buildin_array);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr(MAP_FUNC_NAME), buildin_map);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("G"), buildin_globalmap);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("CG"), buildin_clearglobalmap);
}
