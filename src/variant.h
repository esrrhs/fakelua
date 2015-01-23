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
		uint64_t buf;	// 只是用作64位传递 
	};

	Type type;
	MemData data;
};

#define variant_size sizeof(variant)
#define variant_type_off ((size_t)(size_t*)&((variant*)0)->type)
#define variant_data_off ((size_t)(size_t*)&((variant*)0)->data)

#ifdef FK64
#define V_SAFE_PTR_CLEAN(v)
#else
#define V_SAFE_PTR_CLEAN(v) (v)->data.buf = 0
#endif

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
		ss = fkstringeletoa((v)->data.str); \
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

#define V_SET_NIL(v) \
	(v)->type = variant::NIL; \
	(v)->data.buf = 0;

#define V_SET_POINTER(v, p) \
	(v)->type = variant::POINTER; \
	V_SAFE_PTR_CLEAN(v); \
	(v)->data.ptr = p;

#define V_SET_REAL(v, r) \
	(v)->type = variant::REAL; \
	(v)->data.real = r;

#define V_SET_STRING(v, s) \
	(v)->type = variant::STRING; \
	V_SAFE_PTR_CLEAN(v); \
	(v)->data.str = fk->sh.allocstring(s);

#define V_SET_UUID(v, id) \
	(v)->type = variant::UUID; \
	(v)->data.uuid = id;

#define V_SET_ARRAY(v, a) \
	(v)->type = variant::ARRAY; \
	V_SAFE_PTR_CLEAN(v); \
	(v)->data.va = a;
	
#define V_SET_MAP(v, m) \
	(v)->type = variant::MAP; \
	V_SAFE_PTR_CLEAN(v); \
	(v)->data.vm = m;
	
#define V_GET_POINTER(v, p) \
	if (!((v)->type == variant::POINTER || (v)->type == variant::NIL)) \
	{ \
	    err = true; \
	    String str; \
	    V_TOSTRING(v, str); \
	    seterror(fk, efk_run_data_error, "variant get pointer fail, the variant is %d %s", (v)->type, str.c_str()); \
	    p = 0; \
	    USE(err); \
	} \
	else \
	{ \
	    p = (v)->data.ptr; \
	}
    
#define V_GET_REAL(v, r) \
	if (!((v)->type == variant::REAL || (v)->type == variant::NIL)) \
	{ \
	    err = true; \
	    String str; \
	    V_TOSTRING(v, str); \
	    seterror(fk, efk_run_data_error, "variant get real fail, the variant is %d %s", (v)->type, str.c_str()); \
	    r = 0; \
	    USE(err); \
	} \
	else \
	{ \
	    r = (v)->data.real; \
	}
    
#define V_GET_STRING(v, ss) \
	if (!((v)->type == variant::STRING || (v)->type == variant::NIL)) \
	{ \
	    err = true; \
	    String str; \
	    V_TOSTRING(v, str); \
	    seterror(fk, efk_run_data_error, "variant get string fail, the variant is %d %s", (v)->type, str.c_str()); \
	    ss = 0; \
	    USE(err); \
	} \
	else \
	{ \
	    ss = (v)->data.str ? (v)->data.str->s : 0; \
	}

#define V_GET_UUID(v, id) \
	if (!((v)->type == variant::UUID || (v)->type == variant::NIL)) \
	{ \
	    err = true; \
	    String str; \
	    V_TOSTRING(v, str); \
	    seterror(fk, efk_run_data_error, "variant get uuid fail, the variant is %d %s", (v)->type, str.c_str()); \
	    id = 0; \
	    USE(err); \
	} \
	else \
	{ \
	    id = (v)->data.uuid; \
	}

#define V_ASSERT_CAN_CAL(v) \
    if (!((v)->type == variant::REAL || (v)->type == variant::NIL)) \
	{ \
	    err = true; \
	    String str; \
	    V_TOSTRING(v, str); \
	    seterror(fk, efk_run_cal_error, "variant can not calculate, the variant is %d %s", (v)->type, str.c_str()); \
	    USE(err); \
	}
#define V_ASSERT_CAN_DIVIDE(v) \
    if ((int64_t)(v)->data.real == 0) \
	{ \
	    err = true; \
	    String str; \
	    V_TOSTRING(v, str); \
	    seterror(fk, efk_run_cal_error, "variant can not be divide, the variant is %d %s", (v)->type, str.c_str()); \
	    USE(err); \
	}
#define V_ASSERT_CAN_CAL_BIN(l, r) V_ASSERT_CAN_CAL(l);V_ASSERT_CAN_CAL(r)
    
#define V_PLUS(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real + (r)->data.real;(d)->type = variant::REAL
#define V_MINUS(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real - (r)->data.real;(d)->type = variant::REAL
#define V_MULTIPLY(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real * (r)->data.real;(d)->type = variant::REAL
#define V_DIVIDE(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);V_ASSERT_CAN_DIVIDE(r);(d)->data.real = (l)->data.real / (r)->data.real;(d)->type = variant::REAL
#define V_DIVIDE_MOD(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);V_ASSERT_CAN_DIVIDE(r);(d)->data.real = ((int64_t)(r)->data.real) ? (int64_t)(l)->data.real % ((int64_t)(r)->data.real) : 0;(d)->type = variant::REAL
#define V_AND(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = ((l)->data.real != 0) & ((r)->data.real != 0);(d)->type = variant::REAL
#define V_OR(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = ((l)->data.real != 0) | ((r)->data.real != 0);(d)->type = variant::REAL
#define V_LESS(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real < (r)->data.real;(d)->type = variant::REAL
#define V_MORE(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real > (r)->data.real;(d)->type = variant::REAL
#define V_EQUAL(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real == (r)->data.real;(d)->type = variant::REAL
#define V_MOREEQUAL(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real >= (r)->data.real;(d)->type = variant::REAL
#define V_LESSEQUAL(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real <= (r)->data.real;(d)->type = variant::REAL
#define V_NOTEQUAL(d, l, r) V_ASSERT_CAN_CAL_BIN(l, r);(d)->data.real = (l)->data.real != (r)->data.real;(d)->type = variant::REAL

#define V_BOOL(v) ((v)->data.real != 0)
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

