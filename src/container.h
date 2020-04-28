#pragma once

#include "types.h"
#include "hashmap.h"
#include "variant.h"
#include "array.h"
#include "pool.h"

struct fake;

struct variant_container_base {
    bool isconst;
};

struct variant_array : public variant_container_base {
    array<variant *> va;
};

#define VARIANT_ARRAY_DELETE(vva) ARRAY_DELETE((vva).va)

struct variant_map : public variant_container_base {
    fkhashmap<variant, variant *> vm;
};

#define VARIANT_MAP_DELETE(vvm) HASHMAP_DELETE((vvm).vm)

variant *con_array_get(fake *fk, variant_array *va, const variant *k);

variant *con_map_get(fake *fk, variant_map *vm, const variant *k);

class container {
public:
    container(fake *fk);

    ~container();

    void clear();

    variant_array *newarray();

    variant_map *newmap();

    variant *newvariant();

    variant_map *newgmap();

    variant_array *newconstarray();

    variant_map *newconstmap();

    variant *newconstvariant();

    size_t get_map_size() const;

    size_t get_array_size() const;

    size_t get_variant_size() const;

    size_t get_cmap_size() const;

    size_t get_carray_size() const;

    size_t get_cvariant_size() const;

    void checkgc(bool force);

private:
    void gc();

private:
    fake *m_fk;
    variant_map *m_gm;
    size_t m_last_gc_size;

    fkhashset<variant_array *> m_va_pl;

    fkhashset<variant_map *> m_vm_pl;

    fkhashset<variant *> m_v_pl;

    fkhashset<variant_array *> m_cva_pl;

    fkhashset<variant_map *> m_cvm_pl;

    fkhashset<variant *> m_cv_pl;

    array<void *> m_todelete;
};

