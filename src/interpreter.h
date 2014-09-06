#pragma once

#include "types.h"
#include "variant.h"

struct fuck;
struct fkerrorinfo;
class interpreter
{
public:
    interpreter(fuck * fk, fkerrorinfo * ei) : m_fk(fk), m_ei(ei)
    {
        clear();
    }
    ~interpreter()
    {
    }

    void clear();
    
    bool isend() const;
    
    int run(int cmdnum);
    
private:
    fuck * m_fk;
    fkerrorinfo * m_ei;
    bool m_isend;
};

