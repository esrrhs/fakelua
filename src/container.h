#pragma once

#include "types.h"
#include "hashmap.h"
#include "variant.h"
#include "array.h"
#include "pool.h"

struct fake;

struct variant_array
{
	array<variant *> va;
	bool isconst;
};

#define VARIANT_ARRAY_DELETE(vva) ARRAY_DELETE((vva).va)

struct variant_map
{
	fkhashmap<variant, variant *> vm;
	bool isconst;
};

#define VARIANT_MAP_DELETE(vvm) HASHMAP_DELETE((vvm).vm)

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
	variant * newvariant();

    variant_map * newgmap();
	
	variant_array * newconstarray();
	variant_map * newconstmap();
	variant * newconstvariant();

	size_t get_map_size() const;
	size_t get_array_size() const;
	size_t get_variant_size() const;

	size_t get_cmap_size() const;
	size_t get_carray_size() const;
	size_t get_cvariant_size() const;

private:
	fake * m_fk;
	poollist<variant_array> m_va_pl;
	
	poollist<variant_map> m_vm_pl;
	
	poollist<variant> m_v_pl;
	
	poollist<variant_array> m_cva_pl;
	
	poollist<variant_map> m_cvm_pl;
	
	poollist<variant> m_cv_pl;

    variant_map * m_gm;
};

