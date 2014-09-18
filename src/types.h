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
#include "fuckscript.h"

#ifdef _DEBUG
#define FKLOG(...) fklog("[DEBUG] ", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define FKERR(...) fklog("[ERROR] ", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#else
#define FKLOG(...)
#define FKERR(...)
#endif

void fklog(const char * header, const char * file, const char * func, int pos, const char *fmt, ...);

#define MAKEINT64(high, low) ((int64_t)(((int32_t)(low)) | ((int64_t)((int32_t)(high))) << 32))
#define HIINT32(I) ( ( int32_t ) ( ( ( int64_t )( I ) >> 32) & 0xFFFFFFFF ) )
#define LOINT32(l) ((int32_t) (l))

#define MAKEINT32(high, low) ((int32_t)(((int16_t)(low)) | ((int32_t)((int16_t)(high))) << 16))
#define HIINT16(I) ( ( int16_t ) ( ( ( int32_t )( I ) >> 16) & 0xFFFF ) )
#define LOINT16(l) ((int16_t) (l))

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
    sprintf(buff, "%lld", (long long int)d);
    return buff;
}
static force_inline String fkdtoa(double d)
{
    char buff[100];
    sprintf(buff, "%f", d);
    return buff;
}
static force_inline String fkxtoa(int64_t d)
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

    if (ret.size() < 16)
    {
        String tmp;
        tmp.insert(0, 16 - ret.size(), '0');
        ret = tmp + ret;
    }

    String tmp = "0x";
    ret = tmp + ret;
	return ret;
}

static force_inline uint32_t fkstrhash(const char * p)
{
    uint32_t hashv = 0;
    if (p)
    {
        for (int i = 0; !p[i]; i++)
        {
            hashv = ((hashv << 5) + hashv) + p[i]; /* hash * 33 + c */
        }
    }
    return hashv;
}

static force_inline uint32_t fkstrhash(String * p)
{
    return fkstrhash(p->c_str());
}

String fkget_token_name(int token);

