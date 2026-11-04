#include "fakelua.h"
#include "state/state.h"
#include "var/var.h"
#include "var/var_closure.h"
#include "var/var_string.h"
#include "var/var_multi.h"
#include "native/table/native_table.h"
#include "gtest/gtest.h"
#include "interp/interpreter.h"
#include "interp/runtime.h"
#include "var/var_type.h"
#include <limits>
#include <string>
#include <vector>

using namespace fakelua;

namespace {

[[nodiscard]] VarType MakeInvalidVarTypeForTest() {
    volatile int invalid = 999;
    return static_cast<VarType>(invalid);
}

}// namespace

TEST(var, construct) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    ASSERT_EQ(v.Type(), VarType::Nil);

    Var v1;
    ASSERT_EQ(v1.Type(), VarType::Nil);

    Var v2(true);
    ASSERT_EQ(v2.Type(), VarType::Bool);

    Var v3(static_cast<int64_t>(1));
    ASSERT_EQ(v3.Type(), VarType::Int);

    Var v4(1.1);
    ASSERT_EQ(v4.Type(), VarType::Float);

    Var v4_1(1.00);
    ASSERT_EQ(v4_1.Type(), VarType::Float);

    Var v5;
    v5.SetTempString(s, "hello");
    ASSERT_EQ(v5.Type(), VarType::String);

    Var v6;
    v6.SetTempString(s, std::string("hello"));
    ASSERT_EQ(v6.Type(), VarType::String);

    Var v7;
    v7.SetTempString(s, std::move(std::string("hello")));
    ASSERT_EQ(v7.Type(), VarType::String);

    std::string str("hello");
    Var v8;
    v8.SetTempString(s, str);
    ASSERT_EQ(v8.Type(), VarType::String);

    Var v9;
    v9.SetTempString(s, str);
    ASSERT_EQ(v9.Type(), VarType::String);

    std::string_view StrView("hello");
    Var v10;
    v10.SetTempString(s, StrView);
    ASSERT_EQ(v10.Type(), VarType::String);

    Var v11;
    v11.SetTempString(s, StrView);
    ASSERT_EQ(v11.Type(), VarType::String);
}

TEST(var, set_get) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    v.SetNil();
    ASSERT_EQ(v.Type(), VarType::Nil);

    v.SetBool(true);
    ASSERT_EQ(v.Type(), VarType::Bool);
    ASSERT_EQ(v.GetBool(), true);

    v.SetInt((int64_t) 1);
    ASSERT_EQ(v.Type(), VarType::Int);
    ASSERT_EQ(v.GetInt(), 1);

    v.SetFloat(1.1);
    ASSERT_EQ(v.Type(), VarType::Float);
    ASSERT_EQ(v.GetFloat(), 1.1);

    v.SetFloat(1.0);
    ASSERT_EQ(v.Type(), VarType::Float);
    ASSERT_EQ(v.GetFloat(), 1.0);

    v.SetTempString(s, "hello");
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), "hello");

    v.SetTempString(s, std::string("hello"));
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), "hello");

    v.SetTempString(s, std::move(std::string("hello")));
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), "hello");
}

TEST(var, ToString) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    v.SetNil();
    ASSERT_EQ(v.ToString(), "nil");

    v.SetBool(true);
    ASSERT_EQ(v.ToString(), "true");

    v.SetInt((int64_t) 12345);
    ASSERT_EQ(v.ToString(), "12345");

    v.SetFloat(12345.0);
    ASSERT_EQ(v.ToString(), "12345.0");

    v.SetFloat(12345.1);
    ASSERT_EQ(v.ToString(), "12345.1");

    v.SetFloat(2.1245e-10);
    ASSERT_EQ(v.ToString(), "2.1245e-10");

    v.SetTempString(s, "hello");
    ASSERT_EQ(v.ToString(), "\"hello\"");
}

