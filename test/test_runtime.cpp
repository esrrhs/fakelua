#include "fakelua.h"
#include "jit/vm.h"
#include "state/heap.h"
#include "state/state.h"
#include "state/const_string.h"
#include "util/file_util.h"
#include "util/os.h"
#include "var/var.h"
#include "var/var_multi.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include "var/var_type.h"
#include "compile/compiler.h"
#include "gtest/gtest.h"
#include <filesystem>

using namespace fakelua;

namespace {

constexpr int kInvalidVarInterfaceType = 999;

[[nodiscard]] int InvalidVarInterfaceTypeValue() {
    volatile int value = kInvalidVarInterfaceType;
    return value;
}

struct InvalidVarImpl final : public VarInterface {
    [[nodiscard]] Type ViGetType() const override {
        // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
        return static_cast<Type>(InvalidVarInterfaceTypeValue());
    }

    void ViSetNil() override {
    }

    void ViSetBool(bool) override {
    }

    void ViSetInt(int64_t) override {
    }

    void ViSetFloat(double) override {
    }

    void ViSetString(const std::string_view &) override {
    }

    void ViSetTable(const std::vector<std::pair<VarInterface *, VarInterface *>> &) override {
    }

    [[nodiscard]] bool ViGetBool() const override {
        return false;
    }

    [[nodiscard]] int64_t ViGetInt() const override {
        return 0;
    }

    [[nodiscard]] double ViGetFloat() const override {
        return 0;
    }

    [[nodiscard]] std::string_view ViGetString() const override {
        return {};
    }

    [[nodiscard]] size_t ViGetTableSize() const override {
        return 0;
    }

    [[nodiscard]] std::pair<VarInterface *, VarInterface *> ViGetTableKv(int) const override {
        return {};
    }

    [[nodiscard]] std::string ViToString(int) const override {
        return "invalid";
    }
};

}// namespace

static CVar VmFn0() {
    Var ret;
    ret.SetInt(123);
    return ret;
}

static CVar VmFnEcho1(CVar a1) {
    return a1;
}

static CVar VmFnEcho2(CVar a1, CVar a2) {
    return a1;
}

static CVar VmFnEcho3(CVar a1, CVar a2, CVar a3) {
    return a1;
}

static CVar VmFnEcho4(CVar a1, CVar a2, CVar a3, CVar a4) {
    return a1;
}

static CVar VmFnEcho5(CVar a1, CVar a2, CVar a3, CVar a4, CVar a5) {
    return a1;
}

static CVar VmFnEcho6(CVar a1, CVar a2, CVar a3, CVar a4, CVar a5, CVar a6) {
    return a1;
}

static CVar VmFnEcho7(CVar a1, CVar a2, CVar a3, CVar a4, CVar a5, CVar a6, CVar a7) {
    return a1;
}

static CVar VmFnEcho8(CVar a1, CVar a2, CVar a3, CVar a4, CVar a5, CVar a6, CVar a7, CVar a8) {
    return a1;
}

#define TEST_JIT_TYPE JIT_TCC

