#pragma once

#include "fakelua.h"
#include "jit/jit_common.h"

struct TCCState;

namespace fakelua {

class State;
struct LuaLoc;

class TCCHandle : public JITHandle {
public:
    explicit TCCHandle() = default;

    explicit TCCHandle(State *s, const CompileConfig &cfg);

    ~TCCHandle() override;

    [[nodiscard]] ::TCCState *GetTCCState() const {
        return tcc_state_;
    }

    void SetLuaLoc(LuaLoc *loc) {
        lua_loc_ = loc;
    }

private:
    State *s_ = nullptr;
    ::TCCState *tcc_state_ = nullptr;
    LuaLoc *lua_loc_ = nullptr;
};

using TCCHandlePtr = std::shared_ptr<TCCHandle>;

}// namespace fakelua