TEST(var, SetTempString) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    const std::string str("hello");
    v.SetTempString(s, str);
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), str);

    v.SetTempString(s, std::move(std::string("hello")));
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), str);

    v.SetTempString(s, "hello");
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), str);

    v.SetTempString(s, std::string_view("hello"));
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), str);
}

TEST(var, test_true) {
    Var v_true(true);
    Var v_false(false);
    Var v_nil;

    ASSERT_TRUE(v_true.TestTrue());
    ASSERT_FALSE(v_false.TestTrue());
    ASSERT_FALSE(v_nil.TestTrue());
    ASSERT_TRUE(Var((int64_t) 10LL).TestTrue());
}

TEST(var, get_string_string_type) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    v.SetTempString(s, "test string");
    ASSERT_EQ(v.Type(), VarType::String);
    ASSERT_EQ(v.GetString()->Str(), "test string");
}

TEST(var, get_string_stringid_type) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    v.SetConstString(s, "const string");
    ASSERT_EQ(v.Type(), VarType::StringId);
    ASSERT_EQ(v.GetString()->Str(), "const string");
}

TEST(var, tostring_stringid) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    v.SetConstString(s, "const");
    ASSERT_EQ(v.ToString(), "\"const\"");

    // Test without quote
    ASSERT_EQ(v.ToString(false), "const");
}

TEST(var, hash_string_types) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTempString(s, "hello");
    size_t hash1 = v1.Hash();
    ASSERT_NE(hash1, 0);

    Var v2;
    v2.SetConstString(s, "hello");
    size_t hash2 = v2.Hash();
    ASSERT_NE(hash2, 0);

    // Same string content should have same hash
    ASSERT_EQ(hash1, hash2);
}

TEST(var, hash_int_float) {
    Var v1(static_cast<int64_t>(12345));
    size_t hash1 = v1.Hash();
    ASSERT_NE(hash1, 0);

    Var v2(12345.0);
    size_t hash2 = v2.Hash();
    ASSERT_NE(hash2, 0);
}

TEST(var, equal_string_stringid) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTempString(s, "hello");

    Var v2;
    v2.SetConstString(s, "hello");

    // String == StringId should compare content
    ASSERT_TRUE(v1.Equal(v2));
    ASSERT_TRUE(v2.Equal(v1));

    Var v3;
    v3.SetConstString(s, "world");
    ASSERT_FALSE(v1.Equal(v3));
}

TEST(var, equal_string) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetTempString(s, "hello");

    Var v2;
    v2.SetTempString(s, "hello");

    // Same string content, different pointers
    ASSERT_TRUE(v1.Equal(v2));

    Var v3;
    v3.SetTempString(s, "world");
    ASSERT_FALSE(v1.Equal(v3));
}

TEST(var, equal_stringid) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v1;
    v1.SetConstString(s, "hello");

    Var v2;
    v2.SetConstString(s, "hello");

    // Same StringId should be equal
    ASSERT_TRUE(v1.Equal(v2));

    Var v3;
    v3.SetConstString(s, "world");
    ASSERT_FALSE(v1.Equal(v3));
}

TEST(var, equal_float_nan) {
    Var v1(std::numeric_limits<double>::quiet_NaN());
    Var v2(std::numeric_limits<double>::quiet_NaN());

    // NaN != NaN in IEEE 754
    ASSERT_FALSE(v1.Equal(v2));
}

TEST(var, hash_nil) {
    Var v;
    size_t hash = v.Hash();
    ASSERT_EQ(hash, 0);
}

TEST(var, hash_default_branch) {
    // Test that Hash() handles all types without crashing.
    // The invalid type falls through to the default case (return 0).
    Var v;
    // Force an invalid type on the Var to exercise the default branch.
    {
        volatile int invalid = 999;
        v.type_ = invalid;
    }
    size_t hash = v.Hash();
    ASSERT_EQ(hash, 0);
}

