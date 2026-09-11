#include "stringheap.h"
#include "fake.h"
#include <vector>

stringheap::stringheap(fake *fk) : m_fk(fk), m_shh(fk), m_chunk(0), m_raw_bytes(0) {
}

stringheap::~stringheap() {
}

void stringheap::clear() {
    for (const fkhashset<stringele *>::ele *p = m_shh.first(); p != 0; p = m_shh.next()) {
        stringele *e = p->k;
        safe_fkfree(m_fk, e);
    }
    m_shh.clear();
    concat_chunk *c = m_chunk;
    while (c) {
        concat_chunk *n = c->next;
        safe_fkfree(m_fk, c);
        c = n;
    }
    m_chunk = 0;
    m_raw_bytes = 0;
}

void stringheap::reset() {
    std::vector<stringele *> drop;
    for (const fkhashset<stringele *>::ele *p = m_shh.first(); p != 0; p = m_shh.next()) {
        stringele *e = p->k;
        if (!e->sysref) {
            drop.push_back(e);
        }
    }
    for (size_t i = 0; i < drop.size(); i++) {
        stringele *e = drop[i];
        m_shh.del(e);
        safe_fkfree(m_fk, e);
    }
    concat_chunk *c = m_chunk;
    while (c) {
        concat_chunk *n = c->next;
        safe_fkfree(m_fk, c);
        c = n;
    }
    m_chunk = 0;
    m_raw_bytes = 0;
}

void stringheap::pin(const variant *v) {
    if (!v) {
        return;
    }
    if (v->type == variant::STRING && v->data.str) {
        if (!v->data.str->sysref) {
            v->data.str->sysref = 1;
        }
        return;
    }
    if (v->type == variant::ARRAY && v->data.va && v->data.va->isconst) {
        for (int i = 0; i < (int) ARRAY_SIZE(v->data.va->va); i++) {
            pin(&ARRAY_GET(v->data.va->va, i));
        }
        return;
    }
    if (v->type == variant::MAP && v->data.vm && v->data.vm->isconst) {
        for (const fkhashmap<variant, variant *>::ele *p = v->data.vm->vm.first(); p != 0; p = v->data.vm->vm.next()) {
            pin(&p->k);
            if (p->t) {
                pin(*p->t);
            }
        }
    }
}

void stringheap::pin_func_binary(const func_binary *fb) {
    if (!fb) {
        return;
    }
    for (int i = 0; i < fb->m_const_list_num; i++) {
        pin(&fb->m_const_list[i]);
    }
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

stringele *stringheap::allocconcat(const stringele *l, const stringele *r, const char *ls, int llen, const char *rs,
                                  int rlen) {
    int len = llen + rlen;
    size_t need = sizeof(stringele) + (size_t) len + 1;
    need = (need + 7) & ~(size_t) 7;
    if (UNLIKE(!m_chunk || m_chunk->used + need > m_chunk->cap)) {
        size_t cap = 256 * 1024;
        if (need > cap) {
            cap = need;
        }
        concat_chunk *c = (concat_chunk *) safe_fkmalloc(m_fk, sizeof(concat_chunk) + cap, emt_stringele);
        c->next = m_chunk;
        c->cap = (uint32_t) cap;
        c->used = 0;
        m_chunk = c;
    }
    stringele *e = (stringele *) ((char *) (m_chunk + 1) + m_chunk->used);
    m_chunk->used += (uint32_t) need;
    e->sz = len;
    e->s = (char *) e + sizeof(stringele);
    e->sysref = 0;
    if (llen > 0) {
        memcpy(e->s, ls, llen);
    }
    if (rlen > 0) {
        memcpy(e->s + llen, rs, rlen);
    }
    e->s[len] = 0;
    USE(l);
    USE(r);
    m_raw_bytes += (size_t) len;
    return e;
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
    ret += m_raw_bytes;
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

