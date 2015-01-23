#include "profile.h"
#include "fake.h"

void profile::add_func_sample(const char * func, uint32_t calltime)
{
    profilefuncele * p = m_shh.get(func);
    if (!p)
    {
        profilefuncele tmp;
        tmp.callnum = 1;
        tmp.calltime = calltime;
        m_shh.add(func, tmp);
        return;
    }
    
    p->callnum++;
    p->calltime += calltime;
}

const char * profile::dump()
{
    m_dumpstr.clear();
    for (const stringhashmap<profilefuncele>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
    {
        const profilefuncele & ele = p->t;
        char buff[1024];
		tsnprintf(buff, sizeof(buff)-1, "Func[%s]\tCalls[%d]\tTotalTime(ms)[%u]\tPerCallTime(ms)[%u]\n",
			p->s, ele.callnum, ele.calltime, ele.callnum ? ele.calltime / ele.callnum : 0);
        m_dumpstr += buff;
    }
    return m_dumpstr.c_str();
}
