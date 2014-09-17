#include "processor.h"
#include "fuck.h"
#include "routine.h"

void processor::grow()
{
	size_t newsize = m_routine_max_size + 1 + m_routine_max_size * m_fk->m_routine_grow_speed / 100;
	assert(newsize > m_routine_max_size);
	routine ** new_routine_list = (routine **)m_fk->m_fkmalloc(newsize * sizeof(routine *));
    if (m_routine_list)
    {
        memcpy(new_routine_list, m_routine_list, m_routine_max_size * sizeof(routine *));   
        m_fk->m_fkfree(m_routine_list);
    }
	memset(new_routine_list + m_routine_max_size, 0, (newsize - m_routine_max_size) * sizeof(routine *));
    m_routine_list = new_routine_list;
    m_routine_max_size = newsize;
}

int processor::run(int cmdnum)
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

void processor::checkdelete()
{
    // 大于1/4才开始清除无效的
    if (m_invalid_routine_num * m_fk->m_per_frame_cmd_num < m_routine_num)
    {
        return;
    }

    // delete
    int first = 0;
    for (; first < (int)m_routine_num; first++)
    {
        routine * r = m_routine_list[first];
        if (!r)
        {
            break;
        }
    }
    if (first == (int)m_routine_num)
    {
        return;
    }

    int next = first;
    for (first++; first < (int)m_routine_num; first++)
    {
        routine * r = m_routine_list[first];
        if (r)
        {
            m_routine_list[next] = m_routine_list[first];
            next++;
        }
    }
    m_routine_num = next;
}
