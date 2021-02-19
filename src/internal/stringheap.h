#pragma once

#include "types.h"

class fakelua_state;

class stringheap {
public:
    stringheap(fakelua_state *l);

    ~stringheap();

    stringheap(const stringheap &) = delete;

    stringheap &operator=(const stringheap &) = delete;

public:

private:
    fakelua_state *m_l = nullptr;
};
