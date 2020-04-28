#pragma once

#include "types.h"
#include "hashmap.h"
#include "buildinfunc.h"
#include "binary.h"
#include "bindfunc.h"
#include "native.h"

struct funcunion {
    fkfunctor ff;
    bifunc bif;
    func_binary fb;
    func_native fn;
    bool haveff;
    bool havebif;
    bool havefb;
    bool havefn;
};

class funcmap {
    friend class binary;

    friend class native;

    friend class assembler;

public:
    force_inline funcmap(fake *fk) : m_fk(fk), m_shh(fk) {
    }

    force_inline ~funcmap() {
    }

    force_inline fake *getfake() {
        return m_fk;
    }

    force_inline void clear() {
        clearfb();
        m_shh.clear();
        m_dump.clear();
    }

    force_inline void clearfb() {
        for (fkhashmap<variant, funcunion>::ele *p = m_shh.first(); p != 0; p = m_shh.next()) {
            funcunion &f = *p->t;
            if (f.havefb) {
                FUNC_BINARY_DELETE(f.fb);
            }
            f.havefb = false;
        }
    }

    force_inline const funcunion *get_func(const variant &name) const {
        return m_shh.get(name);
    }

    force_inline void add_func(const variant &name, const func_binary &fb) {
        funcunion *f = add_func_union(name);
        if (f->havefb) {
            FUNC_BINARY_DELETE(f->fb);
        }
        f->fb = fb;
        f->havefb = true;
    }

    force_inline void add_bind_func(const variant &name, const fkfunctor &ff) {
        funcunion *f = add_func_union(name);
        f->ff = ff;
        f->haveff = true;
    }

    force_inline void add_buildin_func(const variant &name, const bifunc &bif) {
        funcunion *f = add_func_union(name);
        f->bif = bif;
        f->havebif = true;
    }

    force_inline void add_func_native(const variant &name, const func_native &fn) {
        funcunion *f = add_func_union(name);
        if (f->havefn) {
            FUNC_NATIVE_DELETE(f->fn);
        }
        f->fn = fn;
        f->havefn = true;
    }

    String &dump();

    force_inline size_t size() const {
        return m_shh.size();
    }

private:
    force_inline funcunion *add_func_union(const variant &name) {
        funcunion *p = m_shh.get(name);
        if (p) {
            return p;
        }

        funcunion tmp;
        memset(&tmp, 0, sizeof(tmp));
        return m_shh.add(name, tmp)->t;
    }

private:
    fake *m_fk;
    fkhashmap<variant, funcunion> m_shh;
    String m_dump;
};
