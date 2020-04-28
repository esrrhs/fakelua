#pragma once

#include "types.h"

struct running {
    running(fake *fk) : m_fk(fk), rundeps(0), stepmod(false) {
        ARRAY_INI(curprocessor, fk);
    }

    ~running() {
    }

    void clear() {
        rundeps = 0;
        ARRAY_CLEAR(curprocessor);
        cur_runinginfo.clear();
        stepmod = false;
        curcode.clear();
        ARRAY_DELETE(curprocessor);
    }

    fake *m_fk;
    // 执行迭代计数
    int rundeps;
    // 当前执行的processor
    array<processor *> curprocessor;
    // running info
    String cur_runinginfo;
    // step mod
    bool stepmod;
    // 当前执行代码
    String curcode;
};

#define GET_CUR_PROCESSOR(pro, ri) if (UNLIKE(ARRAY_EMPTY((ri).curprocessor))) \
    { \
        pro = 0; \
    } \
    else \
    { \
        pro = ARRAY_BACK((ri).curprocessor); \
    }

#define PUSH_CUR_PROCESSOR(pro, ri) if (UNLIKE(ARRAY_SIZE((ri).curprocessor) >= ARRAY_MAX_SIZE((ri).curprocessor))) \
    { \
        int newsize = ARRAY_SIZE((ri).curprocessor) + 1 + ARRAY_MAX_SIZE((ri).curprocessor) * (ri).m_fk->cfg.array_grow_speed / 100; \
        ARRAY_GROW((ri).curprocessor, newsize, processor *); \
    } \
    ARRAY_PUSH_BACK((ri).curprocessor); \
    ARRAY_BACK((ri).curprocessor) = pro;

#define POP_CUR_PROCESSOR(ri) assert(!ARRAY_EMPTY((ri).curprocessor)); \
    ARRAY_POP_BACK((ri).curprocessor); \


