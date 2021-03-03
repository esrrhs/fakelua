#include "state.h"

fakelua_state::fakelua_state() : m_config(this), m_sh(this), m_gc(this) {
}


fakelua_state::~fakelua_state() {
}

stringheap &fakelua_state::get_stringheap() {
    return m_sh;
}

gc &fakelua_state::get_gc() {
    return m_gc;
}

config &fakelua_state::get_config() {
    return m_config;
}

err &fakelua_state::get_err() {
    return m_err;
}

void fakelua_state::add_err(const err &e) {
    if (m_err.empty()) {
        m_err = e;
    }
}
