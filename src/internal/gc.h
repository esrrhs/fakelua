#pragma once

#include "types.h"
#include "gcobject.h"

class fakelua_state;

class gc {
public:
    gc(fakelua_state *l);

    ~gc();

    gc(const gc &) = delete;

    gc &operator=(const gc &) = delete;

public:
    void add(gcobject *gco);

private:
    fakelua_state *m_l = nullptr;
};

