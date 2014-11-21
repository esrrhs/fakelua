#include "container.h"
#include "fake.h"

container::container(fake * fk) : m_fk(fk)
{
	POOL_INI(m_va_pool, fk);
	POOL_INI(m_vm_pool, fk);
	POOL_INI(m_vpool, fk);
	ARRAY_INI(m_va_list, fk);
	ARRAY_INI(m_vm_list, fk);
	ARRAY_INI(m_vlist, fk);
}

container::~container()
{
	clear();
	POOL_DELETE(m_va_pool);
	POOL_DELETE(m_vm_pool);
	POOL_DELETE(m_vpool);
	ARRAY_DELETE(m_va_list);
	ARRAY_DELETE(m_vm_list);
	ARRAY_DELETE(m_vlist);
}

void container::clear()
{
    for (int i = 0; i < (int)ARRAY_SIZE(m_va_list); i++)
    {
        pool<variant_array>::node * n = ARRAY_GET(m_va_list, i);
        variant_array & va = n->t;
        ARRAY_CLEAR(va.va);
        ARRAY_DEEP_CLEAR(va.va, pool<variant_array>::node *);
        POOL_PUSH(m_va_pool, n);
    }
    ARRAY_CLEAR(m_va_list);
    for (int i = 0; i < (int)ARRAY_SIZE(m_vm_list); i++)
    {
        pool<variant_map>::node * n = ARRAY_GET(m_vm_list, i);
        variant_map & vm = n->t;
        HASHMAP_CLEAR(vm.vm);
        POOL_PUSH(m_vm_pool, n);
    }
    ARRAY_CLEAR(m_vm_list);
    for (int i = 0; i < (int)ARRAY_SIZE(m_vlist); i++)
    {
        pool<variant>::node * n = ARRAY_GET(m_vlist, i);
        POOL_PUSH(m_vpool, n);
    }
    ARRAY_CLEAR(m_vlist);
}

variant_array * container::newarray()
{
    pool<variant_array>::node * n = 0;
    if (POOL_EMPTY(m_va_pool))
	{
        POOL_GROW(m_va_pool, pool<variant_array>::node, n);
        ARRAY_INI(n->t.va, m_fk);
    }

    // 分配
    POOL_POP(m_va_pool, n);
    assert(n);

    ARRAY_CLEAR(n->t.va);
    
    // 添加
    if (ARRAY_SIZE(m_va_list) >= ARRAY_MAX_SIZE(m_va_list))
	{
	    size_t newsize = ARRAY_MAX_SIZE(m_va_list) + 1 + ARRAY_MAX_SIZE(m_va_list) * m_fk->cfg.array_grow_speed / 100;
        ARRAY_GROW(m_va_list, newsize, pool<variant_array>::node *);
    }
	ARRAY_PUSH_BACK(m_va_list);
	ARRAY_BACK(m_va_list) = n;
    
    return &n->t;
}

variant_map * container::newmap()
{
    pool<variant_map>::node * n = 0;
    if (POOL_EMPTY(m_vm_pool))
	{
        POOL_GROW(m_vm_pool, pool<variant_map>::node, n);
        HASHMAP_INI(n->t.vm, m_fk);
    }

    // 分配
    POOL_POP(m_vm_pool, n);
    assert(n);

    HASHMAP_CLEAR(n->t.vm);
    
    // 添加
    if (ARRAY_SIZE(m_vm_list) >= ARRAY_MAX_SIZE(m_vm_list))
	{
	    size_t newsize = ARRAY_MAX_SIZE(m_vm_list) + 1 + ARRAY_MAX_SIZE(m_vm_list) * m_fk->cfg.array_grow_speed / 100;
        ARRAY_GROW(m_vm_list, newsize, pool<variant_map>::node *);
    }
	ARRAY_PUSH_BACK(m_vm_list);
	ARRAY_BACK(m_vm_list) = n;
    
    return &n->t;
}

pool<variant>::node * container::newvariant()
{
    pool<variant>::node * n = 0;
    if (POOL_EMPTY(m_vpool))
	{
        POOL_GROW(m_vpool, pool<variant>::node, n);
        V_SET_NIL(&(n->t));
    }

    // 分配
    POOL_POP(m_vpool, n);
    assert(n);
    
    // 添加
    if (ARRAY_SIZE(m_vlist) >= ARRAY_MAX_SIZE(m_vlist))
	{
	    size_t newsize = ARRAY_MAX_SIZE(m_vlist) + 1 + ARRAY_MAX_SIZE(m_vlist) * m_fk->cfg.array_grow_speed / 100;
        ARRAY_GROW(m_vlist, newsize, pool<variant>::node *);
    }
	ARRAY_PUSH_BACK(m_vlist);
	ARRAY_BACK(m_vlist) = n;
    
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
    n = fk->con.newvariant();
    vm->vm.add(*k, n);
    return &(n->t);
}

