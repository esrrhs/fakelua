#pragma once

#include "types.h"
#include "hashmap.h"
#include "variant.h"
#include "array.h"
#include "pool.h"

struct fake;

struct variant_array
{
    array<pool<variant>::node *> va;
};

struct variant_map
{
    vhashmap<pool<variant>::node *> vm;
};

variant * con_array_get(fake * fk, variant_array * va, const variant * k);
variant * con_map_get(fake * fk, variant_map * vm, const variant * k);

class container
{
public:
    container(fake * fk);
	~container();

	void clear();

    variant_array * newarray();
    variant_map * newmap();
    pool<variant>::node * newvariant();
    pool<variant>::node * newglobalvariant();

    variant_map * get_gmap()
    {
        return (variant_map *)&m_gmap;
    }
    
private:
    fake * m_fk;
    poollist<variant_array> m_va_pl;
    
    poollist<variant_map> m_vm_pl;
    
    poollist<variant> m_v_pl;
    
    poollist<variant> m_gv_pl;
    
    vhashmap<pool<variant>::node *> m_gmap;
};

