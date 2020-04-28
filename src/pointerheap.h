#pragma once

#include "types.h"
#include "hashmap.h"
#include "array.h"

struct fake;

struct pointerele {
    void *ptr;
    char *type;
    uint32_t typesz;
};

class pointerheap {
public:
    pointerheap(fake *fk);

    ~pointerheap();

    void clear();

    pointerele *allocpointer(void *ptr, const char *type);

    void checkgc(bool force);

    const char *dump();

    force_inline size_t size() const {
        return m_shh.size();
    }

private:
    void gc();

private:
    fake *m_fk;
    fkhashmap<void *, pointerele> m_shh;
    String m_dumpstr;
    array<pointerele *> m_todelete;
    size_t m_last_gc_size;
};

