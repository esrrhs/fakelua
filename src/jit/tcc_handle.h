#pragma once

#include "jit/jit_common.h"

struct TCCState;

namespace fakelua {

class State;

class TCCHandle : public JITHandle {
public:
    explicit TCCHandle() = default;

    explicit TCCHandle(State *s);

    ~TCCHandle() override;

    [[nodiscard]] ::TCCState *GetTCCState() const {
        return tcc_state_;
    }

private:
    ::TCCState *tcc_state_ = nullptr;
};

typedef std::shared_ptr<TCCHandle> TCCHandlePtr;

}// namespace fakelua
