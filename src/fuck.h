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
    fuck() : m_myflexer(this)
    {
        clear();
    }

    void clear()
    {
        m_myflexer.clear();
        m_efkerror = efk_ok;
    }
    
    // error
    efkerror error();

    // member
    fkmalloc m_fkmalloc;
    fkfree m_fkfree;
    myflexer m_myflexer;
    efkerror m_efkerror;
};


