#include "stringheap.h"
#include "fake.h"

stringheap::stringheap(fake *fk) : m_fk(fk), m_shh(fk), m_last_gc_size(0) {
    ARRAY_INI(m_todelete, m_fk);
}

stringheap::~stringheap() {
}

void stringheap::clear() {
    for (const fkhashset<stringele *>::ele *p = m_shh.first(); p != 0; p = m_shh.next()) {
        stringele *e = p->k;
        safe_fkfree(m_fk, e);
    }
    m_shh.clear();
    ARRAY_DELETE(m_todelete);
}

stringele *stringheap::allocstring(const char *str) {
    stringele tmp;
    tmp.s = (char *) str;
    fkhashset<stringele *>::ele *p = m_shh.get(&tmp);
    if (LIKE(p != 0)) {
        return p->k;
    }
    int len = strlen(str);
    stringele *e = (stringele *) safe_fkmalloc(m_fk, sizeof(stringele) + len + 1, emt_stringele);
    e->sz = len;
    e->s = (char *) e + sizeof(stringele);
    e->sysref = 0;
    if (LIKE(e->sz > 0)) {
        memcpy(e->s, str, e->sz);
    }
    e->s[e->sz] = 0;
    stringele *ret = m_shh.add(e)->k;
    return ret;
}

variant stringheap::allocsysstr(const char *str) {
    fake *fk = m_fk;
    variant v;
    V_SET_STRING(&v, str);
    v.data.str->sysref++;
    return v;
}

void stringheap::checkgc(bool force) {
    size_t newsize = m_last_gc_size + 1 + m_last_gc_size * (m_fk->cfg.gc_grow_speed) / 100;
    if (UNLIKE(force || (int) m_shh.size() >= (int) newsize)) {
        // ¼ÇÂ¼profile
        uint32_t s = 0;
        if (m_fk->pf.isopen()) {
            s = fkgetmstick();
        }

        gc();

        if (m_fk->pf.isopen()) {
            m_fk->pf.add_gc_sample(egt_string, fkgetmstick() - s);
        }

        m_last_gc_size = m_shh.size();
    }
}

void stringheap::gc() {
    ARRAY_CLEAR(m_todelete);

    int before = m_shh.size();
    USE(before);

    array<void *> &used = m_fk->g.get_used_stringele();

    fkhashset<void *> usedset(m_fk);
    for (int i = 0; LIKE(i < (int) ARRAY_SIZE(used)); i++) {
        stringele *n = (stringele *) ARRAY_GET(used, i);
        usedset.add(n);
    }

    for (const fkhashset<stringele *>::ele *p = m_shh.first(); p != 0; p = m_shh.next()) {
        stringele *n = p->k;

        if (UNLIKE(n->sysref)) {
            continue;
        }

        fkhashset<void *>::ele *pp = usedset.get(n);
        if (UNLIKE(pp != 0)) {
            continue;
        }

        if (ARRAY_SIZE(m_todelete) >= ARRAY_MAX_SIZE(m_todelete)) {
            size_t newsize =
                    ARRAY_SIZE(m_todelete) + 1 + ARRAY_MAX_SIZE(m_todelete) * (m_fk->cfg.array_grow_speed) / 100;
            ARRAY_GROW(m_todelete, newsize, stringele *);
        }
        ARRAY_PUSH_BACK(m_todelete);
        ARRAY_BACK(m_todelete) = n;
    }

    for (int i = 0; i < (int) ARRAY_SIZE(m_todelete); i++) {
        stringele *e = ARRAY_GET(m_todelete, i);
        m_shh.del(e);
        safe_fkfree(m_fk, e);
    }

    ARRAY_CLEAR(used);
    ARRAY_CLEAR(m_todelete);
    m_dumpstr.clear();

    int end = m_shh.size();
    USE(end);

    FKLOG("stringheap %p gc from %d to %d", m_fk, before, end);
}

const char *stringheap::dump() {
    m_dumpstr.clear();
    for (const fkhashset<stringele *>::ele *p = m_shh.first(); p != 0; p = m_shh.next()) {
        stringele *e = p->k;
        m_dumpstr += e->s;
        if (e->sysref) {
            m_dumpstr += "(system)";
        }
        m_dumpstr += "\n";
    }
    return m_dumpstr.c_str();
}

size_t stringheap::sys_size() const {
    size_t ret = 0;
    for (const fkhashset<stringele *>::ele *p = m_shh.first(); p != 0; p = m_shh.next()) {
        stringele *e = p->k;
        if (e->sysref) {
            ret++;
        }
    }
    return ret;
}

size_t stringheap::bytesize() const {
    size_t ret = 0;
    for (const fkhashset<stringele *>::ele *p = m_shh.first(); p != 0; p = m_shh.next()) {
        stringele *e = p->k;
        ret += e->sz;
    }
    return ret;
}

size_t stringheap::sys_bytesize() const {
    size_t ret = 0;
    for (const fkhashset<stringele *>::ele *p = m_shh.first(); p != 0; p = m_shh.next()) {
        stringele *e = p->k;
        if (e->sysref) {
            ret += e->sz;
        }
    }
    return ret;
}

