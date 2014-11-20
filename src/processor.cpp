#include "processor.h"
#include "fake.h"

routine * processor::start_routine(binary * bin, const char * func, paramstack * ps)
{
    // 池子够不够
    pool<routine>::node * n = 0;
    if (POOL_EMPTY(m_routine_pool))
	{
        POOL_GROW(m_routine_pool, pool<routine>::node, n);
        ROUTINE_INI(n->t, m_fk);
    }

    // 分配
    POOL_POP(m_routine_pool, n);
    assert(n);
    
    // 初始化下
    ROUTINE_CLEAR(n->t);
    ROUTINE_SET_PRO(n->t, this);
    ROUTINE_ENTRY(n->t, bin, func, ps);
    if (ROUTINE_ISEND(n->t))
    {
        POOL_PUSH(m_routine_pool, n);
        return &n->t;
    }
    
    // 添加
    if (ARRAY_SIZE(m_routine_list) >= ARRAY_MAX_SIZE(m_routine_list))
	{
	    size_t newsize = ARRAY_MAX_SIZE(m_routine_list) + 1 + ARRAY_MAX_SIZE(m_routine_list) * m_fk->cfg.routine_grow_speed / 100;
        ARRAY_GROW(m_routine_list, newsize, pool<routine>::node *);
    }
	ARRAY_PUSH_BACK(m_routine_list);
	ARRAY_BACK(m_routine_list) = n;
	m_routine_num++;
	
    return &n->t;
}
    
void processor::run()
{
    while (m_routine_num > 0)
    {
        for (int i = 0; i < (int)ARRAY_SIZE(m_routine_list); i++)
        {
            pool<routine>::node * n = ARRAY_GET(m_routine_list, i);
            if (!n)
            {
                continue;
            }
            // 注意:此函数内部可能会调用到add接口
            ROUTINE_RUN(n->t, m_fk->cfg.per_frame_cmd_num);
            if (ROUTINE_ISEND(n->t))
            {
                POOL_PUSH(m_routine_pool, n);
                ARRAY_GET(m_routine_list, i) = 0;
                m_routine_num--;
            }
        }
    }
    ARRAY_CLEAR(m_routine_list);
}
