#pragma once

#include "types.h"
#include "variant.h"

#define REAL_MAX_FAKE_PARAM_NUM ((MAX_FAKE_PARAM_NUM + MAX_FAKE_RETURN_NUM) * 2)

struct paramstack
{
    force_inline void clear()
    {
        m_variant_list_num = 0;
    }
    
	variant m_variant_list[REAL_MAX_FAKE_PARAM_NUM];
	size_t m_variant_list_num;
};

#define PS_PUSH_AND_GET(ps, v) \
    if (LIKE((ps).m_variant_list_num < REAL_MAX_FAKE_PARAM_NUM)) \
    { \
        v = &(ps).m_variant_list[(ps).m_variant_list_num]; \
        (ps).m_variant_list_num++; \
    } \
    else \
    { \
	    err = true; \
	    seterror(fk, efk_run_param_error, "push fail, param stack is full %d", REAL_MAX_FAKE_PARAM_NUM); \
        v = &(ps).m_variant_list[REAL_MAX_FAKE_PARAM_NUM - 1]; \
        USE(err); \
    }
    
#define PS_POP_AND_GET(ps, v) \
    if (LIKE((ps).m_variant_list_num > 0)) \
    { \
        v = &(ps).m_variant_list[(ps).m_variant_list_num - 1]; \
        (ps).m_variant_list_num--; \
    } \
    else \
    { \
	    err = true; \
	    seterror(fk, efk_run_param_error, "pop fail, param stack is empty"); \
        v = &(ps).m_variant_list[0]; \
        USE(err); \
    }
    