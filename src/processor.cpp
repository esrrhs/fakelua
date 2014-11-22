#include "processor.h"
#include "fake.h"

routine * processor::start_routine(binary * bin, const variant & func, paramstack * ps)
{
    pool<routine>::node * n = 0;
    POOLLIST_POP(m_pl, n, routine, m_fk->cfg.array_grow_speed);
    
    // 初始化下
    ROUTINE_INI(n->t, m_fk);
    ROUTINE_CLEAR(n->t);
    ROUTINE_SET_PRO(n->t, this);
    ROUTINE_ENTRY(n->t, bin, func, ps);
    if (ROUTINE_ISEND(n->t))
    {
        // 回滚
        POOLLIST_POP_ROLLBACK(m_pl, n);
        return &n->t;
    }
    
	m_routine_num++;
	
    return &n->t;
}
    
void processor::run()
{
    while (m_routine_num > 0)
    {
        for (int i = 0; i < (int)ARRAY_SIZE(m_pl.l); i++)
        {
            pool<routine>::node * n = ARRAY_GET(m_pl.l, i);
            if (!n)
            {
                continue;
            }
            // 注意:此函数内部可能会调用到add接口
            ROUTINE_RUN(n->t, m_fk->cfg.per_frame_cmd_num);
            if (ROUTINE_ISEND(n->t))
            {
                POOL_PUSH(m_pl.p, n);
                ARRAY_GET(m_pl.l, i) = 0;
                m_routine_num--;
            }
        }
    }
    ARRAY_CLEAR(m_pl.l);
}