TEST(var, equal_default_branch) {
    // Test that Equal() with mismatched types returns false.
    Var v1(static_cast<int64_t>(42));
    Var v2;
    {
        volatile int invalid = 999;
        v2.type_ = invalid;
    }
    ASSERT_FALSE(v1.Equal(v2));
    ASSERT_FALSE(v2.Equal(v1));
}

TEST(var, VarTypeToString_all_types) {
    // Verify all valid VarType values produce non-empty strings.
    for (int t = static_cast<int>(VarType::Min); t <= static_cast<int>(VarType::Max); ++t) {
        std::string s = VarTypeToString(static_cast<VarType>(t));
        ASSERT_FALSE(s.empty()) << "VarTypeToString returned empty for type " << t;
    }
}

TEST(var, equal_int_float_cross_type) {
    // Int and Float with the same mathematical value are equal (Lua semantics).
    Var v1(static_cast<int64_t>(42));
    Var v2(42.0);
    ASSERT_TRUE(v1.Equal(v2));
    ASSERT_TRUE(v2.Equal(v1));

    Var v3(static_cast<int64_t>(42));
    Var v4(42.5);
    ASSERT_FALSE(v1.Equal(v4));
    ASSERT_FALSE(v3.Equal(v4));
}

TEST(var, test_true_float) {
    // Non-zero float is truthy.
    ASSERT_TRUE(Var(1.5).TestTrue());
    ASSERT_TRUE(Var(-0.5).TestTrue());
    // Zero float is also truthy in Lua (only nil and false are falsy).
    ASSERT_TRUE(Var(0.0).TestTrue());
}

TEST(var, test_true_string) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    Var v;
    v.SetTempString(s, "hello");
    ASSERT_TRUE(v.TestTrue());

    Var empty_str;
    empty_str.SetTempString(s, "");
    ASSERT_TRUE(empty_str.TestTrue());// empty string is still truthy in Lua
}


TEST(var, tostring_hash_equal_table_multi_closure) {
    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    CVar tbl_cv = table::TableHelper::CreateTable(s);
    Var &tbl = AsVar(tbl_cv);
    ASSERT_EQ(tbl.Type(), VarType::Table);
    ASSERT_TRUE(tbl.TestTrue());
    const std::string tbl_str = tbl.ToString();
    ASSERT_NE(tbl_str.find("table("), std::string::npos);
    ASSERT_NE(tbl.Hash(), 0u);
    ASSERT_TRUE(tbl.Equal(tbl));

    CVar tbl2_cv = table::TableHelper::CreateTable(s);
    Var &tbl2 = AsVar(tbl2_cv);
    ASSERT_FALSE(tbl.Equal(tbl2));

    CVar multi_cv = inter::AllocMultiCVar(s, 2);
    CVar one{};
    one.type_ = static_cast<int>(VarType::Int);
    one.data_.i = 1;
    CVar two{};
    two.type_ = static_cast<int>(VarType::Int);
    two.data_.i = 2;
    inter::SetMultiCVarElement(multi_cv, 0, one);
    inter::SetMultiCVarElement(multi_cv, 1, two);
    Var &multi = AsVar(multi_cv);
    ASSERT_EQ(multi.Type(), VarType::Multi);
    ASSERT_TRUE(multi.TestTrue());
    const std::string multi_str = multi.ToString();
    ASSERT_NE(multi_str.find("multi("), std::string::npos);
    ASSERT_NE(multi_str.find("1"), std::string::npos);
    ASSERT_NE(multi.Hash(), 0u);

    CVar multi2_cv = inter::AllocMultiCVar(s, 2);
    inter::SetMultiCVarElement(multi2_cv, 0, one);
    inter::SetMultiCVarElement(multi2_cv, 1, two);
    ASSERT_TRUE(multi.Equal(AsVar(multi2_cv)));
    CVar ninetynine{};
    ninetynine.type_ = static_cast<int>(VarType::Int);
    ninetynine.data_.i = 99;
    inter::SetMultiCVarElement(multi2_cv, 1, ninetynine);
    ASSERT_FALSE(multi.Equal(AsVar(multi2_cv)));

    CVar multi3_cv = inter::AllocMultiCVar(s, 1);
    inter::SetMultiCVarElement(multi3_cv, 0, one);
    ASSERT_FALSE(multi.Equal(AsVar(multi3_cv)));

    Var cl;
    cl.type_ = static_cast<int>(VarType::Closure);
    cl.data_.cl = reinterpret_cast<VarClosure *>(0x1234);
    ASSERT_TRUE(cl.TestTrue());
    const std::string cl_str = cl.ToString();
    ASSERT_NE(cl_str.find("function("), std::string::npos);
    ASSERT_NE(cl.Hash(), 0u);
    Var cl2;
    cl2.type_ = static_cast<int>(VarType::Closure);
    cl2.data_.cl = reinterpret_cast<VarClosure *>(0x1234);
    ASSERT_TRUE(cl.Equal(cl2));
    cl2.data_.cl = reinterpret_cast<VarClosure *>(0x5678);
    ASSERT_FALSE(cl.Equal(cl2));
}

