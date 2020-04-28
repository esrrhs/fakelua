#pragma once

#include "types.h"
#include "hashmap.h"
#include "binary.h"

struct profilefuncele {
    int callnum;
    uint32_t calltime;
};

struct profilememele {
    e_mem_type type;
    uint32_t size;
};

class profile {
public:
    force_inline profile(fake *fk) : m_fk(fk), m_isopen(false), m_shh(fk), m_gcshh(fk) {
    }

    force_inline ~profile() {
        clear();
    }

    force_inline fake *getfake() {
        return m_fk;
    }

    force_inline void clear() {
        m_isopen = false;
        m_shh.clear();
        m_gcshh.clear();
        m_memuse.clear();
        m_dumpstr.clear();
        memset(m_codetype, 0, sizeof(m_codetype));
        m_memmalloc_num = 0;
        m_memfree_num = 0;
        m_memmalloc_size = 0;
        m_memfree_size = 0;
    }

    force_inline void open() {
        m_isopen = true;
    }

    force_inline void close() {
        m_isopen = false;
    }

    force_inline bool isopen() {
        return m_isopen;
    }

    void add_func_sample(const char *func, uint32_t calltime);

    void add_code_sample(int code);

    void add_gc_sample(int type, uint32_t calltime);

    const char *dump();

    const char *dumpstat();

    const char *dumpmem();

    void add_mem(void *p, size_t size, e_mem_type type);

    void dec_mem(void *p);

private:
    fake *m_fk;
    bool m_isopen;
    typedef fkhashmap<const char *, profilefuncele> stringhashmap;
    stringhashmap m_shh;
    typedef fkhashmap<int, profilefuncele> gchashmap;
    gchashmap m_gcshh;
    String m_dumpstr;
    int m_codetype[OPCODE_MAX];
    typedef std::map<void *, profilememele> memhashmap;
    memhashmap m_memuse;    // ∑¿÷πµ›πÈ
    size_t m_memmalloc_num;
    size_t m_memfree_num;
    size_t m_memmalloc_size;
    size_t m_memfree_size;
};

