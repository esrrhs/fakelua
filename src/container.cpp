#include "container.h"
#include "fake.h"

container::container(fake * fk) : m_fk(fk), m_gmap(fk)
{
	POOLLIST_INI(m_va_pl, fk);
	POOLLIST_INI(m_vm_pl, fk);
	POOLLIST_INI(m_v_pl, fk);
	POOLLIST_INI(m_gv_pl, fk);
}

container::~container()
{
	clear();
	POOLLIST_DELETE(m_va_pl);
	POOLLIST_DELETE(m_vm_pl);
	POOLLIST_DELETE(m_v_pl);
	
    POOLLIST_CLEAR(m_gv_pl, variant, USE(n));
	POOLLIST_DELETE(m_gv_pl);
}

void container::clear()
{
    POOLLIST_CLEAR(m_va_pl, variant_array, ARRAY_DEEP_CLEAR(n->t.va, pool<variant_array>::node *));
    POOLLIST_CLEAR(m_vm_pl, variant_map, HASHMAP_CLEAR(n->t.vm));
    POOLLIST_CLEAR(m_v_pl, variant, USE(n));
}

variant_array * container::newarray()
{
    pool<variant_array>::node * n = 0;
    POOLLIST_POP(m_va_pl, n, variant_array, m_fk->cfg.array_grow_speed);
    
    ARRAY_INI(n->t.va, m_fk);
    ARRAY_CLEAR(n->t.va);
    
    return &n->t;
}

variant_map * container::newmap()
{
    pool<variant_map>::node * n = 0;
    POOLLIST_POP(m_vm_pl, n, variant_map, m_fk->cfg.array_grow_speed);
    
    HASHMAP_INI(n->t.vm, m_fk);
    HASHMAP_CLEAR(n->t.vm);
    
    return &n->t;
}

pool<variant>::node * container::newvariant()
{
    pool<variant>::node * n = 0;
    POOLLIST_POP(m_v_pl, n, variant, m_fk->cfg.array_grow_speed);
    
    V_SET_NIL(&(n->t));
    return n;
}

pool<variant>::node * container::newglobalvariant()
{
    pool<variant>::node * n = 0;
    POOLLIST_POP(m_gv_pl, n, variant, m_fk->cfg.array_grow_speed);
    
    V_SET_NIL(&(n->t));
    return n;
}

variant * con_array_get(fake * fk, variant_array * va, const variant * k)
{
    int index = 0;
    V_GET_REAL(k, index);
    if (index >= (int)ARRAY_MAX_SIZE(va->va))
	{
	    size_t newsize = index + 1 + ARRAY_MAX_SIZE(va->va) * fk->cfg.array_grow_speed / 100;
        ARRAY_GROW(va->va, newsize, pool<variant>::node *);
    }
    
	if (!ARRAY_GET(va->va, index))
	{
	    // 分配个新的
	    ARRAY_GET(va->va, index) = fk->con.newvariant();
	}

	return &(ARRAY_GET(va->va, index)->t);
}

variant * con_map_get(fake * fk, variant_map * vm, const variant * k)
{
    pool<variant>::node * n = 0;
    pool<variant>::node ** p = vm->vm.get(*k);
    if (p)
    {
        n = *p;
        return &(n->t);
    }
    n = vm != fk->con.get_gmap() ? fk->con.newvariant() : fk->con.newglobalvariant();
    vm->vm.add(*k, n);
    return &(n->t);
}

