#pragma once

#include "types.h"
#include "gcobject.h"
#include "object.h"

class variant {
public:
    variant();

    ~variant();

private:
    gcobject m_gcobject;
    union {
        // light userdata
        void *p;

        // booleans
        bool b;

        // light C functions
        // TODO

        // integer numbers
        uint64_t i;

        // float numbers
        double n;

        // string
        string_object *s;
    } m_data;
};
