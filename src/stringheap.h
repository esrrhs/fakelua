#pragma once

#include "types.h"
#include "hashmap.h"
#include "array.h"

struct fake;
struct variant;
struct func_binary;

struct concat_chunk {
    concat_chunk *next;
    uint32_t cap;
    uint32_t used;
};

class stringheap {
public:
    stringheap(fake *fk);

    ~stringheap();

    void clear();

    void reset();

    void pin(const variant *v);

    void pin_func_binary(const func_binary *fb);

    stringele *allocstring(const char *str);
    stringele *allocconcat(const stringele *l, const stringele *r, const char *ls, int llen, const char *rs, int rlen);

    variant allocsysstr(const char *str);

    const char *dump();

    force_inline size_t size() const {
        return m_shh.size();
    }

    size_t sys_size() const;

    size_t bytesize() const;

    size_t sys_bytesize() const;

private:
    fake *m_fk;
    fkhashset<stringele *> m_shh;
    concat_chunk *m_chunk;
    size_t m_raw_bytes;
    String m_dumpstr;
};
