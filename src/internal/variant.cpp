#include "variant.h"

variant::variant() {
}

variant::variant(string_object *s) : m_data(s) {
    std::get<int>(m_data);
}

variant::~variant() {
}
