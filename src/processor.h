#pragma once

#include "types.h"
#include "routine.h"
#include "array.h"
#include "pool.h"

struct fake;
struct processor
{
public:

    routine * start_routine(binary * bin, const char * func, paramstack * ps);
    
    void run();

public:
    fake * m_fk;
    pool<routine> m_routine_pool;
    array<pool<routine>::node *> m_routine_list;
    size_t m_routine_num;
};

#define PROCESS_INI(pro, fk) (pro).m_fk = fk;\
    POOL_INI((pro).m_routine_pool, fk);\
    ARRAY_INI((pro).m_routine_list, fk);\

#define PROCESS_DELETE(pro) POOL_DELETE((pro).m_routine_pool);\
    ARRAY_DELETE((pro).m_routine_list)

#define PROCESS_CLEAR(pro) POOL_CLEAR((pro).m_routine_pool);\
    ARRAY_CLEAR((pro).m_routine_list);\
    (pro).m_routine_num = 0;

