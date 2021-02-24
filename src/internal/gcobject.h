#pragma once

#include "types.h"

enum TYPE {
    VT_NONE,

    VT_NIL,
    VT_BOOLEAN,
    VT_LIGHTUSERDATA,
    VT_NUMBER,
    VT_STRING,
    VT_TABLE,
    VT_FUNCTION,
    VT_USERDATA,
    VT_THREAD,

    VT_MAX,
};

enum FUNCTION_TYPE {
    VT_FT_LUA_CLOSURE, // Lua closure
    VT_FT_LIGHT_C_FUNCTION, // light C function
    VT_FT_C_CLOSURE, // C closure
};

enum STRING_TYPE {
    VT_ST_SHORT_STRINGS, // short strings
    VT_ST_LONG_STRINGS, // long strings
};

enum NUMBERS_TYPE {
    VT_NT_FLOAT_NUMBERS, // float numbers
    VT_NT_INTEGER_NUMBERS, // integer numbers
};

class fakelua_state;

class gcobject {
public:
    gcobject(fakelua_state *L, int type, int subtype);

    virtual ~gcobject();

private:
    gcobject *m_next = nullptr;
    int m_type = 0;
    int m_subtype = 0;
    int m_mark = 0;
};

