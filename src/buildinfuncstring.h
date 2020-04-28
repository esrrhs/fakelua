#pragma once

#include "types.h"
#include "hashmap.h"

struct interpreter;
struct funcunion;

class buildinfuncstring {
public:
    force_inline buildinfuncstring(fake *fk) : m_fk(fk) {
    }

    force_inline ~buildinfuncstring() {
        clear();
    }

    force_inline fake *getfake() {
        return m_fk;
    }

    force_inline void clear() {
    }

    void openstringfunc();

private:
    fake *m_fk;
};

