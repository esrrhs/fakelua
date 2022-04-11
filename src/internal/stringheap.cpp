#include "stringheap.h"
#include "state.h"

const int STRING_INTER_SIZE = 40;

stringheap::stringheap(fakelua_state *l) : m_l(l) {
}

stringheap::~stringheap() {
}

variant stringheap::new_string(const std::string &str) {
    if (str.size() <= m_l->get_config().get_short_str_size()) {
        return new_short_string(str);
    } else {
        return new_long_string(str);
    }
}

variant stringheap::new_short_string(const std::string &str) {
    // TODO
    return variant{};
}

variant stringheap::new_long_string(const std::string &str) {
    return variant{string_object::create(m_l, str)};
}
