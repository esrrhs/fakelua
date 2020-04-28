#pragma once

#include "types.h"
#include "routine.h"
#include "array.h"
#include "pool.h"

struct fake;

struct processor {
public:

    routine *start_routine(const variant &func, int retnum, int *retpos);

    void run();

    force_inline int get_routine_num() const {
        return m_routine_num;
    }

    routine *get_routine_by_id(int id);

    routine *get_routine_by_index(int index);

    const char *get_routine_info();

    const char *get_routine_info_by_index(int index);

    const char *get_routine_info_by_id(int id);

public:
    fake *m_fk;
    poollist<routine> m_pl;
    uint32_t m_routine_num;
    routine *m_curroutine;
    int m_lastroutine;
    int m_lastroutine_runnum;
    routine *m_entryroutine;
    int m_genid;
};

#define PROCESS_INI(pro, fk) (pro).m_fk = fk;\
    POOLLIST_INI((pro).m_pl, fk);\
    (pro).m_routine_num = 0;\
    (pro).m_curroutine = 0;\
    (pro).m_lastroutine = 0;\
    (pro).m_lastroutine_runnum = 0;\
    (pro).m_entryroutine = 0;\
    (pro).m_genid = 0

#define PROCESS_DELETE(pro) \
        POOLLIST_DELETE((pro).m_pl, routine, ROUTINE_DELETE(*n))

#define PROCESS_CLEAR(pro) POOLLIST_CLEAR((pro).m_pl, routine, USE(n));\
    (pro).m_routine_num = 0;\
    (pro).m_curroutine = 0;\
    (pro).m_lastroutine = 0;\
    (pro).m_lastroutine_runnum = 0;\
    (pro).m_entryroutine = 0;\
    (pro).m_genid = 0

#define PROCESS_END(pro) (!(pro).m_routine_num)

