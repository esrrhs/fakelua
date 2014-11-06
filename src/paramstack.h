#pragma once

#include "types.h"
#include "variant.h"

#define REAL_MAX_FUCK_PARAM_NUM (MAX_FUCK_PARAM_NUM + MAX_FUCK_RETURN_NUM)

struct paramstack
{
    force_inline void clear()
    {
        m_variant_list_num = 0;
    }
    
	variant m_variant_list[REAL_MAX_FUCK_PARAM_NUM];
	size_t m_variant_list_num;
};

#define PS_PUSH_AND_GET(ps, v) \
    assert((ps).m_variant_list_num < REAL_MAX_FUCK_PARAM_NUM);\
    v = &(ps).m_variant_list[(ps).m_variant_list_num];\
    (ps).m_variant_list_num++;
    
#define PS_POP_AND_GET(ps, v) \
    assert((ps).m_variant_list_num > 0);\
    v = &(ps).m_variant_list[(ps).m_variant_list_num - 1];\
    (ps).m_variant_list_num--;
    