#pragma once

#include "types.h"
#include "hashmap.h"

struct bindfuncele
{
    fkfunctor ff;
    uint32_t callnum;
};

class bindfunc
{
public:
    force_inline bindfunc(fuck * fk) : m_fk(fk), m_shh(fk)
    {
    }
    force_inline ~bindfunc()
    {
    }

    force_inline fuck * getfuck()
    {
        return m_fk;
    }

    force_inline void clear()
    {
    }

    void addfunc(const char * name, const fkfunctor & ff);

    // 参数和返回值都在m_fk->ps里
    bool call(const char * name)
    {
        bindfuncele * p = m_shh.get(name);
        if (!p)
        {
            return false;
        }

        p->ff.ff(m_fk, &p->ff);

        p->callnum++;
        
        return true;
    }
    
private:
    fuck * m_fk;
    stringhashmap<bindfuncele> m_shh;
};

