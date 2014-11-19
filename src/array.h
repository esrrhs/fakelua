#pragma once

#include "types.h"

// T需要原始C结构 支持memcpy memset
template <typename T>
struct array
{
public:
	force_inline array(fake * fk) : m_fk(fk), m_data(0), m_size(0), m_max_size(0)
	{

	}
	force_inline ~array()
	{
		safe_fkfree(m_fk, m_data);
	}
	
public:
    fake * m_fk;
	T* m_data;
	size_t m_size;
	size_t m_max_size;
};

#define ARRAY_CLEAR(array) (array).m_size = 0
#define ARRAY_SET_FK(array, fk) (array).m_fk = (fk)
#define ARRAY_MAX_SIZE(array) ((array).m_max_size)
#define ARRAY_SIZE(array) ((array).m_size)
#define ARRAY_GROW(array, newsize, T) \
    { \
    	assert(newsize > (array).m_max_size); \
    	T * newbuff = (T *)safe_fkmalloc((array).m_fk, (newsize * sizeof(T))); \
    	if ((array).m_data) \
    	{ \
    		memcpy(newbuff, (array).m_data, (array).m_max_size * sizeof(T)); \
    	} \
    	memset(&newbuff[(array).m_max_size], 0, (newsize - (array).m_max_size) * sizeof(T)); \
    	safe_fkfree((array).m_fk, (array).m_data); \
    	(array).m_data = newbuff; \
    	(array).m_max_size = newsize; \
    }
    
#define ARRAY_GET(array, i) ((array).m_data[(i)])
#define ARRAY_BACK(array) ((array).m_data[(array).m_size - 1])
#define ARRAY_POP_BACK(array) (array).m_size--
#define ARRAY_EMPTY(array) (!(array).m_size)
#define ARRAY_PUSH_BACK(array) (array).m_size++
