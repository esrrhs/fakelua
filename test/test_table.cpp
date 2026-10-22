#include <gtest/gtest.h>

#include "fakelua.h"
#include "native/table/native_table.h"
#include "test_jit.h"
#include "var/var.h"
#include "var/var_multi.h"

#include <cstdint>
#include <string>

using namespace fakelua;

namespace {

// 与 native_table.cpp 的 kMaxUnpackResults 同一公式：一块临时内存装得下的 CVar 个数。
constexpr int64_t kUnpackJustOver = static_cast<int64_t>(((1024ULL * 1024ULL) - sizeof(VarMulti)) / sizeof(CVar) + 1);

template<typename... Args>
void ExpectThrowsWith(State *s, const std::string &name, const char *needle, Args &&...args) {
    for (const auto jit_type: ExceptionJitTypes()) {
        SCOPED_TRACE(::testing::Message() << "jit=" << JitTypeName(jit_type) << " fn=" << name);
        double res = 0;
        try {
            Call(s, jit_type, name, res, std::forward<Args>(args)...);
            ADD_FAILURE() << "expected throw from " << name;
        } catch (const std::exception &e) {
            const std::string msg = e.what();
            EXPECT_NE(msg.find(needle), std::string::npos) << msg;
        }
    }
}

}// namespace

TEST(test_table, test_table_insert_remove) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_insert_remove.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_insert_remove", res);
        EXPECT_NEAR(res, 77.0, 1e-4);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_array_part) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_array_part.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_table_array_part", res);
        EXPECT_EQ(res, 36864326318LL);
    }

    FakeluaDeleteState(s);
}

