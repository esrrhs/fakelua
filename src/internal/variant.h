#pragma once

#include "types.h"
#include "gcobject.h"
#include "object.h"
#include "fakelua.h"

class variant {
public:
    variant();

    variant(string_object *s);

    ~variant();

private:
    gcobject *m_gcobject = nullptr;
    
    union {
        // light userdata
        void *p;

        // booleans
        bool b;

        // light C functions
        fakelua_cfunction f;

        // integer numbers
        uint64_t i;

        // float numbers
        double n;

        // string
        string_object *s;
    } m_data;
};
