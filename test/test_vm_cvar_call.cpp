#include "compile/compiler.h"
#include "fakelua.h"
#include "state/const_string.h"
#include "state/state.h"
#include "var/var_multi.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include "var/var_type.h"
#include "gtest/gtest.h"

using namespace fakelua;

#define TEST_JIT_TYPE JIT_TCC

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
