#pragma once

#include <string>
#include <list>
#include <vector>
#include <map>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <typeinfo>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include "fake-inc.h"

#ifdef _DEBUG
#define FKLOG(...) fklog("[DEBUG] ", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define FKERR(...) fklog("[ERROR] ", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define ISLOG 1
#else
#define FKLOG(...)
#define FKERR(...)
#define ISLOG 0
#endif

void fklog(const char * header, const char * file, const char * func, int pos, const char *fmt, ...);

#define MAKEINT64(high, low) ((int64_t)(((int32_t)(low)) | ((int64_t)((int32_t)(high))) << 32))
#define HIINT32(I) ( ( int32_t ) ( ( ( int64_t )( I ) >> 32) & 0xFFFFFFFF ) )
#define LOINT32(l) ((int32_t) (l))

#define MAKEINT32(high, low) ((int32_t)(((int16_t)(low)) | ((int32_t)((int16_t)(high))) << 16))
#define HIINT16(I) ( ( int16_t ) ( ( ( int32_t )( I ) >> 16) & 0xFFFF ) )
#define LOINT16(l) ((int16_t) (l))

#define FKMIN(a, b) ((a) < (b) ? (a) : (b))
#define FKMAX(a, b) ((a) > (b) ? (a) : (b))

#define force_inline __inline__ __attribute__((always_inline))

#define tsnprintf   snprintf

typedef std::string String;

template<class T>
struct GetSimpleType {
	T type;
};

union MarshallFloat 
{
    uint64_t i; ///< int
	float f; ///< float
};

union MarshallPoiner
{
    uint64_t i; ///< int
	void * p; ///< p
};

static force_inline bool fkisint(double real)
{
	return (real - (int64_t)real == 0);
}

static force_inline int32_t fkatoi(const String * p)
{
	if (p)
	{
		return atoi(p->c_str());
	}
	return 0;
}

static force_inline int64_t fkatol(const String * p)
{
	if (p)
	{
		return atoll(p->c_str());
	}
	return 0;
}
static force_inline float fkatof(const String * p)
{
	if (p)
	{
		return (float)atof(p->c_str());
	}
	return 0;
}
static force_inline double fkatod(const String * p)
{
	if (p)
	{
		return atof(p->c_str());
	}
	return 0;
}
static force_inline String fkitoa(int64_t d)
{
	char buff[100];
	tsnprintf(buff, sizeof(buff) - 1, "%lld", (long long int)d);
	return buff;
}
static force_inline String fkuitoa(uint64_t d)
{
	char buff[100];
	tsnprintf(buff, sizeof(buff) - 1, "%llu", (unsigned long long int)d);
	return buff;
}
static force_inline String fkdtoa(double d)
{
	char buff[100];
	tsnprintf(buff, sizeof(buff) - 1, "%f", d);
	return buff;
}
static force_inline String fkptoa(void * p)
{
	char buff[100];
	tsnprintf(buff, sizeof(buff) - 1, "%p", p);
	return buff;
}
static force_inline String fkxtoa(int64_t d, int wid = 16)
{
	String ret;
	
	char tmpbuf[32]={0};
	uint32_t idx = 31;

	// special case '0'

	if (!d)
	{
		tmpbuf[30] = '0';
		ret = &tmpbuf[30];
	}
	else
	{
		// add numbers
		const uint8_t chars[]="0123456789ABCDEF";
		while(d && idx)
		{
			--idx;
			tmpbuf[idx] = chars[(d % 16)];
			d /= 16;
		}

		ret = &tmpbuf[idx];
	}

	if ((int)ret.size() < wid)
	{
		String tmp;
		tmp.insert(0, wid - ret.size(), '0');
		ret = tmp + ret;
	}

	String tmp = "0x";
	ret = tmp + ret;
	return ret;
}

struct stringele;
String fkstringeletoa(stringele * ele);

static force_inline uint64_t fkstrhash(const char * p)
{
    uint64_t hashv = 0;
	for (int i = 0; p[i]; i++)
	{
		hashv = ((hashv << 5) + hashv) + p[i]; /* hash * 33 + c */
	}
	return hashv;
}

String fkget_token_name(int token);

enum e_mem_type
{
	emt_tmp,
	emt_array,
	emt_func_binary,
	emt_buffer,
	emt_hashmap,
	emt_hashset,
	emt_hashlist,
	emt_flexer,
	emt_pool,
	emt_string,
	emt_native,
	emt_stringheap,
	emt_hashstring,
	emt_stringele,
	emt_max,
};
const char * get_mem_type_name(e_mem_type type);

