#pragma once

#include "types.h"

// T需要原始C结构 支持memcpy memset
template<typename T>
struct array {
    fake *m_fk;
    T *m_data;
    uint32_t m_size;
    uint32_t m_max_size;
    char m_recurflag;
};

#define ARRAY_DELETE(array) safe_fkfree((array).m_fk, (array).m_data); \
    (array).m_data = 0
#define ARRAY_CLEAR(array) (array).m_size = 0
#define ARRAY_DEEP_CLEAR(array, T) (array).m_size = 0;\
    memset((array).m_data, 0, (array).m_max_size * sizeof(T))
#define ARRAY_INI(array, fk) (array).m_fk = (fk)
#define ARRAY_MAX_SIZE(array) ((array).m_max_size)
#define ARRAY_SIZE(array) ((array).m_size)
#define ARRAY_GROW(array, newsize, T) \
    { \
        assert((int)newsize > (int)(array).m_max_size); \
        T * newbuff = (T *)safe_fkmalloc((array).m_fk, (newsize * sizeof(T)), emt_array); \
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
#define ARRAY_RECUR(array) ((array).m_recurflag != 0)
#define ARRAY_ENTER(array) ((array).m_recurflag++)
#define ARRAY_LEAVE(array) ((array).m_recurflag--)
