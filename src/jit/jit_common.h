#pragma once

#include <memory>

namespace fakelua {

class JITHandle {
public:
    JITHandle() = default;
    virtual ~JITHandle() = default;
};

typedef std::shared_ptr<JITHandle> JITHandlePtr;

}// namespace fakelua