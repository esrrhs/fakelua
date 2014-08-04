#pragma once

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

int32_t fkatoi(String * p);
int64_t fkatol(String * p);
float fkatof(String * p);
double fkatod(String * p);
String fkitoa(int64_t d);
String fkdtoa(double d);

uint32_t fkstrhash(String * p);

}

