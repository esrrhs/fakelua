#pragma once

#include "types.h"
#include "hashmap.h"

struct bindfuncele
{
    fkfunctor ff;
    uint32_t callnum;
};

struct interpreter;
class bindfunc
{
public:
    force_inline bindfunc(fake * fk) : m_fk(fk), m_shh(fk)
    {
    }
    force_inline ~bindfunc()
    {
        clear();
    }

    force_inline fake * getfake()
    {
        return m_fk;
    }

    force_inline void clear()
    {
        m_shh.clear();
    }

    void addfunc(const char * name, const fkfunctor & ff);

    // 参数和返回值都在m_fk->ps里
    bool call(interpreter * inter, const char * name)
    {
        bindfuncele * p = m_shh.get(name);
        if (!p)
        {
            return callbuildin(inter, name);
        }

        p->ff.ff(m_fk, &p->ff);

        p->callnum++;
        
        return true;
    }
    
private:
    bool callbuildin(interpreter * inter, const char * name);
    
private:
    fake * m_fk;
    stringhashmap<bindfuncele> m_shh;
};

