#pragma once

#include "types.h"

struct stringele;
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
		stringele * str;
		void * ptr;
		int64_t buf;	// 只是用作64位传递 
	};

	Type type;
	MemData data;
};

#define variant_size sizeof(variant)
#define variant_type_off ((size_t)(size_t*)&((variant*)0)->type)
#define variant_data_off ((size_t)(size_t*)&((variant*)0)->data)

#define V_SET_NIL(v) \
	(v)->type = variant::NIL; \
	(v)->data.buf = 0;

#define V_SET_POINTER(v, p) \
	(v)->type = variant::POINTER; \
	(v)->data.ptr = p;

#define V_SET_REAL(v, r) \
	(v)->type = variant::REAL; \
	(v)->data.real = r;

#define V_SET_STRING(v, s) \
	(v)->type = variant::STRING; \
	(v)->data.str = fk->sh.allocstring(s);

#define V_GET_POINTER(v, p) \
	assert((v)->type == variant::POINTER || (v)->type == variant::NIL); \
	p = (v)->data.ptr;
    
#define V_GET_REAL(v, r) \
	assert((v)->type == variant::REAL || (v)->type == variant::NIL); \
	r = (v)->data.real;
    
#define V_GET_STRING(v, ss) \
	assert((v)->type == variant::STRING || (v)->type == variant::NIL); \
	ss = (v)->data.str ? (v)->data.str->s : 0;

#define V_PLUS(d, l, r) (d)->data.real = (l)->data.real + (r)->data.real;(d)->type = variant::REAL
#define V_MINUS(d, l, r) (d)->data.real = (l)->data.real - (r)->data.real;(d)->type = variant::REAL
#define V_MULTIPLY(d, l, r) (d)->data.real = (l)->data.real * (r)->data.real;(d)->type = variant::REAL
#define V_DIVIDE(d, l, r) (d)->data.real = (l)->data.real / (r)->data.real;(d)->type = variant::REAL
#define V_DIVIDE_MOD(d, l, r) (d)->data.real = (int64_t)(l)->data.real % ((int64_t)(r)->data.real);(d)->type = variant::REAL
#define V_AND(d, l, r) (d)->data.real = ((l)->data.real != 0) & ((r)->data.real != 0);(d)->type = variant::REAL
#define V_OR(d, l, r) (d)->data.real = ((l)->data.real != 0) | ((r)->data.real != 0);(d)->type = variant::REAL
#define V_LESS(d, l, r) (d)->data.real = (l)->data.real < (r)->data.real;(d)->type = variant::REAL
#define V_MORE(d, l, r) (d)->data.real = (l)->data.real > (r)->data.real;(d)->type = variant::REAL
#define V_EQUAL(d, l, r) (d)->data.real = (l)->data.real == (r)->data.real;(d)->type = variant::REAL
#define V_MOREEQUAL(d, l, r) (d)->data.real = (l)->data.real >= (r)->data.real;(d)->type = variant::REAL
#define V_LESSEQUAL(d, l, r) (d)->data.real = (l)->data.real <= (r)->data.real;(d)->type = variant::REAL
#define V_NOTEQUAL(d, l, r) (d)->data.real = (l)->data.real != (r)->data.real;(d)->type = variant::REAL

#define V_BOOL(v) ((v)->data.real != 0)
#define V_TOSTRING(v, ss) \
	if ((v)->type == variant::REAL)\
    {\
		if (fkisint((v)->data.real))\
        {\
			ss = fkitoa((int64_t)(v)->data.real); \
        }\
        else\
        {\
			ss = fkitoa((v)->data.real); \
        }\
    }\
	else if ((v)->type == variant::STRING)\
    {\
		ss = (v)->data.str ? (v)->data.str->s : ""; \
    }\
    else\
    {\
		ss = fkptoa((v)->data.ptr); \
    }

#define V_EQUAL_V(b, l, r) \
    if ((l)->type != (r)->type)\
    {\
        b = false;\
    }\
    else\
    {\
		b = ((l)->data.buf == (r)->data.buf); \
    }

#define V_TYPE_OFF(stackpos) (-1 * (stackpos + 1) * variant_size + variant_type_off)
#define V_DATA_OFF(stackpos) (-1 * (stackpos + 1) * variant_size + variant_data_off)

