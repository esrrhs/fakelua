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

namespace {

constexpr int kInvalidVarInterfaceType = 999;

struct InvalidVarImpl final : public VarInterface {
    [[nodiscard]] Type ViGetType() const override { return static_cast<Type>(kInvalidVarInterfaceType); }
    void ViSetNil() override {}
    void ViSetBool(bool) override {}
    void ViSetInt(int64_t) override {}
    void ViSetFloat(double) override {}
    void ViSetString(const std::string_view &) override {}
    void ViSetTable(const std::vector<std::pair<VarInterface *, VarInterface *>> &) override {}
    [[nodiscard]] bool ViGetBool() const override { return false; }
    [[nodiscard]] int64_t ViGetInt() const override { return 0; }
    [[nodiscard]] double ViGetFloat() const override { return 0; }
    [[nodiscard]] std::string_view ViGetString() const override { return {}; }
    [[nodiscard]] size_t ViGetTableSize() const override { return 0; }
    [[nodiscard]] std::pair<VarInterface *, VarInterface *> ViGetTableKv(int) const override { return {}; }
    [[nodiscard]] std::string ViToString(int) const override { return "invalid"; }
};

} // namespace

static CVar VmFn0() {
    Var ret;
    ret.SetInt(123);
    return ret;
}

static CVar VmFnEcho1(CVar a1) { return a1; }
static CVar VmFnEcho2(CVar a1, CVar a2) { return a1; }
static CVar VmFnEcho3(CVar a1, CVar a2, CVar a3) { return a1; }
static CVar VmFnEcho4(CVar a1, CVar a2, CVar a3, CVar a4) { return a1; }
static CVar VmFnEcho5(CVar a1, CVar a2, CVar a3, CVar a4, CVar a5) { return a1; }
static CVar VmFnEcho6(CVar a1, CVar a2, CVar a3, CVar a4, CVar a5, CVar a6) { return a1; }
static CVar VmFnEcho7(CVar a1, CVar a2, CVar a3, CVar a4, CVar a5, CVar a6, CVar a7) { return a1; }
static CVar VmFnEcho8(CVar a1, CVar a2, CVar a3, CVar a4, CVar a5, CVar a6, CVar a7, CVar a8) { return a1; }

// On macOS, use GCC JIT since TCC JIT has arm64 codegen issues
#ifdef __APPLE__
#define TEST_JIT_TYPE JIT_GCC
#else
#define TEST_JIT_TYPE JIT_TCC
#endif

