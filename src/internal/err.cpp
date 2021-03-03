#include "err.h"

err::err() {
}

err::err(int code, const std::string &code_str) : m_code(code), m_code_str(code_str) {
}

err::~err() {
}

bool err::empty() {
    return m_code == 0;
}

int err::code() {
    return m_code;
}

const std::string &err::str() {
    return m_code_str;
}
