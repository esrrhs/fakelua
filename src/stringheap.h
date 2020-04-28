#pragma once

#include "types.h"
#include "hashmap.h"
#include "array.h"

struct fake;

class stringheap {
public:
    stringheap(fake *fk);

    ~stringheap();

    void clear();

    stringele *allocstring(const char *str);

    variant allocsysstr(const char *str);

    void checkgc(bool force);

    const char *dump();

    force_inline size_t size() const {
        return m_shh.size();
    }

    size_t sys_size() const;

    size_t bytesize() const;

    size_t sys_bytesize() const;

private:
    void gc();

private:
    fake *m_fk;
    fkhashset<stringele *> m_shh;
    array<stringele *> m_todelete;
    String m_dumpstr;
    size_t m_last_gc_size;
};

