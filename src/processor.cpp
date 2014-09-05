#include "processor.h"
#include "fuck.h"
#include "routine.h"

void processor::clear()
{
    if (m_routine_list)
    {
        m_fk->m_fkfree(m_routine_list);
    }
    m_routine_list = 0;
    m_routine_num = 0;
    m_routine_max_size = 0;
}

bool processor::empty() const
{
    return m_routine_num <= 0;
}

bool processor::full() const
{
    return m_routine_num == m_routine_max_size;
}

void processor::grow()
{
    size_t newsize = m_routine_max_size * 2;
    routine ** new_routine_list = (routine **)m_fk->m_fkmalloc(newsize * sizeof(routine *));
    if (m_routine_list)
    {
        memcpy(new_routine_list, m_routine_list, m_routine_max_size * sizeof(routine *));   
        m_fk->m_fkfree(m_routine_list);
    }
    m_routine_list = new_routine_list;
    m_routine_max_size = newsize;
}

void processor::add(routine * r)
{
    if (full())
    {
        grow();
    }
    
    m_routine_list[m_routine_num] = r;
    m_routine_num++;
}

bool processor::isend() const
{
    return empty();
}
    
int processor::run(int cmdnum)
{
    int curcmdnum = 0;

    int percmdnum = cmdnum / m_routine_num;
    if (percmdnum <= 0)
    {
        percmdnum = cmdnum;
    }
    
    while (!empty())
    {
        for (int i = 0; i < (int)m_routine_num; i++)
        {
            routine * r = m_routine_list[i];
            curcmdnum += r->run(percmdnum);
        }

        // delete
        int first = 0;
        for (; first < (int)m_routine_num; first++)
        {
            routine * r = m_routine_list[first];
            if (r->isend())
            {
                break;
            }
        }
        if (first == (int)m_routine_num)
        {
            continue;
        }

        int next = first;
        for (first++; first < (int)m_routine_num; first++)
        {
            routine * r = m_routine_list[first];
            if (!r->isend())
            {
                m_routine_list[next] = m_routine_list[first];
                next++;
            }
        }
        m_routine_num = next;
    }
    
    return curcmdnum;
}

