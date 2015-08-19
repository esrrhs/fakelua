#pragma once

#include "types.h"

// T需要原始C结构 支持memcpy memset
template <typename T>
struct pool
{
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

	
template <typename T>
struct poollist
{
	pool<T> p;
	array<typename pool<T>::node *> l;
};

#define POOLLIST_INI(pl, fk) POOL_INI((pl).p, fk);\
	ARRAY_INI((pl).l, fk)

#define POOLLIST_DELETE(pl) POOL_DELETE((pl).p);\
	ARRAY_DELETE((pl).l)

#define POOLLIST_CLEAR(pl, T, CL) \
	for (int i = 0; i < (int)ARRAY_SIZE((pl).l); i++)\
	{\
		pool<T>::node * n = ARRAY_GET((pl).l, i);\
		CL;\
		POOL_PUSH((pl).p, n);\
	}\
	POOL_CLEAR((pl).p);\
	ARRAY_CLEAR((pl).l)
	
#define POOLLIST_POP(pl, n, T, growspeed)\
	if (POOL_EMPTY((pl).p))\
	{\
		POOL_GROW((pl).p, pool<T>::node, n);\
	}\
	POOL_POP((pl).p, n);\
	if (ARRAY_SIZE((pl).l) >= ARRAY_MAX_SIZE((pl).l))\
	{\
		size_t newsize = ARRAY_MAX_SIZE((pl).l) + 1 + ARRAY_MAX_SIZE((pl).l) * (growspeed) / 100;\
		ARRAY_GROW((pl).l, newsize, pool<T>::node *);\
	}\
	ARRAY_PUSH_BACK((pl).l);\
	ARRAY_BACK((pl).l) = n
	