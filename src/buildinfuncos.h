#pragma once

#include "types.h"
#include "hashmap.h"

struct interpreter;
struct funcunion;

class buildinfuncos
{
public:
	force_inline buildinfuncos(fake * fk) : m_fk(fk)
    {
    }
    force_inline ~buildinfuncos()
    {
        clear();
    }

    force_inline fake * getfake()
    {
        return m_fk;
    }

    force_inline void clear()
	{
    }

    void openosfunc();
    
	void setargv(int argc, const char *argv[]);
	int get_argc() const;
	const char * get_argv(int pos) const;

private:
	fake * m_fk;
	int m_argc;
	const char ** m_argv;
};

