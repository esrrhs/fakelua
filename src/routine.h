#pragma once

#include "types.h"

struct fuck;
class binary;
class routine
{
public:
    routine(fuck * fk, binary * bin, const char * func) : m_fk(fk), m_binary(bin), m_func(func)
    {
        clear();
    }
    ~routine()
    {
    }

    void clear();

private:
    fuck * m_fk;
    binary * m_binary;
    const char * m_func;
};

