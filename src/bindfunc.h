#pragma once

#include "types.h"
#include "hashmap.h"

struct interpreter;
struct funcunion;

class bindfunc {
public:
    force_inline bindfunc(fake *fk) : m_fk(fk) {
    }

    force_inline ~bindfunc() {
    }

    force_inline fake *getfake() {
        return m_fk;
    }

    force_inline void clear() {
    }

    void addfunc(const variant &name, const fkfunctor &ff);

private:
    fake *m_fk;
};

// 参数和返回值都在m_fk->ps里
#define BIND_FUNC_CALL(f, inter) \
    assert((f)->haveff); \
    (f)->ff.ff(m_fk, &(f)->ff)
