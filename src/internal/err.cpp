#include "err.h"

err::err() {
}

err::err(int code, const std::string &code_str) : m_code(code), m_code_str(code_str) {
}

err::~err() {
}

bool err::empty() const {
    return m_code == 0;
}

int err::code() const {
    return m_code;
}

const std::string &err::str() const {
    return m_code_str;
}
