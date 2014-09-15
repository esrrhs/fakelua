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
    void seterror(fkerrorinfo * ei, efkerror err, const char *fmt, ...);
    
    // member
    fkmalloc m_fkmalloc;
    fkfree m_fkfree;

    // processor每帧调用cmd条数
    int m_per_frame_cmd_num;
    // processor清除无效协程判断的比例
    int m_delete_routine_scale;
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

