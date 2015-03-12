#pragma once

#include "types.h"
#include "hashmap.h"

struct profilefuncele
{
    int callnum;
    uint32_t calltime;
};

class profile
{
public:
    force_inline profile(fake * fk) : m_fk(fk), m_isopen(false), m_shh(fk)
    {
    }
    force_inline ~profile()
    {
        clear();
    }

    force_inline fake * getfake()
    {
        return m_fk;
    }

    force_inline void clear()
    {
        m_isopen = false;
        m_shh.clear();
        m_dumpstr.clear();
    }

    force_inline void open()
    {
        m_isopen = true;
    }
    
    force_inline void close()
    {
        m_isopen = false;
    }
    
    force_inline bool isopen()
    {
        return m_isopen;
    }
    
    void add_func_sample(const char * func, uint32_t calltime);

    const char * dump();

private:
    fake * m_fk;
    bool m_isopen;
    typedef fkhashmap<const char *, profilefuncele> stringhashmap;
    stringhashmap m_shh;
    String m_dumpstr;
};

