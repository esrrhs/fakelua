#pragma once

#include "types.h"

struct fuck;

struct stringele
{
    stringele() : update(0), hash(0) {}
    stringele(const char * _str) : update(0), hash(0), str(_str) {}
    // 更新计数
    uint32_t update;
    // hash值
    uint32_t hash;
    String str;
};

typedef std::vector<stringele> stringelelist;

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
        m_stringelelist.clear();
    }

    force_inline stringele * allocstring(const char * str)
    {
        // TODO 做hash和gc
        for (int i = 0; i < (int)m_stringelelist.size(); i++)
        {
            if (m_stringelelist[i].str == str)
            {
                return &m_stringelelist[i];
            }
        }
        m_stringelelist.push_back(stringele(str));
        return &m_stringelelist[m_stringelelist.size() - 1];
    }
    
private:
    fuck * m_fk;    
    stringelelist m_stringelelist;
};

