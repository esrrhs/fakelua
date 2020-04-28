#pragma once

#include "types.h"

// T需要原始C结构 支持memcpy memset
template<typename T>
struct pool {
    fake *m_fk;
    uint32_t m_growsize;
    array<T *> m_free;
};

#define POOL_DELETE(po, T, DL) \
    {\
        pool<T> & pol = (po);\
        for (int i = 0; i < (int)ARRAY_MAX_SIZE(pol.m_free); i++)\
        {\
            T * n = ARRAY_GET(pol.m_free, i);\
            if (LIKE(n != 0))\
            {\
                DL;\
                safe_fkfree(pol.m_fk, n);\
            }\
        }\
        ARRAY_DELETE(pol.m_free);\
    }

#define POOL_CLEAR(po)

#define POOL_INI(po, fk) (po).m_fk = (fk);\
    ARRAY_INI((po).m_free, fk)

#define POOL_GROW(po, n, T)\
    {\
        n = (T *)safe_fkmalloc((po).m_fk, sizeof(T), emt_pool);\
        memset(n, 0, sizeof(T));\
        POOL_PUSH(po, n, T);\
        (po).m_growsize++;\
    }

#define POOL_GROW_SIZE(po) ((po).m_growsize)

#define POOL_EMPTY(po) (ARRAY_EMPTY((po).m_free))

#define POOL_PUSH(po, n, T)\
    if (UNLIKE(ARRAY_SIZE((po).m_free) >= ARRAY_MAX_SIZE((po).m_free)))\
    {\
        size_t newsize = ARRAY_SIZE((po).m_free) + 1 + ARRAY_MAX_SIZE((po).m_free) * (get_fakeconfig((po).m_fk).array_grow_speed) / 100;\
        ARRAY_GROW((po).m_free, newsize, T *);\
    }\
    assert(n);\
    ARRAY_PUSH_BACK((po).m_free);\
    ARRAY_BACK((po).m_free) = n

#define POOL_POP(po, n)\
    assert(!(ARRAY_EMPTY((po).m_free)));\
    n = ARRAY_BACK((po).m_free);\
    assert(n);\
    ARRAY_POP_BACK((po).m_free)


template<typename T>
struct poollist {
    pool<T> p;
    array<T *> l;
};

#define POOLLIST_INI(pl, fk) POOL_INI((pl).p, fk);\
    ARRAY_INI((pl).l, fk)

#define POOLLIST_DELETE(pl, T, DL) POOL_DELETE((pl).p, T, DL);\
    ARRAY_DELETE((pl).l)

#define POOLLIST_CLEAR(pl, T, CL) \
    for (int i = 0; i < (int)ARRAY_SIZE((pl).l); i++)\
    {\
        T * n = ARRAY_GET((pl).l, i);\
        CL;\
        POOL_PUSH((pl).p, n, T);\
    }\
    POOL_CLEAR((pl).p);\
    ARRAY_CLEAR((pl).l)

#define POOLLIST_POP(pl, n, T)\
    if (POOL_EMPTY((pl).p))\
    {\
        POOL_GROW((pl).p, n, T);\
    }\
    POOL_POP((pl).p, n);\
    if (ARRAY_SIZE((pl).l) >= ARRAY_MAX_SIZE((pl).l))\
    {\
        size_t newsize = ARRAY_SIZE((pl).l) + 1 + ARRAY_MAX_SIZE((pl).l) * (get_fakeconfig((pl).l.m_fk).array_grow_speed) / 100;\
        ARRAY_GROW((pl).l, newsize, T *);\
    }\
    ARRAY_PUSH_BACK((pl).l);\
    ARRAY_BACK((pl).l) = n
	