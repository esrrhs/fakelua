#pragma once

#include "types.h"
#include "variant.h"
#include "native.h"
#include "paramstack.h"

struct fake;

class machine {
public:
    force_inline machine(fake *fk) : m_fk(fk), m_isend(false) {
    }

    force_inline ~machine() {
    }

    force_inline void clear() {
        m_isend = false;
    }

    force_inline bool isend() const {
        return m_isend;
    }

    void call(const variant &func);

    static void static_call(fake *fk, const variant &func);

private:
    fake *m_fk;
    bool m_isend;
};

