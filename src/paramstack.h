#pragma once

#include "types.h"
#include "variant.h"

struct fuck;
class paramstack
{
public:
    paramstack(fuck * fk) : m_fk(fk)
    {
        clear();
    }
    ~paramstack()
    {
    }

    void clear();

    fuck * getfuck();

    void push(const variant & v);
    void pop(variant & v);

    variant operator[](int i);

    size_t size() const;

private:
    fuck * m_fk;
    variant_list m_variant_list;
};

