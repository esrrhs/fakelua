#include "bindfunc.h"
#include "fake.h"
#include "fakescript.h"
#ifdef WIN32
#include <Windows.h>
#endif

// print, very slow
void buildinprint(fake * fk)
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
void buildinlog(fake * fk)
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
        const char * func = "todo";//fk->inter.get_running_func_name();
        
    	fprintf(pLog, "[%d.%d.%d, %d.%d.%d][%s] : %s\n", 
    		tptr->tm_year + 1990, tptr->tm_mon + 1, tptr->tm_mday, tptr->tm_hour, tptr->tm_min, tptr->tm_sec,
    		func, str.c_str());

    	fclose(pLog);
	}

    // ret
    fkpspush<int>(fk, (int)str.size());
}

// sleep
void buildinsleep(fake * fk)
{
    int millionseconds = fkpspop<int>(fk);
#if defined(WIN32)
	Sleep(millionseconds);
#else
	usleep(millionseconds * 1000);
#endif
    fkpspush<int>(fk, 0);
}

void buildinfunc::openbasefunc()
{
    m_shh.add("print", buildinprint);
    m_shh.add("log", buildinlog);
    m_shh.add("sleep", buildinsleep);
}

bool buildinfunc::call(const char * name)
{
    bifunc * p = m_shh.get(name);
    if (!p)
    {
        return false;
    }
    (*p)(m_fk);
    return true;
}
