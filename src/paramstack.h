#pragma once

#include "types.h"
#include "variant.h"

#define REAL_MAX_FAKE_PARAM_NUM ((MAX_FAKE_PARAM_NUM + MAX_FAKE_RETURN_NUM) * 2)

struct paramstack {
    size_t m_variant_list_num;
    variant m_variant_list[REAL_MAX_FAKE_PARAM_NUM];
};

#define PS_CLEAR(ps) (ps).m_variant_list_num = 0;

#define PS_PUSH_AND_GET(ps, v) \
    if (LIKE((ps).m_variant_list_num < REAL_MAX_FAKE_PARAM_NUM)) \
    { \
        v = &(ps).m_variant_list[(ps).m_variant_list_num]; \
        (ps).m_variant_list_num++; \
    } \
    else \
    { \
        err = true; \
        seterror(fk, efk_run_param_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk), "push fail, param stack is full %d", REAL_MAX_FAKE_PARAM_NUM); \
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
        seterror(fk, efk_run_param_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk), "pop fail, param stack is empty"); \
        v = &(ps).m_variant_list[0]; \
        USE(err); \
    }

#define PS_GET(ps, v, i) \
    if (LIKE((i) >= 0 && (int)(i) < (int)(ps).m_variant_list_num)) \
    { \
        v = &(ps).m_variant_list[(i)]; \
    } \
    else \
    { \
        err = true; \
        seterror(fk, efk_run_param_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk), "get fail, i %d, stack len %d", i, (ps).m_variant_list_num); \
        v = &(ps).m_variant_list[0]; \
        USE(err); \
    }

#define PS_SIZE(ps) ((ps).m_variant_list_num)
	