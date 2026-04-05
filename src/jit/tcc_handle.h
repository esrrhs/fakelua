#pragma once

#include <memory>

struct TCCState;

namespace fakelua {

class State;

class TCCHandle {
public:
    explicit TCCHandle() = default;

    explicit TCCHandle(State *s);

    ~TCCHandle();

    [[nodiscard]] ::TCCState *GetTCCState() const {
        return tcc_state_;
    }

private:
    ::TCCState *tcc_state_ = nullptr;
};

typedef std::shared_ptr<TCCHandle> TCCHandlePtr;

}// namespace fakelua