// 可选参数显式传 nil 与省略等价（Lua 5.4 luaL_optinteger 语义）：
// table.concat 的 i/j、table.unpack 的 i/j、table.remove 的 pos。
// 期望值由 Lua 5.4.9 参考实现校准。
TEST(test_table, test_table_optnil) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_optnil.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_table_optnil", res);
        EXPECT_EQ(res, 83LL);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_native_array_part) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    // 1. 测试直接调用 TableHelper::SetTableInt，逆序写入触发 TableArrResizeInsert -> TableArrTryResize
    CVar tbl = table::TableHelper::CreateTable(s);
    for (int64_t i = 8; i >= 2; --i) {
        table::TableHelper::SetTableInt(s, tbl, i, Var(static_cast<int64_t>(i * 10)));
    }
    table::TableHelper::SetTableInt(s, tbl, 100, Var(int64_t{1000}));
    table::TableHelper::SetTableInt(s, tbl, 1, Var(int64_t{10}));

    EXPECT_EQ(table::TableHelper::GetTableInt(s, tbl, 1).data_.i, 10);
    EXPECT_EQ(table::TableHelper::GetTableInt(s, tbl, 2).data_.i, 20);
    EXPECT_EQ(table::TableHelper::GetTableInt(s, tbl, 8).data_.i, 80);
    EXPECT_EQ(table::TableHelper::GetTableInt(s, tbl, 100).data_.i, 1000);
    EXPECT_EQ(table::TableHelper::GetTableLen(tbl), 8);

    // 2. 桶模式下的 TableArrResizeInsert
    CVar tbl2 = table::TableHelper::CreateTable(s);
    for (int64_t i = 30; i >= 2; --i) {
        table::TableHelper::SetTableInt(s, tbl2, i, Var(static_cast<int64_t>(i)));
    }
    table::TableHelper::SetTableInt(s, tbl2, 1, Var(int64_t{1}));
    EXPECT_EQ(table::TableHelper::GetTableLen(tbl2), 30);
    EXPECT_EQ(table::TableHelper::GetTableInt(s, tbl2, 1).data_.i, 1);
    EXPECT_EQ(table::TableHelper::GetTableInt(s, tbl2, 30).data_.i, 30);

    // 3. 带数组空洞的表导出到 VarInterface（覆盖 fakelua.cpp 中 av.Type() == VarType::Nil）
    CVar tbl3 = table::TableHelper::CreateTable(s);
    table::TableHelper::SetTableInt(s, tbl3, 1, Var(int64_t{100}));
    table::TableHelper::SetTableInt(s, tbl3, 2, Var(int64_t{200}));
    table::TableHelper::SetTableInt(s, tbl3, 2, Var());
    VarInterface *vi = inter::FakeluaToNativeObj(s, tbl3);
    ASSERT_NE(vi, nullptr);
    delete vi;

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_concat) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_concat.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_concat", res);
        EXPECT_NEAR(res, 100.0, 1e-4);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_pack_unpack) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_pack_unpack.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_pack_unpack", res);
        EXPECT_NEAR(res, 123.0, 1e-4);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_sort) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_sort.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_sort", res);
        EXPECT_NEAR(res, 100.0, 1e-4);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_large_seq) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_large_seq.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_large_seq", res);
        EXPECT_NEAR(res, 8000.0, 1e-4);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_seqlen) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_seqlen.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_seqlen", res);
        EXPECT_NEAR(res, 8100.0, 1e-4);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_sort_invalid_comparator) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./table/test_table_sort_bad_cmp.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    CallThrow(s, "test_table_sort_bad_cmp", res);

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_create) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_create.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_create", res);
        EXPECT_NEAR(res, 100.0, 1e-4);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_move) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_move.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_move", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_create_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_create_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_create_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_insert_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_insert_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_insert_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_float_index) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_float_index.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_float_index", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_remove_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_remove_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_remove_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_concat_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_concat_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_concat_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_sort_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_sort_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_sort_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_sort_custom_cmp) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_sort_cmp.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_sort_custom_cmp", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_move_overlap) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_sort_cmp.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_move_overlap", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_move_no_overlap) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_sort_cmp.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_move_no_overlap", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_concat_float) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_sort_cmp.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_concat_float", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_sort_single) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_sort_cmp.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_sort_single", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_boundary) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_boundary.lua", config);
        double res = 0;
        Call(s, jit_type, "test_table_boundary", res);
        EXPECT_NEAR(res, 5000, 0.5);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_boundary_error) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./table/test_table_boundary_error.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    CallThrow(s, "test_table_boundary_error", res);

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_error_paths) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    CompileFile(s, "./table/test_table_error_paths.lua", config);

    // TCC 是 C 编译器，不支持 C++ 异常传播，只测试 GCC 后端
    double res = 0;
    CallThrow(s, "test_table_insert_bad_arg", res);
    CallThrow(s, "test_table_remove_bad_arg", res);
    CallThrow(s, "test_table_sort_bad_arg", res);
    CallThrow(s, "test_table_concat_bad_arg", res);
    CallThrow(s, "test_table_concat_bad_sep", res);
    CallThrow(s, "test_table_move_bad_arg", res);
    CallThrow(s, "test_table_concat_too_many", res);
    CallThrow(s, "test_table_concat_min_max_range", res);
    CallThrow(s, "test_table_create_too_many", res);
    CallThrow(s, "test_table_insert_2pow63", res);
    CallThrow(s, "test_table_remove_2pow63", res);
    CallThrow(s, "test_table_sort_nan", res);
    CallThrow(s, "test_table_nan_key_set", res);
    CallThrow(s, "test_table_nan_key_get", res);
    CallThrow(s, "test_table_concat_nan", res);
    CallThrow(s, "test_table_concat_frac", res);

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_insert_arity) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;

    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./table/test_table_insert_arity.lua", config);
        int64_t res = 0;
        Call(s, jit_type, "test_table_insert_arity", res);
        EXPECT_EQ(res, 5000LL);
    }

    FakeluaDeleteState(s);
}

TEST(test_table, test_table_move_unpack_limits) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./table/test_table_limits.lua", config);

    int64_t moved = 0;
    CallAll(s, "test_table_move_over_cap", moved);
    EXPECT_EQ(moved, 5000);

    int64_t unpacked = 0;
    CallAll(s, "test_table_unpack_ok", unpacked);
    EXPECT_EQ(unpacked, 5000);

    ExpectThrowsWith(s, "test_table_move_src_wrap", "too many elements to move");
    ExpectThrowsWith(s, "test_table_move_src_overflow", "too many elements to move");
    ExpectThrowsWith(s, "test_table_move_dst_wrap", "destination wrap around");
    ExpectThrowsWith(s, "test_table_unpack_over", "too many results to unpack", kUnpackJustOver);
    ExpectThrowsWith(s, "test_table_unpack_wrap", "too many results to unpack");

    for (const char *name: {"test_table_move_presize_skip", "test_table_move_overlap_backward", "test_table_move_empty_range"}) {
        int64_t r = 0;
        CallAll(s, name, r);
        EXPECT_EQ(r, 5000) << name;
    }

    FakeluaDeleteState(s);
}
