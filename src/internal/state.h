#pragma once

#include "types.h"
#include "stringheap.h"
#include "gc.h"
#include "config.h"
#include "err.h"

class fakelua_state {
public:
    fakelua_state();

    virtual ~fakelua_state();

    fakelua_state(const fakelua_state &) = delete;

    fakelua_state &operator=(const fakelua_state &) = delete;

public:
    stringheap &get_stringheap();

    gc &get_gc();

    config &get_config();

    void throw_err(const err &e);

private:
    config m_config;
    stringheap m_sh;
    gc m_gc;
};
