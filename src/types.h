#pragma once

#include <string>
#include <list>
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

#define FKLOG(...) fklog(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
void fklog(const char * file, const char * func, int pos, const char *fmt, ...);

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

int32_t fkatoi(String * p);
int64_t fkatol(String * p);
float fkatof(String * p);
double fkatod(String * p);
String fkitoa(int64_t d);
String fkdtoa(double d);

uint32_t fkstrhash(String * p);

String fkget_token_name(int token);

