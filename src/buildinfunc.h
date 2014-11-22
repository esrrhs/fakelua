#pragma once

#include "types.h"
#include "hashmap.h"

struct interpreter;
struct funcunion;

typedef void (*bifunc)(fake * fk, interpreter * inter);

class buildinfunc
{
public:
	force_inline buildinfunc(fake * fk) : m_fk(fk)
    {
    }
    force_inline ~buildinfunc()
    {
        clear();
    }

    force_inline fake * getfake()
    {
        return m_fk;
    }

    force_inline void clear()
    {
    }

    void openbasefunc();
    
private:
    fake * m_fk;
};

// 参数和返回值都在m_fk->ps里
#define BUILDIN_FUNC_CALL(f, inter) \
	assert((f)->havebif);\
	assert((f)->bif); \
	((f)->bif)(m_fk, (inter))
