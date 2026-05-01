#pragma once

#include "jit/jit_common.h"
#include "fakelua.h"

struct TCCState;

namespace fakelua {

class State;

class TCCHandle : public JITHandle {
public:
    explicit TCCHandle() = default;

    explicit TCCHandle(State *s, const CompileConfig &cfg);

    ~TCCHandle() override;

    [[nodiscard]] ::TCCState *GetTCCState() const {
        return tcc_state_;
    }

private:
    ::TCCState *tcc_state_ = nullptr;
};

using TCCHandlePtr = std::shared_ptr<TCCHandle>;

}// namespace fakelua