static CVar callVmWithNArgs(State *s, const char *name, int n) {
    CVar args[9];
    for (int i = 0; i < 9; ++i) {
        args[i] = inter::NativeToFakeluaInt(s, 100 + i);
    }

    switch (n) {
        case 1: return FakeluaCallByName(s, TEST_JIT_TYPE, name, 1, args[0]);
        case 2: return FakeluaCallByName(s, TEST_JIT_TYPE, name, 2, args[0], args[1]);
        case 3: return FakeluaCallByName(s, TEST_JIT_TYPE, name, 3, args[0], args[1], args[2]);
        case 4: return FakeluaCallByName(s, TEST_JIT_TYPE, name, 4, args[0], args[1], args[2], args[3]);
        case 5: return FakeluaCallByName(s, TEST_JIT_TYPE, name, 5, args[0], args[1], args[2], args[3], args[4]);
        case 6: return FakeluaCallByName(s, TEST_JIT_TYPE, name, 6, args[0], args[1], args[2], args[3], args[4], args[5]);
        case 7: return FakeluaCallByName(s, TEST_JIT_TYPE, name, 7, args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
        case 8: return FakeluaCallByName(s, TEST_JIT_TYPE, name, 8, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
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
    s.GetVM().RegisterFunction(VmFunction("fn0", 0, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFn0), {}));
    // 为每个 arity 注册一个匹配 arg_count 的函数（Bug M：FakeluaCallByName 严格校验）。
    s.GetVM().RegisterFunction(VmFunction("fnv1", 1, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnEcho1), {}));
    s.GetVM().RegisterFunction(VmFunction("fnv2", 2, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnEcho2), {}));
    s.GetVM().RegisterFunction(VmFunction("fnv3", 3, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnEcho3), {}));
    s.GetVM().RegisterFunction(VmFunction("fnv4", 4, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnEcho4), {}));
    s.GetVM().RegisterFunction(VmFunction("fnv5", 5, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnEcho5), {}));
    s.GetVM().RegisterFunction(VmFunction("fnv6", 6, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnEcho6), {}));
    s.GetVM().RegisterFunction(VmFunction("fnv7", 7, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnEcho7), {}));
    s.GetVM().RegisterFunction(VmFunction("fnv8", 8, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnEcho8), {}));

    const CVar r0 = FakeluaCallByName(&s, TEST_JIT_TYPE, "fn0", 0);
    ASSERT_EQ(inter::FakeluaToNativeInt(&s, r0), 123);

    const char *names[] = {"fnv1", "fnv2", "fnv3", "fnv4", "fnv5", "fnv6", "fnv7", "fnv8"};
    for (int n = 1; n <= 8; ++n) {
        const CVar ret = callVmWithNArgs(&s, names[n - 1], n);
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

    s.GetVM().RegisterFunction(VmFunction("fnv", 8, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnEcho8), {}));
    EXPECT_THROW((void) FakeluaCallByName(&s, JIT_TCC, "fnv", 9, a0, a1, a2, a3, a4, a5, a6, a7, a8), std::exception);

    // Bug M: 参数个数与函数签名不匹配时必须抛异常（否则会读取未初始化栈内存）。
    // 这里 fnv 要求 8 个参数，分别传 3 个 / 0 个都应抛异常。
    EXPECT_THROW((void) FakeluaCallByName(&s, JIT_TCC, "fnv", 3, a0, a1, a2), std::exception);
    EXPECT_THROW((void) FakeluaCallByName(&s, JIT_TCC, "fnv", 0), std::exception);
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

