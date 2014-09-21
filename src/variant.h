#pragma once

#include "types.h"

struct stringbuf;
struct variant 
{
	enum Type 
	{
		NIL,
		REAL,
		STRING,
		POINTER,
	};	
    
	union MemData
	{
		double real;
		stringbuf * str;
		void * ptr;
		int64_t buf;	// 只是用作64位传递 
	};

	Type type;
	MemData data;
};

#define variant_size sizeof(variant)
#define variant_type_off ((size_t)(size_t*)&((variant*)0)->type)
#define variant_data_off ((size_t)(size_t*)&((variant*)0)->data)

#define V_SET_POINTER(v, p) \
    v->type = variant::POINTER;\
    v->data.ptr = p;
    
#define V_SET_REAL(v, r) \
    v->type = variant::REAL;\
    v->data.real = r;

#define V_SET_STRING(v, s) \
    v->type = variant::STRING;\
    v->data.str = 0;    // TODO

#define V_GET_POINTER(v, p) \
    p = v->data.ptr;
    
#define V_GET_REAL(v, r) \
    r = v->data.real;
    
#define V_GET_STRING(v, s) \
    s = 0;    // TODO

#define V_PLUS(d, l, r) d->data.real = l->data.real + r->data.real
#define V_MINUS(d, l, r) d->data.real = l->data.real - r->data.real
#define V_MULTIPLY(d, l, r) d->data.real = l->data.real * r->data.real
#define V_DIVIDE(d, l, r) d->data.real = l->data.real / r->data.real
#define V_DIVIDE_MOD(d, l, r) d->data.real = (int64_t)l->data.real % ((int64_t)r->data.real)
#define V_AND(d, l, r) d->data.real = (l->data.real != 0) & (r->data.real != 0)
#define V_OR(d, l, r) d->data.real = (l->data.real != 0) | (r->data.real != 0)
#define V_LESS(d, l, r) d->data.real = l->data.real < r->data.real
#define V_MORE(d, l, r) d->data.real = l->data.real > r->data.real
#define V_EQUAL(d, l, r) d->data.real = l->data.real == r->data.real
#define V_MOREEQUAL(d, l, r) d->data.real = l->data.real >= r->data.real
#define V_LESSEQUAL(d, l, r) d->data.real = l->data.real <= r->data.real
#define V_NOTEQUAL(d, l, r) d->data.real = l->data.real != r->data.real

#define V_BOOL(v) (v->data.real != 0)
#define V_TOSTRING(v, s) \
    if (v->type == variant::REAL)\
    {\
        if (fkisint(v->data.real))\
        {\
            s = fkitoa((int64_t)v->data.real);\
        }\
        else\
        {\
            s = fkitoa(v->data.real);\
        }\
    }\
    else if (v->type == variant::STRING)\
    {\
        s = "TODO"; \
    }\
    else\
    {\
        s = fkptoa(v->data.ptr);\
    }

#define V_EQUAL_V(b, l, r) \
    if (l->type != r->type)\
    {\
        b = false;\
    }\
    if (l->type == variant::REAL)\
    {\
        b = (l->data.real == r->data.real);\
    }\
    else if (l->type == variant::STRING)\
    {\
        b = false; /*TODO*/\
    }\
    else\
    {\
        b = (l->data.ptr == r->data.ptr);\
    }

#define V_TYPE_OFF(stackpos) (-1 * (stackpos + 1) * variant_size + variant_type_off)
#define V_DATA_OFF(stackpos) (-1 * (stackpos + 1) * variant_size + variant_data_off)

void cheat_complie();

