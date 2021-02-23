#include "variant.h"

variant::variant() {
}

variant::variant(string_object *s) {
    m_data.s = s;
}

variant::~variant() {
}
