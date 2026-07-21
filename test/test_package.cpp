#include "compile/compiler.h"
#include "fakelua.h"
#include "gtest/gtest.h"

#include <functional>
#include <vector>

using namespace fakelua;

static std::vector<JITType> GetSupportedJitTypes() {
    return {JIT_TCC, JIT_GCC};
}

static void PackageRunHelper(const std::function<void(State *, JITType, bool)> &f) {
    const auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    for (const auto type: GetSupportedJitTypes()) {
        f(s, type, true);
        f(s, type, false);
    }
    FakeluaDeleteState(s);
}

TEST(package, test_package_basic) {
    PackageRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./package/player.lua", {.debug_mode = debug_mode});
        CompileFile(s, "./package/bag.lua", {.debug_mode = debug_mode});
        CompileFile(s, "./package/test_package.lua", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 520);
    });
}
