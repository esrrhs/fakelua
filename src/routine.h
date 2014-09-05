#pragma once

#include "types.h"
#include "variant.h"

struct fuck;
class func_binary;
class routine
{
public:
    routine(fuck * fk, const func_binary * fb) : m_fk(fk), m_fb(fb), m_ret(fk)
    {
        clear();
    }
    ~routine()
    {
    }

    void clear();

    const variant & getret() const;

    bool isend() const;
    
    int run(int cmdnum);

private:
    void runcmd();
    
private:
    fuck * m_fk;
    const func_binary * m_fb;
    variant m_ret;
    int m_cmdpos;
    bool m_isend;
};

