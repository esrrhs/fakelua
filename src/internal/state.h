#pragma once

#include "types.h"
#include "stringheap.h"
#include "gc.h"
#include "config.h"

class fakelua_state {
public:
    fakelua_state();

    ~fakelua_state();

    fakelua_state(const fakelua_state &) = delete;

    fakelua_state &operator=(const fakelua_state &) = delete;

    stringheap &get_stringheap() { return m_sh; }

    gc &get_gc() { return m_gc; }

    config &get_config() { return m_config; }

private:
    config m_config;
    stringheap m_sh;
    gc m_gc;
};
