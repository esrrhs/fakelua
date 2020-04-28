#include "pointerheap.h"
#include "fake.h"

pointerheap::pointerheap(fake *fk) : m_fk(fk), m_shh(fk), m_last_gc_size(0) {
    ARRAY_INI(m_todelete, m_fk);
}

pointerheap::~pointerheap() {
}

void pointerheap::clear() {
    for (const fkhashmap<void *, pointerele>::ele *p = m_shh.first(); p != 0; p = m_shh.next()) {
        pointerele *e = p->t;
        safe_fkfree(m_fk, e->type);
    }
    m_shh.clear();
    m_dumpstr.clear();
    ARRAY_DELETE(m_todelete);
}

pointerele *pointerheap::allocpointer(void *ptr, const char *type) {
    pointerele *p = m_shh.get(ptr);
    if (LIKE(p != 0)) {
        if (UNLIKE(strcmp(p->type, type) != 0)) {
            safe_fkfree(m_fk, p->type);
            p->typesz = strlen(type);
            p->type = stringdump(m_fk, type, p->typesz);
        }
        return p;
    }

    pointerele e;
    e.ptr = ptr;
    e.typesz = strlen(type);
    e.type = stringdump(m_fk, type, e.typesz);

    return m_shh.add(ptr, e)->t;
}

void pointerheap::checkgc(bool force) {
    size_t newsize = m_last_gc_size + 1 + m_last_gc_size * (m_fk->cfg.gc_grow_speed) / 100;
    if (UNLIKE(force || (int) m_shh.size() >= (int) newsize)) {
        // ¼ÇÂ¼profile
        uint32_t s = 0;
        if (m_fk->pf.isopen()) {
            s = fkgetmstick();
        }

        gc();

        if (m_fk->pf.isopen()) {
            m_fk->pf.add_gc_sample(egt_pointer, fkgetmstick() - s);
        }

        m_last_gc_size = m_shh.size();
    }
}

void pointerheap::gc() {
    ARRAY_CLEAR(m_todelete);

    int before = m_shh.size();
    USE(before);

    array<void *> &used = m_fk->g.get_used_pointer();

    fkhashset<void *> usedset(m_fk);
    for (int i = 0; LIKE(i < (int) ARRAY_SIZE(used)); i++) {
        pointerele *n = (pointerele *) ARRAY_GET(used, i);
        usedset.add(n);
    }

    for (const fkhashmap<void *, pointerele>::ele *p = m_shh.first(); p != 0; p = m_shh.next()) {
        pointerele *n = p->t;

        fkhashset<void *>::ele *pp = usedset.get(n);
        if (UNLIKE(pp != 0)) {
            continue;
        }

        if (ARRAY_SIZE(m_todelete) >= ARRAY_MAX_SIZE(m_todelete)) {
            size_t newsize =
                    ARRAY_SIZE(m_todelete) + 1 + ARRAY_MAX_SIZE(m_todelete) * (m_fk->cfg.array_grow_speed) / 100;
            ARRAY_GROW(m_todelete, newsize, pointerele *);
        }
        ARRAY_PUSH_BACK(m_todelete);
        ARRAY_BACK(m_todelete) = n;
    }

    for (int i = 0; i < (int) ARRAY_SIZE(m_todelete); i++) {
        pointerele *e = ARRAY_GET(m_todelete, i);
        safe_fkfree(m_fk, e->type);
        m_shh.del(e->ptr);
    }

    ARRAY_CLEAR(used);
    ARRAY_CLEAR(m_todelete);
    m_dumpstr.clear();

    int end = m_shh.size();
    USE(end);

    FKLOG("pointerheap %p gc from %d to %d", m_fk, before, end);
}

const char *pointerheap::dump() {
    m_dumpstr.clear();
    for (const fkhashmap<void *, pointerele>::ele *p = m_shh.first(); p != 0; p = m_shh.next()) {
        pointerele *e = p->t;
        m_dumpstr += fkptoa(e->ptr);
        m_dumpstr += "(";
        m_dumpstr += e->type;
        m_dumpstr += ")\n";
    }
    return m_dumpstr.c_str();
}