void * safe_fkmalloc(fake * fk, size_t size, e_mem_type type);
void safe_fkfree(fake * fk, const void * p);

void seterror(fake * fk, efkerror err, const char * file, int lineno, const char * func, const char *fmt, ...);
void setwarn(fake * fk, const char *fmt, ...);

struct variant;
String vartostring(const variant * v);

const char * vartypetostring(int type);

template <typename T>
void USE(T t) {}

struct paramstack;
paramstack * getps(fake * fk);

char * stringdump(fake * fk, const char * src, size_t sz, e_mem_type type = emt_string);

uint32_t fkgetmstick();

struct pointerele;
struct variant_array;
struct variant_map;
String fkpointertoa(pointerele * pe);
String fkarraytoa(variant_array * va);
String fkmaptoa(variant_map * vm);

#ifdef __x86_64
#define FK64
#endif

struct routine;

static force_inline String fkgen_package_name(const String & p, const String & n)
{
	if (p.empty())
	{
		return n;
	}
	else
	{
		return p + "." + n;
	}
}

#define MAP_FUNC_NAME "map"
#define GMAP_FUNC_NAME "_G"

#define LIKE(x) __builtin_expect((x),1)
#define UNLIKE(x) __builtin_expect((x),0)

struct buffer;
bool save_variant(fake * fk, const variant * v, buffer * b);
bool load_variant(fake * fk, variant * v, buffer * b);
bool save_string(fake * fk, const char * str, buffer * b);
bool load_string(fake * fk, String & str, buffer * b);

enum debug_command
{
	debug_next,
	debug_step,
	debug_next_instruction,
	debug_step_instruction,
	debug_continue,
	debug_breakpoint,
	debug_enable,
	debug_disable,
	debug_delete,
	debug_info,
	debug_finish,
	debug_list,
	debug_print,
	debug_set,
	debug_watch,
	debug_backtrace,
	debug_frame,
	debug_disa,
	debug_routine,
};

String fix_string_wrap(const String & str, int len);

struct stringele
{
	// string
	uint32_t sz;
	uint32_t sysref;
	char * s;
};

const fakeconfig & get_fakeconfig(fake * fk);

#define SAVE_NORMAL(x) \
	if (!b->write((const char *)&(x), sizeof(x))) \
	{ \
		return false; \
	}
	
#define SAVE_STRING(x) \
	if (!save_string(fk, (x), b)) \
	{ \
		return false; \
	}
	
#define SAVE_ARRAY(x, len) \
	SAVE_NORMAL(len); \
	if (!b->write((const char *)(x), (len) * sizeof((x)[0]))) \
	{ \
		return false; \
	}
	
#define SAVE_VARRAY(x, len) \
	SAVE_NORMAL(len); \
	for (int i = 0 ; i < (len); i++) \
	{ \
		if (!save_variant(fk, &((x)[i]), b)) \
		{ \
			return false; \
		} \
	}
	
#define LOAD_NORMAL(x) \
	if (!b->read((char *)&(x), sizeof(x))) \
	{ \
		return false; \
	}
		
#define LOAD_STRING(x) \
	String x##name; \
	if (!load_string(fk, x##name, b)) \
	{ \
		return false; \
	} \
	(x) = stringdump(fk, x##name.c_str(), x##name.size());
		
#define LOAD_ARRAY(x, len, type) \
	LOAD_NORMAL(len); \
	(x) = (type *)safe_fkmalloc(fk, (len) * sizeof((x)[0]), emt_func_binary); \
	if (!b->read((char *)(x), (len) * sizeof(type))) \
	{ \
		return false; \
	}
		
#define LOAD_VARRAY(x, len) \
	LOAD_NORMAL(len); \
	(x) = (variant *)safe_fkmalloc(fk, ((len) * sizeof(variant)), emt_func_binary); \
	for (int i = 0 ; i < (len); i++) \
	{ \
		if (!load_variant(fk, &((x)[i]), b)) \
		{ \
			return false; \
		} \
	}
	
void * fk_mmap_alloc(size_t size);
void fk_mmap_set_exec(void * buff, size_t size);

enum e_gc_type
{
    egt_string,
    egt_max
};

const char * get_gc_type_name(int type);
