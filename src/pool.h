#pragma once

#include "types.h"

// T需要原始C结构 支持memcpy memset
template <typename T>
struct pool
{
public:
    struct node
    {
        node * next;
        T t;
    };
    fake * m_fk;
	node * m_data;
};

#define POOL_DELETE(po) \
    while ((po).m_data != 0)\
    {\
        void * p = (po).m_data;\
        (po).m_data = (po).m_data->next;\
        safe_fkfree((po).m_fk, p);\
    }
    
#define POOL_CLEAR(po)

#define POOL_INI(po, fk) (po).m_fk = (fk)

#define POOL_GROW(po, node, n)\
    {\
        n = (node *)safe_fkmalloc((po).m_fk, sizeof(node));\
        memset(n, 0, sizeof(node));\
        POOL_PUSH(po, n);\
    }

#define POOL_EMPTY(po) (!(po).m_data)
    
#define POOL_PUSH(po, n)\
    (n)->next = (po).m_data;\
    (po).m_data = (n)

#define POOL_POP(po, n)\
    assert((po).m_data);\
    n = (po).m_data;\
    (po).m_data = (po).m_data->next

