#pragma once

#include "types.h"
#include "hashmap.h"
#include "array.h"
#include "pool.h"

struct interpreter;
struct funcunion;
struct buffer;
struct selector;

class buildinfuncnet
{
public:
	buildinfuncnet(fake * fk);
	~buildinfuncnet();

	force_inline fake * getfake()
	{
		return m_fk;
	}

	void clear();

	void opennetfunc();

	buffer * newbuffer(int size);
	selector * newselector();
	
	size_t get_buffer_size() const;
	size_t get_selector_size() const;
private:
	fake * m_fk;
	poollist<buffer> m_buffer;
	poollist<selector> m_selector;
};

