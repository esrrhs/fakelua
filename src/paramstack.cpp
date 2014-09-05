#include "paramstack.h"
#include "fuck.h"

void paramstack::clear()
{
}

fuck * paramstack::getfuck()
{
    return m_fk;
}

void paramstack::push(const variant & v)
{
    m_variant_list.push_back(v);
}

void paramstack::pop(variant & v)
{
    if (!m_variant_list.empty())
    {
        v = m_variant_list.back();
        m_variant_list.pop_back();
    }
}


