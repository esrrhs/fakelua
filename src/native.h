#pragma once

#include "types.h"
#include "hashmap.h"

class func_native
{
    friend class native;
    friend class asmgen;
    friend class assembler;
    friend class machine;
public:
	force_inline func_native(fake * fk) : m_fk(fk), m_buff(0), m_size(0)
    {
    }
    
    String dump() const;
    
private:
    fake * m_fk;
    // Ãû×Ö
    const char * m_name;
    // code
    char * m_buff;
    size_t m_size;
};

class native
{
public:
    force_inline native(fake * fk) : m_fk(fk), m_shh(fk)
    {
    }
    force_inline ~native()
    {
    }

    force_inline fake * getfake()
    {
        return m_fk;
    }

    force_inline void clear()
    {
        m_shh.clear();
    }

    force_inline bool add_func(const char * name, const func_native & nt)
    {
        m_shh.add(name, nt);
        return true;
    }
    
    force_inline const func_native * get_func(const char * name) const
    {
        return m_shh.get(name);
    }
    
    String dump() const;
    
private:
    fake * m_fk;
    stringhashmap<func_native> m_shh;
};

