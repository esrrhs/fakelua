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

typedef std::pair<String, profilefuncele> sortele;
struct profilefuncelesort
{
    bool operator()(const sortele & l, const sortele & r)
    {
        return l.second.calltime > r.second.calltime;
    }
};

const char * profile::dump()
{
    std::vector<sortele> sortelevec;
    for (const stringhashmap::ele * p = m_shh.first(); p != 0; p = m_shh.next())
    {
        const profilefuncele & ele = *p->t;
        sortelevec.push_back(std::make_pair(p->k, ele));
    }

    std::sort(sortelevec.begin(), sortelevec.end(), profilefuncelesort());

    m_dumpstr.clear();
    for (int i = 0; i < (int)sortelevec.size(); i++)
    {
        const sortele & se = sortelevec[i];
        const profilefuncele & ele = se.second;
        char buff[1024];
		tsnprintf(buff, sizeof(buff)-1, "Func[%s]\tCalls[%d]\tTotalTime(ms)[%u]\tPerCallTime(ms)[%u]\n",
			se.first.c_str(), ele.callnum, ele.calltime, ele.callnum ? ele.calltime / ele.callnum : 0);
        m_dumpstr += buff;
    }
    
    return m_dumpstr.c_str();
}
