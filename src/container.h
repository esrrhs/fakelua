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

private:
    fake * m_fk;
    pool<variant_array> m_va_pool;
    array<pool<variant_array>::node *> m_va_list;
    
    pool<variant_map> m_vm_pool;
    array<pool<variant_map>::node *> m_vm_list;
    
    pool<variant> m_vpool;
    array<pool<variant>::node *> m_vlist;
};

