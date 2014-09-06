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
#include "types.h"
#include "myflexer.h"
#include "fuckscript.h"

struct fkerrorinfo;
struct fuck
{
    fuck()
    {
        clear();
    }

    void clear()
    {
    }
    
    // error
    void seterror(fkerrorinfo * ei, efkerror errno, const char *fmt, ...);
    
    // member
    fkmalloc m_fkmalloc;
    fkfree m_fkfree;
};

template <typename T>
T * fknew(fuck * fk)
{
    T * t = (T *)fk->m_fkmalloc(sizeof(T));
    new (t) T();
    return t;
}

template <typename T, typename P1>
T * fknew(fuck * fk, P1 p1)
{
    T * t = (T *)fk->m_fkmalloc(sizeof(T));
	if (!t)
	{
		return 0;
	}
    new (t) T(p1);
    return t;
}

template <typename T>
void fkdelete(fuck * fk, T * p)
{
	if (p)
	{
		p->~T();
		fk->m_fkfree(p);
	}
}

