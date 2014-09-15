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

#ifdef DEBUG
#define FKLOG(...) fklog("[DEBUG] ", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define FKERR(...) fklog("[ERROR] ", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#else
#define FKLOG(...)
#define FKERR(...)
#endif

void fklog(const char * header, const char * file, const char * func, int pos, const char *fmt, ...);

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

int32_t fkatoi(const String * p);
int64_t fkatol(const String * p);
float fkatof(const String * p);
double fkatod(const String * p);
String fkitoa(int64_t d);
String fkdtoa(double d);
String fkxtoa(int64_t d);

uint32_t fkstrhash(String * p);

String fkget_token_name(int token);

#define MAKEINT64(high, low) ((int64_t)(((int32_t)(low)) | ((int64_t)((int32_t)(high))) << 32))
#define HIINT32(I) ( ( int32_t ) ( ( ( int64_t )( I ) >> 32) & 0xFFFFFFFF ) )
#define LOINT32(l) ((int32_t) (l))

#define MAKEINT32(high, low) ((int32_t)(((int16_t)(low)) | ((int32_t)((int16_t)(high))) << 16))
#define HIINT16(I) ( ( int16_t ) ( ( ( int32_t )( I ) >> 16) & 0xFFFF ) )
#define LOINT16(l) ((int16_t) (l))
