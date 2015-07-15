#pragma once

#include "types.h"
#include "hashmap.h"
#include "binary.h"

struct profilefuncele
{
	int callnum;
	uint32_t calltime;
};

class profile
{
public:
	force_inline profile(fake * fk) : m_fk(fk), m_isopen(false), m_shh(fk)
	{
	}
	force_inline ~profile()
	{
		clear();
	}

	force_inline fake * getfake()
	{
		return m_fk;
	}

	force_inline void clear()
	{
		m_isopen = false;
		m_shh.clear();
		m_dumpstr.clear();
		memset(m_codetype, 0, sizeof(m_codetype));
	}

	force_inline void open()
	{
		m_isopen = true;
	}
	
	force_inline void close()
	{
		m_isopen = false;
	}
	
	force_inline bool isopen()
	{
		return m_isopen;
	}
	
	void add_func_sample(const char * func, uint32_t calltime);
	void add_code_sample(int code);

	const char * dump();

private:
	fake * m_fk;
	bool m_isopen;
	typedef fkhashmap<const char *, profilefuncele> stringhashmap;
	stringhashmap m_shh;
	String m_dumpstr;
	int m_codetype[OPCODE_MAX];
};

