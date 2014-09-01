#pragma once

#include "types.h"

struct fuck;
struct func_binary
{
    func_binary(fuck * fk) : m_fk(fk)
    {
        m_buff = 0;
        m_size = 0;
    }

    void push(int code)
    {
        if (full())
        {
            grow();
        }
        m_buff[m_curpos] = code;
        m_curpos++;
    }

    bool full() const
    {
        return m_curpos >= m_size;
    }
    
    void grow();

    fuck * m_fk;
    int * m_buff;
    size_t m_curpos;
    size_t m_size;
};

typedef std::map<String, int> func_binary_map;
typedef std::vector<func_binary> func_binary_list;

class binary
{
public:
    binary(fuck * fk) : m_fk(fk)
    {
        clear();
    }
    ~binary()
    {
    }

    void clear();

    bool is_have_func(const String & name);
    bool add_func(const String & name, func_binary bin);

private:
    fuck * m_fk;
    func_binary_map m_func_index_map;
    func_binary_list m_func_list;
};

