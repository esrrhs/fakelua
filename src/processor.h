#pragma once

#include "types.h"

struct fuck;
class routine;
class processor
{
public:
    processor(fuck * fk) : m_fk(fk)
    {
        clear();
    }
    ~processor()
    {
    }

    void clear();

    bool run(routine * r);
    
private:
    fuck * m_fk;
};

