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
#include "fakescript.h"

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

#ifdef WIN32
#define force_inline __forceinline
#else
#define force_inline __inline__ __attribute__((always_inline))
#endif

typedef std::string String;

template<class T>
struct GetSimpleType {
	T type;
};

union MarshallFloat 
{
	uint32_t i; ///< int
	float f; ///< float
};

union MarshallPoiner
{
	uint32_t i; ///< int
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
#ifdef WIN32
		return _atoi64(p->c_str());
#else
		return atoll(p->c_str());
#endif
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
	sprintf(buff, "%lld", (long long int)d);
	return buff;
}
static force_inline String fkuitoa(uint64_t d)
{
	char buff[100];
	sprintf(buff, "%llu", (unsigned long long int)d);
	return buff;
}
static force_inline String fkdtoa(double d)
{
	char buff[100];
	sprintf(buff, "%f", d);
	return buff;
}
static force_inline String fkptoa(void * p)
{
	char buff[100];
	sprintf(buff, "%p", p);
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

static force_inline uint32_t fkstrhash(const char * p)
{
	uint32_t hashv = 0;
	for (int i = 0; p[i]; i++)
	{
		hashv = ((hashv << 5) + hashv) + p[i]; /* hash * 33 + c */
	}
	return hashv;
}

String fkget_token_name(int token);

void * safe_fkmalloc(fake * fk, size_t size);
void safe_fkfree(fake * fk, const void * p);

void seterror(fake * fk, efkerror err, const char * file, int lineno, const char * func, const char *fmt, ...);

struct variant;
String vartostring(const variant * v);

const char * vartypetostring(int type);

template <typename T>
void USE(T t) {}

struct paramstack;
paramstack * getps(fake * fk);

char * stringdump(fake * fk, const char * src, size_t sz);

uint32_t fkgetmstick();

#if defined(WIN32)
#define tsnprintf   _snprintf
#else
#define tsnprintf   snprintf
#endif

struct variant_array;
struct variant_map;
String fkarraytoa(variant_array * va);
String fkmaptoa(variant_map * vm);

#if defined(WIN32)
#ifdef _M_X64 
#define FK64
#endif
#else
#ifdef __x86_64
#define FK64
#endif
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

#ifdef WIN32
#define LIKE(x) (x)
#define UNLIKE(x) (x)
#else
#define LIKE(x) __builtin_expect((x),1)
#define UNLIKE(x) __builtin_expect((x),0)
#endif

// socket
#ifdef WIN32
	#define GET_NET_ERROR WSAGetLastError()
	#define NET_BLOCK_ERROR WSAEWOULDBLOCK
	#define NET_BLOCK_ERROR_EX WSAEWOULDBLOCK
#else
	#define GET_NET_ERROR errno
	#define NET_BLOCK_ERROR EWOULDBLOCK
	#define NET_BLOCK_ERROR_EX EAGAIN
#endif

#if defined(WIN32)
typedef int socklen_t;
#else
#endif

struct buffer;
bool save_variant(fake * fk, const variant * v, buffer * b);
bool load_variant(fake * fk, variant * v, buffer * b);
bool save_string(fake * fk, const char * str, buffer * b);
bool load_string(fake * fk, String & str, buffer * b);

