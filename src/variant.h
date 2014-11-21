#pragma once

#include "types.h"

struct stringele;
struct variant_array;
struct variant_map;
struct variant 
{
	enum Type 
	{
		NIL,
		REAL,       // 参与计算的数值
		STRING,     // 字符串
		POINTER,    // 指针
		UUID,       // int64的uuid，不参与计算，为了效率
		ARRAY,      // 数组
		MAP,        // 集合
	};	
    
	union MemData
	{
		double real;
		stringele * str;
		void * ptr;
		uint64_t uuid;
		variant_array * va;
		variant_map * vm;
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

#define V_SET_UUID(v, id) \
	(v)->type = variant::UUID; \
	(v)->data.uuid = id;

#define V_SET_ARRAY(v, a) \
	(v)->type = variant::ARRAY; \
	(v)->data.va = a;
	
#define V_SET_MAP(v, m) \
	(v)->type = variant::MAP; \
	(v)->data.vm = m;
	
#define V_GET_POINTER(v, p) \
	assert((v)->type == variant::POINTER || (v)->type == variant::NIL); \
	p = (v)->data.ptr;
    
#define V_GET_REAL(v, r) \
	assert((v)->type == variant::REAL || (v)->type == variant::NIL); \
	r = (v)->data.real;
    
#define V_GET_STRING(v, ss) \
	assert((v)->type == variant::STRING || (v)->type == variant::NIL); \
	ss = (v)->data.str ? (v)->data.str->s : 0;

#define V_GET_UUID(v, id) \
	assert((v)->type == variant::UUID || (v)->type == variant::NIL); \
	id = (v)->data.uuid;

#define V_ASSERT_CAN_CAL(v) assert((v)->type == variant::NIL || (v)->type == variant::REAL)
#define V_ASSERT_CAN_CAL_BIN(l, r) V_ASSERT_CAN_CAL(l);V_ASSERT_CAN_CAL(r)
    
#define V_PLUS(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real + (r)->data.real;(d)->type = variant::REAL
#define V_MINUS(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real - (r)->data.real;(d)->type = variant::REAL
#define V_MULTIPLY(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real * (r)->data.real;(d)->type = variant::REAL
#define V_DIVIDE(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real / (r)->data.real;(d)->type = variant::REAL
#define V_DIVIDE_MOD(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (int64_t)(l)->data.real % ((int64_t)(r)->data.real);(d)->type = variant::REAL
#define V_AND(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = ((l)->data.real != 0) & ((r)->data.real != 0);(d)->type = variant::REAL
#define V_OR(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = ((l)->data.real != 0) | ((r)->data.real != 0);(d)->type = variant::REAL
#define V_LESS(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real < (r)->data.real;(d)->type = variant::REAL
#define V_MORE(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real > (r)->data.real;(d)->type = variant::REAL
#define V_EQUAL(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real == (r)->data.real;(d)->type = variant::REAL
#define V_MOREEQUAL(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real >= (r)->data.real;(d)->type = variant::REAL
#define V_LESSEQUAL(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real <= (r)->data.real;(d)->type = variant::REAL
#define V_NOTEQUAL(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real != (r)->data.real;(d)->type = variant::REAL

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
	else if ((v)->type == variant::UUID)\
    {\
        ss = fkuitoa((v)->data.uuid); \
    }\
    else if ((v)->type == variant::POINTER)\
    {\
		ss = fkptoa((v)->data.ptr); \
    }\
    else if ((v)->type == variant::ARRAY)\
    {\
		ss = fkarraytoa((v)->data.va); \
    }\
    else if ((v)->type == variant::MAP)\
    {\
		ss = fkmaptoa((v)->data.vm); \
    }\
    else if ((v)->type == variant::NIL)\
    {\
		ss = "nil"; \
    }\
    else\
    {\
        ss = "ERROR"; \
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

