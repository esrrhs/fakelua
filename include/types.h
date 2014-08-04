#pragma once

#include <string>
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

namespace fscript
{

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

}

