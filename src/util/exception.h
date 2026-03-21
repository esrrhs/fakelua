#pragma once

namespace fakelua {

class FakeluaException : public std::runtime_error {
public:
    explicit FakeluaException(const std::string &msg) : std::runtime_error(msg) {
    }
};

[[noreturn]] void ThrowFakeluaException(const std::string &msg);

}// namespace fakelua
