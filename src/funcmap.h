#pragma once

#include "types.h"
#include "hashmap.h"

struct func_ele
{
    bool isok;
    int pos;
};

class funcmap
{
public:
    force_inline funcmap(fuck * fk) : m_fk(fk), m_shh(fk)
    {
    }
    force_inline ~funcmap()
    {
    }

    force_inline fuck * getfuck()
    {
        return m_fk;
    }

    force_inline void clear()
    {
        m_shh.clear();
    }

    force_inline bool checkok()
    {
        for (stringhashmap<func_ele>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
        {
            if (!p->t.isok)
            {
                seterror(m_fk, efk_compile_func_not_define, "compile func %s not define", p->s);
                return false;
            }
        }
        return true;
    }
    
    force_inline bool is_have_func(const char * name) const
    {
        return m_shh.get(name) != 0;
    }
    force_inline int reg_func(const String & name)
    {
        if (is_have_func(name.c_str()))
        {
            return -1;
        }

        func_ele e;
        e.pos = m_shh.size();
        e.isok = false;
        m_shh.add(name.c_str(), e);

        return e.pos;
    }

    force_inline bool reg_func_ok(const String & name)
    {
        func_ele * p = m_shh.get(name.c_str());
        if (p)
        {
            p->isok = true;
            return true;
        }
        return false;
    }
    
    force_inline int get_func(const char * name) const
    {
        func_ele * p = m_shh.get(name);
        if (p)
        {
            // find it
            return p->pos;
        }
        return -1;
    }

private:
    fuck * m_fk;
    stringhashmap<func_ele> m_shh;
};

