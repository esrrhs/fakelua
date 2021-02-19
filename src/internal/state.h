#pragma once

#include "types.h"
#include "stringheap.h"

class fakelua_state {
public:
    fakelua_state();

    ~fakelua_state();

    fakelua_state(const fakelua_state &) = delete;

    fakelua_state &operator=(const fakelua_state &) = delete;

    stringheap &get_stringheap() { return m_sh; }

private:
    stringheap m_sh;
};
