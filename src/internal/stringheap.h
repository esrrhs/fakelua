#pragma once

#include "types.h"
#include "variant.h"

class fakelua_state;

class stringheap {
public:
    stringheap(fakelua_state *l);

    ~stringheap();

    stringheap(const stringheap &) = delete;

    stringheap &operator=(const stringheap &) = delete;

public:
    variant new_string(const std::string &str);

private:
    variant new_short_string(const std::string &str);
    variant new_long_string(const std::string &str);

private:
    fakelua_state *m_l = nullptr;
};
