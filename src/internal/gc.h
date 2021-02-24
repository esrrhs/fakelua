#pragma once

#include "types.h"
#include "gcobject.h"

class fakelua_state;

class gc {
public:
    gc(fakelua_state *l);

    virtual ~gc();

    gc(const gc &) = delete;

    gc &operator=(const gc &) = delete;

public:
    gcobject *add(std::unique_ptr<gcobject> gco);

private:
    fakelua_state *m_l = nullptr;
    std::list<std::unique_ptr<gcobject>> m_gcobject_list;
};