// Bug H 回归测试：当当前块已经被 padding 偏移时，一个仅略小于块大小的分配
// 不应被错误地拒绝（旧代码用 size + padding > BLOCK_SIZE 过于保守）。
TEST(runtime, heap_allocator_large_alloc_after_small) {
    HeapAllocator alloc;
    // 先分配一个小的非对齐大小，使 current_block_offset_ 不是 alignment 的整数倍。
    ASSERT_NE(alloc.Alloc(5), nullptr);
    // 随后分配接近块大小的内存。旧代码会拒绝（因为 BLOCK_SIZE + padding > BLOCK_SIZE），
    // 修复后应该成功切到新块。
    ASSERT_NE(alloc.Alloc(1024 * 1024), nullptr);
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

TEST(runtime, inter_native_scalar_roundtrip) {
    State s;

    ASSERT_TRUE(inter::FakeluaToNativeBool(&s, inter::NativeToFakeluaBool(&s, true)));
    ASSERT_EQ(inter::FakeluaToNativeChar(&s, inter::NativeToFakeluaChar(&s, -12)), -12);
    ASSERT_EQ(inter::FakeluaToNativeUchar(&s, inter::NativeToFakeluaUchar(&s, 250)), 250);
    ASSERT_EQ(inter::FakeluaToNativeShort(&s, inter::NativeToFakeluaShort(&s, -1234)), -1234);
    ASSERT_EQ(inter::FakeluaToNativeUshort(&s, inter::NativeToFakeluaUshort(&s, 54321)), 54321);
    ASSERT_EQ(inter::FakeluaToNativeInt(&s, inter::NativeToFakeluaInt(&s, -1234567)), -1234567);
    ASSERT_EQ(inter::FakeluaToNativeUint(&s, inter::NativeToFakeluaUint(&s, 3456789012u)), 3456789012u);
    ASSERT_EQ(inter::FakeluaToNativeLong(&s, inter::NativeToFakeluaLong(&s, -99887766L)), -99887766L);
    ASSERT_EQ(inter::FakeluaToNativeUlong(&s, inter::NativeToFakeluaUlong(&s, 99887766UL)), 99887766UL);
    ASSERT_EQ(inter::FakeluaToNativeLonglong(&s, inter::NativeToFakeluaLonglong(&s, -1234567890123LL)), -1234567890123LL);
    ASSERT_EQ(inter::FakeluaToNativeUlonglong(&s, inter::NativeToFakeluaUlonglong(&s, 1234567890123ULL)), 1234567890123ULL);
    ASSERT_FLOAT_EQ(inter::FakeluaToNativeFloat(&s, inter::NativeToFakeluaFloat(&s, 1.25f)), 1.25f);
    ASSERT_DOUBLE_EQ(inter::FakeluaToNativeDouble(&s, inter::NativeToFakeluaDouble(&s, 2.75)), 2.75);
    ASSERT_FLOAT_EQ(inter::FakeluaToNativeFloat(&s, inter::NativeToFakeluaInt(&s, 9)), 9.0f);
    ASSERT_DOUBLE_EQ(inter::FakeluaToNativeDouble(&s, inter::NativeToFakeluaInt(&s, 11)), 11.0);

    char mutable_str[] = "mutable";
    ASSERT_EQ(inter::FakeluaToNativeString(&s, inter::NativeToFakeluaCstr(&s, "hello")), "hello");
    ASSERT_EQ(inter::FakeluaToNativeString(&s, inter::NativeToFakeluaStr(&s, mutable_str)), "mutable");
    ASSERT_EQ(inter::FakeluaToNativeString(&s, inter::NativeToFakeluaString(&s, std::string("world"))), "world");
    const std::string sv = "view";
    ASSERT_EQ(inter::FakeluaToNativeStringView(&s, inter::NativeToFakeluaStringView(&s, std::string_view(sv))), "view");
}

TEST(runtime, inter_object_conversion_and_errors) {
    State s;
    std::vector<VarInterface *> allocated;
    SetVarInterfaceNewFunc(&s, [&]() {
        auto *p = new SimpleVarImpl();
        allocated.push_back(p);
        return p;
    });

    auto *vi_nil = inter::FakeluaToNativeObj(&s, inter::NativeToFakeluaNil(&s));
    ASSERT_EQ(vi_nil->ViGetType(), VarInterface::Type::NIL);

    auto *vi_bool = inter::FakeluaToNativeObj(&s, inter::NativeToFakeluaBool(&s, false));
    ASSERT_EQ(vi_bool->ViGetType(), VarInterface::Type::BOOL);
    ASSERT_FALSE(vi_bool->ViGetBool());

    auto *vi_float = inter::FakeluaToNativeObj(&s, inter::NativeToFakeluaDouble(&s, 3.5));
    ASSERT_EQ(vi_float->ViGetType(), VarInterface::Type::FLOAT);
    ASSERT_DOUBLE_EQ(vi_float->ViGetFloat(), 3.5);

    Var sid;
    sid.SetConstString(&s, "const_sid");
    auto *vi_sid = inter::FakeluaToNativeObj(&s, sid);
    ASSERT_EQ(vi_sid->ViGetType(), VarInterface::Type::STRING);
    ASSERT_EQ(vi_sid->ViGetString(), "const_sid");
    ASSERT_EQ(inter::FakeluaToNativeString(&s, sid), "const_sid");

    Var table_quick;
    table_quick.SetTable(&s);
    table_quick.TableSet(&s, Var(int64_t(1)), Var(int64_t(11)), true);
    table_quick.TableSet(&s, Var(int64_t(2)), Var(int64_t(22)), true);
    auto *vi_quick = inter::FakeluaToNativeObj(&s, table_quick);
    ASSERT_EQ(vi_quick->ViGetType(), VarInterface::Type::TABLE);
    ASSERT_EQ(vi_quick->ViGetTableSize(), 2U);

    Var table_hash;
    table_hash.SetTable(&s);
    Var k;
    k.SetTempString(&s, "k");
    table_hash.TableSet(&s, k, Var(int64_t(7)), true);
    auto *vi_hash = inter::FakeluaToNativeObj(&s, table_hash);
    ASSERT_EQ(vi_hash->ViGetType(), VarInterface::Type::TABLE);
    ASSERT_EQ(vi_hash->ViGetTableSize(), 1U);
    ASSERT_EQ(vi_hash->ViGetTableKv(0).first->ViGetType(), VarInterface::Type::STRING);

    SimpleVarImpl nested_k;
    nested_k.ViSetString("nk");
    SimpleVarImpl nested_v;
    nested_v.ViSetInt(42);
    SimpleVarImpl scalar_nil;
    scalar_nil.ViSetNil();
    auto *roundtrip_nil = inter::FakeluaToNativeObj(&s, inter::NativeToFakeluaObj(&s, &scalar_nil));
    ASSERT_EQ(roundtrip_nil->ViGetType(), VarInterface::Type::NIL);
    SimpleVarImpl scalar_bool;
    scalar_bool.ViSetBool(true);
    auto *roundtrip_bool = inter::FakeluaToNativeObj(&s, inter::NativeToFakeluaObj(&s, &scalar_bool));
    ASSERT_EQ(roundtrip_bool->ViGetType(), VarInterface::Type::BOOL);
    ASSERT_TRUE(roundtrip_bool->ViGetBool());
    SimpleVarImpl scalar_float;
    scalar_float.ViSetFloat(6.25);
    auto *roundtrip_float = inter::FakeluaToNativeObj(&s, inter::NativeToFakeluaObj(&s, &scalar_float));
    ASSERT_EQ(roundtrip_float->ViGetType(), VarInterface::Type::FLOAT);
    ASSERT_DOUBLE_EQ(roundtrip_float->ViGetFloat(), 6.25);
    SimpleVarImpl table_key;
    table_key.ViSetString("outer");
    SimpleVarImpl table_val;
    table_val.ViSetTable({{&nested_k, &nested_v}});
    SimpleVarImpl root;
    root.ViSetTable({{&table_key, &table_val}});
    CVar root_var = inter::NativeToFakeluaObj(&s, &root);
    auto *roundtrip = inter::FakeluaToNativeObj(&s, root_var);
    ASSERT_EQ(roundtrip->ViGetType(), VarInterface::Type::TABLE);
    ASSERT_EQ(roundtrip->ViGetTableSize(), 1U);
    ASSERT_EQ(roundtrip->ViGetTableKv(0).second->ViGetType(), VarInterface::Type::TABLE);

    InvalidVarImpl invalid;
    EXPECT_THROW((void) inter::NativeToFakeluaObj(&s, &invalid), std::exception);

    for (auto *p: allocated) {
        delete p;
    }
}

// 验证 StateConfig 中 TCC include_paths 能被正确传递到 State
TEST(runtime, state_config_is_stored) {
    StateConfig cfg;
    cfg.tcc_config.include_paths = {"/tmp/custom_include"};
    cfg.gcc_config.libraries = {"customlib"};
    FakeluaStateGuard sg(cfg);
    auto s = sg.GetState();

    ASSERT_EQ(s->GetStateConfig().tcc_config.include_paths.size(), 1u);
    ASSERT_EQ(s->GetStateConfig().tcc_config.include_paths[0], "/tmp/custom_include");
    ASSERT_EQ(s->GetStateConfig().gcc_config.libraries.size(), 1u);
    ASSERT_EQ(s->GetStateConfig().gcc_config.libraries[0], "customlib");
}
