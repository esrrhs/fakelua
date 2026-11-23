#pragma once
#include "state/const_string.h"
#include "state/state.h"
#include <cstring>

namespace fakelua {
inline LuaLoc *BindGeneratedCode(void *symbol, State *s, JITType jit_type, const std::string &filename) {
    if (!symbol) ThrowFakeluaException("JIT image has no binding entry point");
    using Intern = int64_t (*)(void *, const char *, size_t);
    using Bind = void *(*) (void *, void *, void *, const char *, Intern, void *, const void *);
    // Host-owned bytes survive unloading/recompilation of another image.
    auto *file = static_cast<char *>(s->GetHeap().GetAllocator(true).Alloc(filename.size() + 1));
    std::memcpy(file, filename.c_str(), filename.size() + 1);
    return static_cast<LuaLoc *>(reinterpret_cast<Bind>(symbol)(
            s, &s->GetTempHeapSlot(), &s->GetConstHeapSlot(), file,
            +[](void *state, const char *data, size_t size) -> int64_t { return static_cast<State *>(state)->GetConstString().Alloc(std::string_view(data, size)); },
            &s->GetActiveLuaLoc(jit_type), s->GetVM().VersionPtr()));
}
}// namespace fakelua
