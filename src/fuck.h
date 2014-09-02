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

struct fuck
{
    fuck()
    {
        clear();
    }

    void clear()
    {
        m_efkerror = efk_ok;
        m_errorstr.clear();
    }
    
    // error
    efkerror error();
    const char * errorstr();
    void seterror(efkerror errno, const char *fmt, ...);
    
    // member
    fkmalloc m_fkmalloc;
    fkfree m_fkfree;
    efkerror m_efkerror;
    String m_errorstr;
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
    new (t) T(p1);
    return t;
}

