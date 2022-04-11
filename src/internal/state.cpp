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

void fakelua_state::throw_err(const err &e) {
    fakelua_exception fe;
    fe.code = static_cast<fakelua_error>(e.code());
    string_to_cstr<fakelua_exception_string_length>(fe.msg, e.str());
    throw fe;
}
