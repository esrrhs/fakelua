#pragma once

#include "types.h"
#include "gcobject.h"
#include "object.h"
#include "fakelua.h"

class variant {
public:
    variant();

    variant(string_object *s);

    virtual ~variant();

private:
    // void * == light userdata
    // bool == booleans
    // fakelua_cfunction == light C functions
    // uint64_t == integer numbers
    // double == float numbers
    // string_object * == string
    std::variant<void *, bool, fakelua_cfunction, uint64_t, double, string_object *> m_data;
};
