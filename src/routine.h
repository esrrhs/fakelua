#pragma once

#include "types.h"
#include "variant.h"
#include "interpreter.h"

struct fuck;
class binary;
class paramstack;
class routine
{
public:
    force_inline routine(fuck * fk) : m_fk(fk), m_bin(0), m_ps(0), m_interpreter(fk)
    {
    }

    force_inline void entry(binary * bin, const char * func, paramstack * ps)
    {
        m_bin = bin;
        m_ps = ps;
        m_interpreter.call(bin, func, ps);
        if (isend())
        {
            m_ret = m_interpreter.getret();
        }
    }

    force_inline const variant & getret() const
    {
        return m_ret;
    }

    force_inline bool isend() const
    {
        return m_interpreter.isend();
    }
    
    force_inline int run(int cmdnum)
    {
        if (isend())
        {   
            m_ret = m_interpreter.getret();
            return 0;
        }
        
        int runcmdnum = m_interpreter.run(cmdnum);

        if (isend())
        {   
            m_ret = m_interpreter.getret();
        }

        return runcmdnum;
    }    


private:
    fuck * m_fk;
    binary * m_bin;
    paramstack * m_ps;
    variant m_ret;
    // ½âÊÍÆ÷
    interpreter m_interpreter;
};

