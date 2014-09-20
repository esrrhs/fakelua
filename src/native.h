#pragma once

#include "types.h"

class func_native
{
    friend class asmgen;
    friend class assembler;
public:
	force_inline func_native(fuck * fk) : m_fk(fk), m_buff(0), m_size(0)
    {
    }
    
    String dump() const;
    
private:
    fuck * m_fk;
    // Ãû×Ö
    String m_name;
    // code
    char * m_buff;
    size_t m_size;
};

typedef std::vector<func_native> func_native_list;

class native
{
public:
    force_inline native(fuck * fk) : m_fk(fk)
    {
    }
    force_inline ~native()
    {
    }

    force_inline fuck * getfuck()
    {
        return m_fk;
    }

    force_inline void clear()
    {
    }

    force_inline bool add_func(func_native & nt)
    {
        m_func_native_list.push_back(nt);
        return true;
    }
    
    String dump() const;
    
private:
    fuck * m_fk;
    func_native_list m_func_native_list;
};