TEST(var, equal_same_type_int_bool_float) {
    ASSERT_TRUE(Var(static_cast<int64_t>(7)).Equal(Var(static_cast<int64_t>(7))));
    ASSERT_FALSE(Var(static_cast<int64_t>(7)).Equal(Var(static_cast<int64_t>(8))));
    ASSERT_TRUE(Var(true).Equal(Var(true)));
    ASSERT_FALSE(Var(true).Equal(Var(false)));
    ASSERT_TRUE(Var(1.25).Equal(Var(1.25)));
    ASSERT_FALSE(Var(1.25).Equal(Var(1.5)));
}

TEST(var, type_unknown_string) {
    EXPECT_EQ(VarTypeToString(static_cast<VarType>(999)), "UNKNOWN");
    EXPECT_EQ(VarTypeToString(static_cast<VarType>(-1)), "UNKNOWN");
}

TEST(var, fakelua_varto_vi_and_native_bridge) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();

    CVar nil{};
    VarInterface *vi_nil = inter::FakeluaToNativeObj(s, nil);
    ASSERT_NE(vi_nil, nullptr);
    EXPECT_EQ(vi_nil->ViGetType(), VarInterface::Type::NIL);
    delete vi_nil;

    CVar b{};
    b.type_ = static_cast<int>(VarType::Bool);
    b.data_.b = true;
    VarInterface *vi_b = inter::FakeluaToNativeObj(s, b);
    EXPECT_EQ(vi_b->ViGetType(), VarInterface::Type::BOOL);
    EXPECT_TRUE(vi_b->ViGetBool());
    delete vi_b;

    CVar f{};
    f.type_ = static_cast<int>(VarType::Float);
    f.data_.f = 2.5;
    VarInterface *vi_f = inter::FakeluaToNativeObj(s, f);
    EXPECT_EQ(vi_f->ViGetType(), VarInterface::Type::FLOAT);
    delete vi_f;

    // Hash/bucket table path (nodes_ + active_list_)
    CVar big = table::TableHelper::CreateTable(s);
    for (int i = 0; i < 20; ++i) {
        CVar k{};
        k.type_ = static_cast<int>(VarType::StringId);
        k.data_.i = s->GetConstString().Alloc("k" + std::to_string(i));
        table::TableHelper::SetTable(s, big, k, interp_rt::Int(i));
    }
    VarInterface *vi_t = inter::FakeluaToNativeObj(s, big);
    ASSERT_NE(vi_t, nullptr);
    EXPECT_EQ(vi_t->ViGetType(), VarInterface::Type::TABLE);
    delete vi_t;

    // Closure / Multi unsupported
    CVar cl{};
    cl.type_ = static_cast<int>(VarType::Closure);
    EXPECT_THROW(inter::FakeluaToNativeObj(s, cl), std::exception);
    CVar multi = inter::AllocMultiCVar(s, 2);
    EXPECT_THROW(inter::FakeluaToNativeObj(s, multi), std::exception);

    // ViToVar paths via NativeToFakeluaVarInterface
    EXPECT_EQ(inter::NativeToFakeluaVarInterface(s, nullptr).type_, static_cast<int>(VarType::Nil));
    SimpleVarImpl bool_vi;
    bool_vi.ViSetBool(false);
    EXPECT_EQ(inter::NativeToFakeluaVarInterface(s, &bool_vi).type_, static_cast<int>(VarType::Bool));
    SimpleVarImpl float_vi;
    float_vi.ViSetFloat(1.25);
    EXPECT_EQ(inter::NativeToFakeluaVarInterface(s, &float_vi).type_, static_cast<int>(VarType::Float));
    // default / unknown: leave as NIL after Clear — already covered; synthesize bogus by INT then
    // Call NativeFloat helper
    CVar nf = NativeFloat(s, 3.14);
    EXPECT_EQ(nf.type_, static_cast<int>(VarType::Float));

    EXPECT_EQ(inter::GetMultiCVarCount(multi), 2);
    CVar args_multi[1] = {multi};
    inter::SetMultiCVarElement(multi, 0, interp_rt::Int(11));
    inter::SetMultiCVarElement(multi, 1, interp_rt::Int(22));
    EXPECT_EQ(inter::GetNativeArg(s, args_multi, 1, 0).data_.i, 11);
    EXPECT_EQ(inter::GetNativeArg(s, args_multi, 1, 1).data_.i, 22);

    // Nested multi unwrap in GetNativeArg
    CVar nested = inter::AllocMultiCVar(s, 1);
    inter::SetMultiCVarElement(nested, 0, multi);// multi-in-multi
    CVar args_nested[1] = {nested};
    // idx 0 yields nested multi then unwraps first element of multi -> 11?
    // GetNativeArg: args[0] is Multi nested; gets idx from nested; then while Multi unwraps.
    CVar got = inter::GetNativeArg(s, args_nested, 1, 0);
    // nested[0] is `multi` (Multi); unwrap loop takes multi.vars[0] == 11
    EXPECT_EQ(got.type_, static_cast<int>(VarType::Int));

    EXPECT_THROW(inter::DispatchCallClosure(s, nullptr, nullptr, 0, JIT_GCC), std::exception);
    VarClosure empty_cl{};
    empty_cl.func_ptr = nullptr;
    empty_cl.code_str = nullptr;
    EXPECT_THROW(inter::DispatchCallClosure(s, &empty_cl, nullptr, 0, JIT_GCC), std::exception);

    EXPECT_THROW(InterpreterExecute(s, nullptr, nullptr, 0, nullptr), std::exception);
    EXPECT_THROW(FakeluaInterpCall(s, nullptr, 0, nullptr), std::exception);

    // C exports used by JIT
    CVar m2 = FakeluaAllocMultiCVar(s, 1);
    FakeluaSetMultiCVarElement(&m2, 0, interp_rt::Int(9));
    EXPECT_EQ(inter::GetMultiCVarElement(m2, 0).data_.i, 9);

    // NativeObject null
    EXPECT_EQ(inter::NativeToFakeluaNativeObject(s, nullptr).type_, static_cast<int>(VarType::Nil));
}

TEST(var, cvar_to_integer_catch_paths) {
    FakeluaStateGuard guard;
    auto *s = guard.GetState();
    Var huge_digits;
    huge_digits.SetTempString(s, std::string(400, '9'));
    EXPECT_EQ(inter::CVarToInteger(reinterpret_cast<const CVar &>(huge_digits), 7), 7);
    Var bad_exp;
    bad_exp.SetTempString(s, "1.2e");
    EXPECT_EQ(inter::CVarToInteger(reinterpret_cast<const CVar &>(bad_exp), 8), 8);
}
