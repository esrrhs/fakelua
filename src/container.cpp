#include "container.h"
#include "fake.h"

container::container(fake * fk) : m_fk(fk)
{
	POOLLIST_INI(m_va_pl, fk);
	POOLLIST_INI(m_vm_pl, fk);
	POOLLIST_INI(m_v_pl, fk);
	POOLLIST_INI(m_cva_pl, fk);
	POOLLIST_INI(m_cvm_pl, fk);
	POOLLIST_INI(m_cv_pl, fk);
}

container::~container()
{
	clear();
	
	{
		POOLLIST_DELETE(m_va_pl, variant_array, VARIANT_ARRAY_DELETE(*n));
		POOLLIST_DELETE(m_vm_pl, variant_map, VARIANT_MAP_DELETE(*n));
		POOLLIST_DELETE(m_v_pl, variant, USE(n));
	}
	
	{
		POOLLIST_CLEAR(m_cva_pl, variant_array, ARRAY_DEEP_CLEAR(n->va, variant_array *));
		POOLLIST_CLEAR(m_cvm_pl, variant_map, HASHMAP_CLEAR(n->vm));
		POOLLIST_CLEAR(m_cv_pl, variant, USE(n));
		
		POOLLIST_DELETE(m_cva_pl, variant_array, VARIANT_ARRAY_DELETE(*n));
		POOLLIST_DELETE(m_cvm_pl, variant_map, VARIANT_MAP_DELETE(*n));
		POOLLIST_DELETE(m_cv_pl, variant, USE(n));
	}
}

void container::clear()
{
	POOLLIST_CLEAR(m_va_pl, variant_array, ARRAY_DEEP_CLEAR(n->va, variant_array *));
	POOLLIST_CLEAR(m_vm_pl, variant_map, HASHMAP_CLEAR(n->vm));
	POOLLIST_CLEAR(m_v_pl, variant, USE(n));
}

variant_array * container::newarray()
{
	variant_array * n = 0;
	POOLLIST_POP(m_va_pl, n, variant_array);
	
	ARRAY_INI(n->va, m_fk);
	ARRAY_CLEAR(n->va);

	n->isconst = false;
	
	return n;
}

variant_map * container::newmap()
{
	variant_map * n = 0;
	POOLLIST_POP(m_vm_pl, n, variant_map);
	
	HASHMAP_INI(n->vm, m_fk);
	HASHMAP_CLEAR(n->vm);
	
	n->isconst = false;
	
	return n;
}

variant * container::newvariant()
{
	variant * n = 0;
	POOLLIST_POP(m_v_pl, n, variant);
	
	V_SET_NIL(n);
	return n;
}

variant * con_array_get(fake * fk, variant_array * va, const variant * k)
{
	bool err = false;
	int index = 0;
	V_GET_REAL(k, index);
	CHECK_ERR(err);
	if (UNLIKE(err))
	{
		return 0;
	}
	
	if (UNLIKE(index < 0))
	{
		seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk), "interpreter get array fail, index %d", index);
		return 0;
	}
	
	if (UNLIKE(index >= (int)ARRAY_MAX_SIZE(va->va)))
	{
		size_t newsize = index + 1 + ARRAY_MAX_SIZE(va->va) * fk->cfg.array_grow_speed / 100;
		ARRAY_GROW(va->va, newsize, variant *);
	}
	
	if (!ARRAY_GET(va->va, index))
	{
		// 分配个新的
		ARRAY_GET(va->va, index) = va->isconst ? fk->con.newconstvariant() : fk->con.newvariant();
	}

	ARRAY_SIZE(va->va) = FKMAX((int)ARRAY_SIZE(va->va), index + 1);

	return ARRAY_GET(va->va, index);
}

variant * con_map_get(fake * fk, variant_map * vm, const variant * k)
{
	variant * n = 0;
	variant ** p = vm->vm.get(*k);
	if (p)
	{
		n = *p;
		return n;
	}
	n = vm->isconst ? fk->con.newconstvariant() : fk->con.newvariant();
	vm->vm.add(*k, n);
	return n;
}

variant_array * container::newconstarray()
{
	variant_array * n = 0;
	POOLLIST_POP(m_cva_pl, n, variant_array);
	
	ARRAY_INI(n->va, m_fk);
	ARRAY_CLEAR(n->va);
	
	n->isconst = true;
	
	return n;
}

variant_map * container::newconstmap()
{
	variant_map * n = 0;
	POOLLIST_POP(m_cvm_pl, n, variant_map);
	
	HASHMAP_INI(n->vm, m_fk);
	HASHMAP_CLEAR(n->vm);
	
	n->isconst = true;
	
	return n;
}

variant * container::newconstvariant()
{
	variant * n = 0;
	POOLLIST_POP(m_cv_pl, n, variant);
	
	V_SET_NIL(n);
	return n;
}

size_t container::get_map_size() const
{
	return POOL_GROW_SIZE(m_vm_pl.p);
}

size_t container::get_array_size() const
{
	return POOL_GROW_SIZE(m_va_pl.p);
}

size_t container::get_variant_size() const
{
	return POOL_GROW_SIZE(m_v_pl.p);
}

size_t container::get_cmap_size() const
{
	return POOL_GROW_SIZE(m_cvm_pl.p);
}

size_t container::get_carray_size() const
{
	return POOL_GROW_SIZE(m_cva_pl.p);
}

size_t container::get_cvariant_size() const
{
	return POOL_GROW_SIZE(m_cv_pl.p);
}

