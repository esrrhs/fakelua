#include "stringheap.h"
#include "fake.h"

stringheap::stringheap(fake *fk) : m_fk(fk), m_shh(fk) {
}

stringheap::~stringheap() {
}

void stringheap::clear() {
    for (const fkhashset<stringele *>::ele *p = m_shh.first(); p != 0; p = m_shh.next()) {
        stringele *e = p->k;
        safe_fkfree(m_fk, e);
    }
    m_shh.clear();
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

