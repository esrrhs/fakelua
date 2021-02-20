#include "stringheap.h"
#include "state.h"

stringheap::stringheap(fakelua_state *l) : m_l(l) {
}

stringheap::~stringheap() {
}

variant stringheap::new_string(const std::string &str) {
    // TODO
    return variant{};
}