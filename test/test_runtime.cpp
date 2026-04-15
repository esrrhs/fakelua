#include "fakelua.h"
#include "jit/vm.h"
#include "state/heap.h"
#include "state/state.h"
#include "util/file_util.h"
#include "util/os.h"
#include "var/var.h"
#include "gtest/gtest.h"
#include <filesystem>

using namespace fakelua;

static CVar VmFn0() {
    Var ret;
    ret.SetInt(123);
    return ret;
}

static CVar VmFnEchoFirst(CVar first, ...) {
    return first;
}

static CVar callVmWithNArgs(State *s, const char *name, int n) {
    CVar args[9];
    for (int i = 0; i < 9; ++i) {
        args[i] = inter::NativeToFakeluaInt(s, 100 + i);
    }

    switch (n) {
        case 1: return FakeluaCallByName(s, JIT_TCC, name, 1, args[0]);
        case 2: return FakeluaCallByName(s, JIT_TCC, name, 2, args[0], args[1]);
        case 3: return FakeluaCallByName(s, JIT_TCC, name, 3, args[0], args[1], args[2]);
        case 4: return FakeluaCallByName(s, JIT_TCC, name, 4, args[0], args[1], args[2], args[3]);
        case 5: return FakeluaCallByName(s, JIT_TCC, name, 5, args[0], args[1], args[2], args[3], args[4]);
        case 6: return FakeluaCallByName(s, JIT_TCC, name, 6, args[0], args[1], args[2], args[3], args[4], args[5]);
        case 7: return FakeluaCallByName(s, JIT_TCC, name, 7, args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
        case 8: return FakeluaCallByName(s, JIT_TCC, name, 8, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
        default: break;
    }
    return {};
}

TEST(runtime, exec_returns_output) {
    const std::string out = Exec("echo fakelua_runtime_test");
    ASSERT_NE(out.find("fakelua_runtime_test"), std::string::npos);
}

TEST(runtime, vm_call_by_name_success_cases) {
    State s;
    s.GetVM().RegisterFunction(VmFunction("fn0", 0, reinterpret_cast<void *>(&VmFn0), {}));
    s.GetVM().RegisterFunction(VmFunction("fnv", 8, reinterpret_cast<void *>(&VmFnEchoFirst), {}));

    const CVar r0 = FakeluaCallByName(&s, JIT_TCC, "fn0", 0);
    ASSERT_EQ(inter::FakeluaToNativeInt(&s, r0), 123);

    for (int n = 1; n <= 8; ++n) {
        const CVar ret = callVmWithNArgs(&s, "fnv", n);
        ASSERT_EQ(inter::FakeluaToNativeInt(&s, ret), 100);
    }
}

TEST(runtime, vm_call_by_name_error_cases) {
    State s;
    CVar a0 = inter::NativeToFakeluaInt(&s, 0);
    CVar a1 = inter::NativeToFakeluaInt(&s, 1);
    CVar a2 = inter::NativeToFakeluaInt(&s, 2);
    CVar a3 = inter::NativeToFakeluaInt(&s, 3);
    CVar a4 = inter::NativeToFakeluaInt(&s, 4);
    CVar a5 = inter::NativeToFakeluaInt(&s, 5);
    CVar a6 = inter::NativeToFakeluaInt(&s, 6);
    CVar a7 = inter::NativeToFakeluaInt(&s, 7);
    CVar a8 = inter::NativeToFakeluaInt(&s, 8);

    EXPECT_THROW((void) FakeluaCallByName(&s, JIT_TCC, "missing", 0), std::exception);

    s.GetVM().RegisterFunction(VmFunction("nulladdr", 0, nullptr, {}));
    EXPECT_THROW((void) FakeluaCallByName(&s, JIT_TCC, "nulladdr", 0), std::exception);

    s.GetVM().RegisterFunction(VmFunction("fnv", 8, reinterpret_cast<void *>(&VmFnEchoFirst), {}));
    EXPECT_THROW((void) FakeluaCallByName(&s, JIT_TCC, "fnv", 9, a0, a1, a2, a3, a4, a5, a6, a7, a8), std::exception);
}

TEST(runtime, vm_helper_functions_throw_and_alloc) {
    State s;
    EXPECT_THROW(FakeluaThrowError(&s, "runtime_error"), std::exception);
    ASSERT_NE(FakeluaAllocTemp(&s, 32), nullptr);
}

TEST(runtime, heap_allocator_boundary_and_reset) {
    HeapAllocator alloc;
    ASSERT_NE(alloc.Alloc(1024 * 1024 - 64), nullptr);
    ASSERT_NE(alloc.Alloc(128), nullptr);
    ASSERT_GT(alloc.Size(), 1024U * 1024U);

    alloc.Reset();
    ASSERT_EQ(alloc.Size(), 0U);

    EXPECT_THROW((void) alloc.Alloc(1024 * 1024 + 1), std::exception);
}

TEST(runtime, generate_tmp_filename_creates_dir) {
    const auto tmpdir = std::filesystem::temp_directory_path() / "fakelua";
    std::error_code ec;
    std::filesystem::remove_all(tmpdir, ec);

    const std::string filename = GenerateTmpFilename("runtime_cov_", ".tmp");
    ASSERT_TRUE(std::filesystem::exists(tmpdir));
    ASSERT_NE(filename.find("runtime_cov_"), std::string::npos);
    ASSERT_TRUE(filename.ends_with(".tmp"));
}
