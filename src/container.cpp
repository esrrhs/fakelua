#include "container.h"
#include "fake.h"

variant *con_array_get(fake *fk, variant_array *va, const variant *k) {
    bool err = false;
    int index = 0;
    V_GET_REAL(k, index);
    CHECK_ERR(err);
    if (UNLIKE(err)) {
        return 0;
    }

    if (UNLIKE(index < 0)) {
        seterror(fk, efk_run_inter_error, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk),
                 "interpreter get array fail, index %d", index);
        return 0;
    }

    if (UNLIKE(index >= (int) ARRAY_MAX_SIZE(va->va))) {
        size_t newsize = index + 1 + ARRAY_MAX_SIZE(va->va) * fk->cfg.array_grow_speed / 100;
        ARRAY_GROW(va->va, newsize, variant *);
    }

    if (!ARRAY_GET(va->va, index)) {
        // ????????
        ARRAY_GET(va->va, index) = va->isconst ? fk->con.newconstvariant() : fk->con.newvariant();
    }

    ARRAY_SIZE(va->va) = FKMAX((int) ARRAY_SIZE(va->va), index + 1);

    return ARRAY_GET(va->va, index);
}

variant *con_map_get(fake *fk, variant_map *vm, const variant *k) {
    variant *n = 0;
    variant **p = vm->vm.get(*k);
    if (p) {
        n = *p;
        return n;
    }
    n = vm->isconst ? fk->con.newconstvariant() : fk->con.newvariant();
    vm->vm.add(*k, n);
    return n;
}

container::container(fake *fk) : m_fk(fk), m_gm(0), m_va_pl(fk),
                                 m_vm_pl(fk), m_v_pl(fk), m_cva_pl(fk), m_cvm_pl(fk), m_cv_pl(fk) {
}

container::~container() {
}

void container::clear() {
    for (const fkhashset<variant_array *>::ele *p = m_va_pl.first(); p != 0; p = m_va_pl.next()) {
        variant_array *e = p->k;
        ARRAY_DEEP_CLEAR(e->va, variant_array *);
        safe_fkfree(m_fk, e);
    }
    m_va_pl.clear();

    for (const fkhashset<variant_map *>::ele *p = m_vm_pl.first(); p != 0; p = m_vm_pl.next()) {
        variant_map *e = p->k;
        HASHMAP_CLEAR(e->vm);
        safe_fkfree(m_fk, e);
    }
    m_vm_pl.clear();

    for (const fkhashset<variant *>::ele *p = m_v_pl.first(); p != 0; p = m_v_pl.next()) {
        variant *e = p->k;
        safe_fkfree(m_fk, e);
    }
    m_v_pl.clear();

    for (const fkhashset<variant_array *>::ele *p = m_cva_pl.first(); p != 0; p = m_cva_pl.next()) {
        variant_array *e = p->k;
        ARRAY_DEEP_CLEAR(e->va, variant_array *);
        safe_fkfree(m_fk, e);
    }
    m_cva_pl.clear();

    for (const fkhashset<variant_map *>::ele *p = m_cvm_pl.first(); p != 0; p = m_cvm_pl.next()) {
        variant_map *e = p->k;
        HASHMAP_CLEAR(e->vm);
        safe_fkfree(m_fk, e);
    }
    m_cvm_pl.clear();

    for (const fkhashset<variant *>::ele *p = m_cv_pl.first(); p != 0; p = m_cv_pl.next()) {
        variant *e = p->k;
        safe_fkfree(m_fk, e);
    }
    m_cv_pl.clear();
}

variant_array *container::newarray() {
    variant_array *n = (variant_array *) safe_fkmalloc(m_fk, sizeof(variant_array), emt_array);
    memset(n, 0, sizeof(variant_array));

    ARRAY_INI(n->va, m_fk);
    n->isconst = false;

    return m_va_pl.add(n)->k;
}

variant_map *container::newmap() {
    variant_map *n = (variant_map *) safe_fkmalloc(m_fk, sizeof(variant_map), emt_map);
    memset(n, 0, sizeof(variant_map));

    HASHMAP_INI(n->vm, m_fk);
    HASHMAP_CLEAR(n->vm);

    n->isconst = false;

    return m_vm_pl.add(n)->k;
}

variant_map *container::newgmap() {
    if (!m_gm) {
        m_gm = newmap();
    }

    return m_gm;
}

variant *container::newvariant() {
    variant *n = (variant *) safe_fkmalloc(m_fk, sizeof(variant), emt_variant);
    V_SET_NIL(n);
    return m_v_pl.add(n)->k;
}

variant_array *container::newconstarray() {
    variant_array *n = (variant_array *) safe_fkmalloc(m_fk, sizeof(variant_array), emt_array);
    memset(n, 0, sizeof(variant_array));

    ARRAY_INI(n->va, m_fk);
    ARRAY_CLEAR(n->va);

    n->isconst = true;

    return m_cva_pl.add(n)->k;
}

variant_map *container::newconstmap() {
    variant_map *n = (variant_map *) safe_fkmalloc(m_fk, sizeof(variant_map), emt_map);
    memset(n, 0, sizeof(variant_map));

    HASHMAP_INI(n->vm, m_fk);
    HASHMAP_CLEAR(n->vm);

    n->isconst = true;

    return m_cvm_pl.add(n)->k;
}

variant *container::newconstvariant() {
    variant *n = (variant *) safe_fkmalloc(m_fk, sizeof(variant), emt_variant);
    V_SET_NIL(n);
    return m_cv_pl.add(n)->k;
}

size_t container::get_map_size() const {
    return m_vm_pl.size();
}

size_t container::get_array_size() const {
    return m_va_pl.size();
}

size_t container::get_variant_size() const {
    return m_v_pl.size();
}

size_t container::get_cmap_size() const {
    return m_cvm_pl.size();
}

size_t container::get_carray_size() const {
    return m_cva_pl.size();
}

size_t container::get_cvariant_size() const {
    return m_cv_pl.size();
}
