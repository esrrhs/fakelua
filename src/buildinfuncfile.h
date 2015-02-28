#pragma once

#include "types.h"
#include "hashmap.h"

struct interpreter;
struct funcunion;

class buildinfuncfile
{
public:
	force_inline buildinfuncfile(fake * fk) : m_fk(fk)
    {
    }
    force_inline ~buildinfuncfile()
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

    void openfilefunc();
    
private:
    fake * m_fk;
};

