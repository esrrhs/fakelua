#pragma once

#include "types.h"
#include "gcobject.h"

class fakelua_state;

class config {
public:
    config(fakelua_state *l);

    ~config();

    config(const config &) = delete;

    config &operator=(const config &) = delete;

public:
    uint32_t get_short_str_size() {
        return m_short_str_size;
    }

private:
    fakelua_state *m_l = nullptr;
    uint32_t m_short_str_size = 40; // Maximum length for short strings
};
