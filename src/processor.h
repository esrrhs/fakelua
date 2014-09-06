#pragma once

#include "types.h"


struct fuck;
class routine;

typedef std::list<routine *> routine_list;

class processor
{
public:
    processor(fuck * fk) : m_fk(fk), m_routine_list(0)
    {
        clear();
    }
    ~processor()
    {
    }

    void clear();

    void add(routine * r);

    int run(int cmdnum);

    bool isend() const;
    
private:
    bool empty() const;
    bool full() const;
    void grow();
    void checkdelete();
private:
    fuck * m_fk;
    routine ** m_routine_list;
    size_t m_routine_num;
    size_t m_routine_max_size;
    size_t m_invalid_routine_num;
};