static CVar callVmWithNArgs(State *s, const char *name, int n) {
    CVar args[9];
    for (int i = 0; i < 9; ++i) {
        args[i] = inter::NativeToFakeluaInt(s, 100 + i);
    }

    switch (n) {
        case 1:
            return FakeluaCallByName(s, TEST_JIT_TYPE, name, 1, args[0]);
        case 2:
            return FakeluaCallByName(s, TEST_JIT_TYPE, name, 2, args[0], args[1]);
        case 3:
            return FakeluaCallByName(s, TEST_JIT_TYPE, name, 3, args[0], args[1], args[2]);
        case 4:
            return FakeluaCallByName(s, TEST_JIT_TYPE, name, 4, args[0], args[1], args[2], args[3]);
        case 5:
            return FakeluaCallByName(s, TEST_JIT_TYPE, name, 5, args[0], args[1], args[2], args[3], args[4]);
        case 6:
            return FakeluaCallByName(s, TEST_JIT_TYPE, name, 6, args[0], args[1], args[2], args[3], args[4], args[5]);
        case 7:
            return FakeluaCallByName(s, TEST_JIT_TYPE, name, 7, args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
        case 8:
            return FakeluaCallByName(s, TEST_JIT_TYPE, name, 8, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
        default:
            break;
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

    s.GetVM().RegisterFunction(VmFunction("nulladdr", 0, JIT_TCC, nullptr, {}));
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
    constexpr size_t kOneMiB = 1024ULL * 1024ULL;
    // 先分配一个小的非对齐大小，使 current_block_offset_ 不是 alignment 的整数倍。
    ASSERT_NE(alloc.Alloc(5), nullptr);
    // 随后分配接近块大小的内存。旧代码会拒绝（因为 BLOCK_SIZE + padding > BLOCK_SIZE），
    // 修复后应该成功切到新块。
    ASSERT_NE(alloc.Alloc(kOneMiB), nullptr);
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

// ==========================================
// 合并自 test_vm_cvar_call.cpp 的测试用例与辅助定义
// ==========================================

// 定义 9 到 32 个参数的辅助参数类型宏
#define PARAMS_9 CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar, CVar
#define PARAMS_10 PARAMS_9, CVar
#define PARAMS_11 PARAMS_10, CVar
#define PARAMS_12 PARAMS_11, CVar
#define PARAMS_13 PARAMS_12, CVar
#define PARAMS_14 PARAMS_13, CVar
#define PARAMS_15 PARAMS_14, CVar
#define PARAMS_16 PARAMS_15, CVar
#define PARAMS_17 PARAMS_16, CVar
#define PARAMS_18 PARAMS_17, CVar
#define PARAMS_19 PARAMS_18, CVar
#define PARAMS_20 PARAMS_19, CVar
#define PARAMS_21 PARAMS_20, CVar
#define PARAMS_22 PARAMS_21, CVar
#define PARAMS_23 PARAMS_22, CVar
#define PARAMS_24 PARAMS_23, CVar
#define PARAMS_25 PARAMS_24, CVar
#define PARAMS_26 PARAMS_25, CVar
#define PARAMS_27 PARAMS_26, CVar
#define PARAMS_28 PARAMS_27, CVar
#define PARAMS_29 PARAMS_28, CVar
#define PARAMS_30 PARAMS_29, CVar
#define PARAMS_31 PARAMS_30, CVar
#define PARAMS_32 PARAMS_31, CVar

// 使用宏定义 VmFnEcho9 到 VmFnEcho32
#define DEFINE_ECHO_FN(N)                                                                                                                                                                              \
    static CVar VmFnEcho##N(PARAMS_##N) {                                                                                                                                                              \
        CVar ret;                                                                                                                                                                                      \
        ret.type_ = static_cast<int>(VarType::Int);                                                                                                                                                    \
        ret.data_.i = N;                                                                                                                                                                               \
        return ret;                                                                                                                                                                                    \
    }

DEFINE_ECHO_FN(9)
DEFINE_ECHO_FN(10)
DEFINE_ECHO_FN(11)
DEFINE_ECHO_FN(12)
DEFINE_ECHO_FN(13)
DEFINE_ECHO_FN(14)
DEFINE_ECHO_FN(15)
DEFINE_ECHO_FN(16)
DEFINE_ECHO_FN(17)
DEFINE_ECHO_FN(18)
DEFINE_ECHO_FN(19)
DEFINE_ECHO_FN(20)
DEFINE_ECHO_FN(21)
DEFINE_ECHO_FN(22)
DEFINE_ECHO_FN(23)
DEFINE_ECHO_FN(24)
DEFINE_ECHO_FN(25)
DEFINE_ECHO_FN(26)
DEFINE_ECHO_FN(27)
DEFINE_ECHO_FN(28)
DEFINE_ECHO_FN(29)
DEFINE_ECHO_FN(30)
DEFINE_ECHO_FN(31)
DEFINE_ECHO_FN(32)

#undef DEFINE_ECHO_FN

// 模拟空实现的 C++ 函数，用于异常测试
static CVar VmFnDummy() {
    return CVar{};
}

TEST(vm_cvar_call, arity_9_to_32) {
    State s;
    CVar a;
    a.type_ = static_cast<int>(VarType::Int);
    a.data_.i = 999;

#define ARG_LIST_9 a, a, a, a, a, a, a, a, a
#define ARG_LIST_10 ARG_LIST_9, a
#define ARG_LIST_11 ARG_LIST_10, a
#define ARG_LIST_12 ARG_LIST_11, a
#define ARG_LIST_13 ARG_LIST_12, a
#define ARG_LIST_14 ARG_LIST_13, a
#define ARG_LIST_15 ARG_LIST_14, a
#define ARG_LIST_16 ARG_LIST_15, a
#define ARG_LIST_17 ARG_LIST_16, a
#define ARG_LIST_18 ARG_LIST_17, a
#define ARG_LIST_19 ARG_LIST_18, a
#define ARG_LIST_20 ARG_LIST_19, a
#define ARG_LIST_21 ARG_LIST_20, a
#define ARG_LIST_22 ARG_LIST_21, a
#define ARG_LIST_23 ARG_LIST_22, a
#define ARG_LIST_24 ARG_LIST_23, a
#define ARG_LIST_25 ARG_LIST_24, a
#define ARG_LIST_26 ARG_LIST_25, a
#define ARG_LIST_27 ARG_LIST_26, a
#define ARG_LIST_28 ARG_LIST_27, a
#define ARG_LIST_29 ARG_LIST_28, a
#define ARG_LIST_30 ARG_LIST_29, a
#define ARG_LIST_31 ARG_LIST_30, a
#define ARG_LIST_32 ARG_LIST_31, a

#define TEST_VM_CALL(N)                                                                                                                                                                                \
    s.GetVM().RegisterFunction(VmFunction("fnv" #N, N, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnEcho##N), {}));                                                                                    \
    {                                                                                                                                                                                                  \
        CVar ret = FakeluaCallByName(&s, TEST_JIT_TYPE, "fnv" #N, N, ARG_LIST_##N);                                                                                                                    \
        ASSERT_EQ(ret.type_, static_cast<int>(VarType::Int));                                                                                                                                          \
        ASSERT_EQ(ret.data_.i, N);                                                                                                                                                                     \
    }

    TEST_VM_CALL(9)
    TEST_VM_CALL(10)
    TEST_VM_CALL(11)
    TEST_VM_CALL(12)
    TEST_VM_CALL(13)
    TEST_VM_CALL(14)
    TEST_VM_CALL(15)
    TEST_VM_CALL(16)
    TEST_VM_CALL(17)
    TEST_VM_CALL(18)
    TEST_VM_CALL(19)
    TEST_VM_CALL(20)
    TEST_VM_CALL(21)
    TEST_VM_CALL(22)
    TEST_VM_CALL(23)
    TEST_VM_CALL(24)
    TEST_VM_CALL(25)
    TEST_VM_CALL(26)
    TEST_VM_CALL(27)
    TEST_VM_CALL(28)
    TEST_VM_CALL(29)
    TEST_VM_CALL(30)
    TEST_VM_CALL(31)
    TEST_VM_CALL(32)

#undef TEST_VM_CALL
#undef ARG_LIST_32
#undef ARG_LIST_31
#undef ARG_LIST_30
#undef ARG_LIST_29
#undef ARG_LIST_28
#undef ARG_LIST_27
#undef ARG_LIST_26
#undef ARG_LIST_25
#undef ARG_LIST_24
#undef ARG_LIST_23
#undef ARG_LIST_22
#undef ARG_LIST_21
#undef ARG_LIST_20
#undef ARG_LIST_19
#undef ARG_LIST_18
#undef ARG_LIST_17
#undef ARG_LIST_16
#undef ARG_LIST_15
#undef ARG_LIST_14
#undef ARG_LIST_13
#undef ARG_LIST_12
#undef ARG_LIST_11
#undef ARG_LIST_10
#undef ARG_LIST_9
}

TEST(vm_cvar_call, parameter_limit_exceptions) {
    State s;

    // 1. 测试 expected_arg_count > kMaxFunctionInputParams (32)
    s.GetVM().RegisterFunction(VmFunction("fn_too_many", 33, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnDummy), {}));
    EXPECT_THROW(FakeluaCallByName(&s, TEST_JIT_TYPE, "fn_too_many", 0), std::exception);

    // 2. 测试 arg_num > kMaxFunctionInputParams (32)
    s.GetVM().RegisterFunction(VmFunction("fn_ok", 2, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnDummy), {}));
    EXPECT_THROW(FakeluaCallByName(&s, TEST_JIT_TYPE, "fn_ok", 33), std::exception);
}

// 供多返回值测试使用的 C++ 辅助函数
static CVar VmFnEchoArg4(CVar a1, CVar a2, CVar a3, CVar a4) {
    return a4;
}

static CVar VmFnEchoArg2(CVar a1, CVar a2) {
    return a2;
}

static CVar VmFnEchoArg3(CVar a1, CVar a2, CVar a3) {
    return a3;
}

TEST(vm_cvar_call, multi_return_expansion) {
    State s;

    // 注册测试函数
    s.GetVM().RegisterFunction(VmFunction("echo4", 4, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnEchoArg4), {}));
    s.GetVM().RegisterFunction(VmFunction("echo2", 2, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnEchoArg2), {}));
    s.GetVM().RegisterFunction(VmFunction("echo3", 3, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnEchoArg3), {}));

    // 构造 CVar 常规参数
    CVar int1 = inter::NativeToFakeluaInt(&s, 1);
    CVar int20 = inter::NativeToFakeluaInt(&s, 20);
    CVar int30 = inter::NativeToFakeluaInt(&s, 30);
    CVar int40 = inter::NativeToFakeluaInt(&s, 40);
    CVar int50 = inter::NativeToFakeluaInt(&s, 50);
    CVar int100 = inter::NativeToFakeluaInt(&s, 100);

    // 构造含有 2 个元素的 Multi 参数
    VarMulti *m_two_a = VarMulti::AllocTemp(&s, 2);
    m_two_a->GetVars()[0] = int20;
    m_two_a->GetVars()[1] = int30;

    CVar multi_two_a;
    multi_two_a.type_ = static_cast<int>(VarType::Multi);
    multi_two_a.data_.m = m_two_a;

    // 构造另一个含有 2 个元素的 Multi 参数
    VarMulti *m_two_b = VarMulti::AllocTemp(&s, 2);
    m_two_b->GetVars()[0] = int40;
    m_two_b->GetVars()[1] = int50;

    CVar multi_two_b;
    multi_two_b.type_ = static_cast<int>(VarType::Multi);
    multi_two_b.data_.m = m_two_b;

    // 构造空 Multi 参数
    VarMulti *m_empty = VarMulti::AllocTemp(&s, 0);
    CVar multi_empty;
    multi_empty.type_ = static_cast<int>(VarType::Multi);
    multi_empty.data_.m = m_empty;

    // 构造 1 个元素的 Multi 参数
    VarMulti *m_one = VarMulti::AllocTemp(&s, 1);
    m_one->GetVars()[0] = int20;
    CVar multi_one;
    multi_one.type_ = static_cast<int>(VarType::Multi);
    multi_one.data_.m = m_one;

    // 1. 分支 A: has_any_multi 为 true，但最后一个参数不是 Multi 并且 arg_num != expected_arg_count -> 抛出异常
    EXPECT_THROW(FakeluaCallByName(&s, TEST_JIT_TYPE, "echo2", 3, multi_two_a, int1, int20), std::exception);

    // 2. 正常的多参数解包覆盖：
    // expected_arg_count = 4, 我们传 3 个参数：int1, multi_two_a (非末尾，只取 vars[0]), multi_two_b (末尾，全部展开)
    // 预期实际参数：int1, int20, int40, int50
    // echo4 会返回第 4 个参数，即 int50
    CVar r1 = FakeluaCallByName(&s, TEST_JIT_TYPE, "echo4", 3, int1, multi_two_a, multi_two_b);
    ASSERT_EQ(r1.type_, static_cast<int>(VarType::Int));
    ASSERT_EQ(r1.data_.i, 50);

    // 3. 非末尾空 Multi 的覆盖：
    // expected_arg_count = 2, 传 2 个参数：multi_empty (非末尾，填充为 Nil), int100
    // 预期实际参数：Nil, int100
    // echo2 返回第 2 个参数，即 int100
    CVar r2 = FakeluaCallByName(&s, TEST_JIT_TYPE, "echo2", 2, multi_empty, int100);
    ASSERT_EQ(r2.type_, static_cast<int>(VarType::Int));
    ASSERT_EQ(r2.data_.i, 100);

    // 4. 填充 Nil padding 的覆盖：
    // expected_arg_count = 3, 传 2 个参数：int100, multi_one (末尾展开为 1 个 int20)，剩余 1 个由 while 填充为 Nil
    // 预期实际参数：int100, int20, Nil
    // echo3 返回第 3 个参数，即 Nil
    CVar r3 = FakeluaCallByName(&s, TEST_JIT_TYPE, "echo3", 2, int100, multi_one);
    ASSERT_EQ(r3.type_, static_cast<int>(VarType::Nil));

    // 5. 超出最大输入限制解包时的保护分支
    // 构造一个长度极大的 Multi
    VarMulti *m_huge = VarMulti::AllocTemp(&s, 20);
    for (int i = 0; i < 20; ++i) {
        m_huge->GetVars()[i] = int1;
    }
    CVar multi_huge;
    multi_huge.type_ = static_cast<int>(VarType::Multi);
    multi_huge.data_.m = m_huge;

    // 注册 32 个参数的函数并调用，传入 32 个参数，最后一个是 multi_huge，
    // 它会尝试解包 20 个，但解包到 32 限制后应该直接安全中止，防止溢出。
    s.GetVM().RegisterFunction(VmFunction("fnv32", 32, TEST_JIT_TYPE, reinterpret_cast<void *>(&VmFnEcho32), {}));

    // 传入 31 个 int1 参数，最后 1 个是 multi_huge。合计参数应该被截断到 32 限制
    CVar r4 = FakeluaCallByName(&s, TEST_JIT_TYPE, "fnv32", 32, int1, int1, int1, int1, int1, int1, int1, int1, int1, int1, int1, int1, int1, int1, int1, int1, int1, int1, int1, int1, int1, int1,
                                int1, int1, int1, int1, int1, int1, int1, int1, int1, multi_huge);
    ASSERT_EQ(r4.type_, static_cast<int>(VarType::Int));
    ASSERT_EQ(r4.data_.i, 32);
}
