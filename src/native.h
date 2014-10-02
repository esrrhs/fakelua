#pragma once

#include "types.h"

class func_native
{
    friend class native;
    friend class asmgen;
    friend class assembler;
    friend class machine;
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

    force_inline bool set_func(int pos, func_native & nt)
    {
        while (pos >= (int)m_func_native_list.size())
        {
            m_func_native_list.push_back(func_native(m_fk));
        }
        
        m_func_native_list[pos] = nt;
        return true;
    }
    
    force_inline const func_native * get_func(int pos) const
    {
        assert(pos >= 0 && pos < (int)m_func_native_list.size());
        return &m_func_native_list[pos];
    }
    
    String dump() const;
    
private:
    fuck * m_fk;
    func_native_list m_func_native_list;
};

