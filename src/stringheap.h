#pragma once

#include "types.h"

struct fuck;

class stringheap
{
public:
    force_inline stringheap(fuck * fk) : m_fk(fk)
    {
    }
    force_inline ~stringheap()
    {
    }

    force_inline void clear()
    {
    }

private:
    fuck * m_fk;    
};

