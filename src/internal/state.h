#pragma once

#include "types.h"

class fakelua_state {
public:
    fakelua_state();

    ~fakelua_state();

    fakelua_state(const fakelua_state &) = delete;

    fakelua_state &operator=(const fakelua_state &) = delete;

};
