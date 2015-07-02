#pragma once

#include "types.h"
#include "hashmap.h"

struct interpreter;
struct funcunion;

class buildinfuncmath
{
public:
	force_inline buildinfuncmath(fake * fk) : m_fk(fk), m_seed(0)
    {
    }
	force_inline ~buildinfuncmath()
    {
        clear();
    }

    force_inline fake * getfake()
    {
        return m_fk;
    }

    force_inline void clear()
    {
		m_seed = 0;
    }

    void openmathfunc();

	void setseed(int seed);
	int getseed() const;

private:
    fake * m_fk;
	int m_seed;
};

