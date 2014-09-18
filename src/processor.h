#pragma once

#include "types.h"
#include "routine.h"

struct fuck;
class routine;

typedef std::list<routine *> routine_list;

class processor
{
public:
    force_inline processor(fuck * fk) : m_fk(fk), m_routine_list(0), 
        m_routine_num(0),
        m_routine_max_size(0),
        m_invalid_routine_num(0)
    {
    }
    force_inline ~processor()
    {
        safe_fkfree(m_fk, m_routine_list);
    }

    force_inline void add(routine * r)
    {
        if (full())
        {
            grow();
        }
        
        m_routine_list[m_routine_num] = r;
        m_routine_num++;
    }

    force_inline int run(int cmdnum)
    {
        int curcmdnum = 0;

        int percmdnum = cmdnum / m_routine_num;
        if (percmdnum <= 0)
        {
            percmdnum = cmdnum;
        }
        
        m_invalid_routine_num = 0;
        for (int i = 0; i < (int)m_routine_num; i++)
        {
            routine * r = m_routine_list[i];
            if (!r)
            {
                m_invalid_routine_num++;
                continue;
            }
            // 注意:此函数内部可能会调用到add接口
            curcmdnum += r->run(percmdnum);
            if (r->isend())
            {
                m_routine_list[i] = 0;
                m_invalid_routine_num++;
            }
        }

        checkdelete();
        
        return curcmdnum;
    }

    force_inline bool isend() const
    {
        return empty();
    }
    
private:
    force_inline bool empty() const
    {
        return m_routine_num <= 0;
    }
    force_inline bool full() const
    {
        return m_routine_num == m_routine_max_size;
    }

    void grow();
    void checkdelete();
private:
    fuck * m_fk;
    routine ** m_routine_list;
    size_t m_routine_num;
    size_t m_routine_max_size;
    size_t m_invalid_routine_num;
};

