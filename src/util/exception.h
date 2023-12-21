#pragma once

#include "common.h"

namespace fakelua {

class fakelua_exception : public std::runtime_error {
public:
    fakelua_exception(const std::string &msg) : std::runtime_error(msg) {
    }
};

[[noreturn]] void throw_fakelua_exception(const std::string &msg);

}// namespace fakelua
