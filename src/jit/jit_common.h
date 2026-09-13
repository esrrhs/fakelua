#pragma once

#include <memory>

namespace fakelua {

class JITHandle {
public:
    JITHandle() = default;
    virtual ~JITHandle() = default;
};

using JITHandlePtr = std::shared_ptr<JITHandle>;

}// namespace fakelua