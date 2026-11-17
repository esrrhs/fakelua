#include "jit/vm.h"
#include "state/const_string.h"
#include "state/state.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include <array>
#include <gtest/gtest.h>
#include <limits>

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

namespace {
void RegisterOpaqueTableKeys(State *s) {
    RegisterNativeFunction(s, "opaque_table_key", 1, false, [](State *, CVar *args, int) { return args[0]; });
    RegisterNativeFunction(s, "opaque_string_key", 1, false, [](State *state, CVar *args, int) { return inter::NativeToFakeluaStringView(state, AsVar(args[0]).ToString(false, false)); });
}
}// namespace

TEST(test_table, dynamic_quick_keys_and_cross_string_representations) {
    FakeluaStateGuard state;
    RegisterOpaqueTableKeys(state.GetState());
    CompileFile(state.GetState(), "./table/test_table_dynamic_keys.lua", {});
    int64_t result = 0;
    CallAll(state.GetState(), "test_dynamic_quick_keys", result);
    EXPECT_EQ(result, 1);
}

TEST(test_table, dynamic_hash_collision_insert_update_remove) {
    FakeluaStateGuard state;
    RegisterOpaqueTableKeys(state.GetState());
    CompileFile(state.GetState(), "./table/test_table_dynamic_keys.lua", {});
    int64_t result = 0;
    CallAll(state.GetState(), "test_dynamic_collision_chains", result);
    EXPECT_EQ(result, 1);
}

TEST(test_table, host_sparse_hash_promotes_dense_prefix_without_losing_keys) {
    FakeluaStateGuard state;
    auto *s = state.GetState();
    CVar value = table::TableHelper::CreateTable(s);
    for (int64_t key: {2, 3, 4, 5, 50, 60, 70, 80, 100}) table::TableHelper::SetTableInt(s, value, key, inter::NativeToFakeluaInt(s, key * 10));
    for (int64_t key: {2, 3, 4, 5, 50, 60, 70, 80, 100}) EXPECT_EQ(inter::CVarToInteger(table::TableHelper::GetTableInt(s, value, key)), key * 10);
    EXPECT_EQ(table::TableHelper::GetTableLen(value), 0);
    table::TableHelper::SetTableInt(s, value, 1, inter::NativeToFakeluaInt(s, 10));
    EXPECT_EQ(table::TableHelper::GetTableLen(value), 5);
    EXPECT_EQ(table::TableHelper::CollectKVPairs(value).size(), 10u);
    CVar nan{};
    nan.type_ = static_cast<int>(VarType::Float);
    nan.data_.f = std::numeric_limits<double>::quiet_NaN();
    EXPECT_THROW(table::TableHelper::SetTable(s, value, nan, inter::NativeToFakeluaInt(s, 1)), std::exception);
}

TEST(test_table, host_table_and_closure_keys_use_reference_identity) {
    FakeluaStateGuard state;
    auto *s = state.GetState();
    CompileString(s, "function table_key_factory() return function() return 1 end end", {});
    CVar value = table::TableHelper::CreateTable(s);
    std::vector<CVar> keys;
    for (int i = 0; i < 12; ++i) {
        keys.push_back(table::TableHelper::CreateTable(s));
        keys.push_back(CallByNameArgs(s, JIT_INTERP, "table_key_factory", 0, nullptr));
    }
    for (size_t i = 0; i < keys.size(); ++i) table::TableHelper::SetTable(s, value, keys[i], inter::NativeToFakeluaInt(s, i + 10));
    for (size_t i = 0; i < keys.size(); ++i) EXPECT_EQ(inter::CVarToInteger(table::TableHelper::GetTable(s, value, keys[i])), i + 10);
    for (size_t i = 0; i < keys.size(); i += 3) table::TableHelper::SetTable(s, value, keys[i], inter::NativeToFakeluaNil(s));
    for (size_t i = 0; i < keys.size(); ++i) {
        const auto found = table::TableHelper::GetTable(s, value, keys[i]);
        if (i % 3 == 0) EXPECT_EQ(found.type_, static_cast<int>(VarType::Nil));
        else
            EXPECT_EQ(inter::CVarToInteger(found), i + 10);
    }
    EXPECT_EQ(table::TableHelper::CollectKVPairs(value).size(), 16u);
}

TEST(test_table, abi_spec_key_fallback_read_write_and_length) {
    FakeluaStateGuard state;
    auto *s = state.GetState();
    CVar value = table::TableHelper::CreateTable(s);
    std::array<CVar, 3> keys{inter::NativeToFakeluaDouble(s, 1.0), inter::NativeToFakeluaInt(s, 2), inter::NativeToFakeluaString(s, "label")};
    std::array<CVar, 3> values{inter::NativeToFakeluaInt(s, 10), inter::NativeToFakeluaInt(s, 20), inter::NativeToFakeluaInt(s, 30)};
    value.data_.t->spec_keys = keys.data();
    value.data_.t->spec_vals = values.data();
    value.data_.t->spec_count = keys.size();
    value.data_.t->seq_len_valid_ = 0;
    EXPECT_EQ(inter::CVarToInteger(table::TableHelper::GetTableInt(s, value, 1)), 10);
    EXPECT_EQ(inter::CVarToInteger(table::TableHelper::GetTable(s, value, keys[2])), 30);
    EXPECT_EQ(inter::CVarToInteger(table::TableHelper::GetTableStrId(s, value, "label")), 30);
    EXPECT_EQ(table::TableHelper::GetTableLen(value), 2);
    table::TableHelper::SetTableInt(s, value, 2, inter::NativeToFakeluaInt(s, 99));
    EXPECT_EQ(inter::CVarToInteger(values[1]), 99);
    EXPECT_EQ(table::TableHelper::GetTableInt(s, value, 4).type_, static_cast<int>(VarType::Nil));
    EXPECT_TRUE(table::TableHelper::VarKeyEqualInt(keys[0], 1));
    EXPECT_FALSE(table::TableHelper::VarKeyEqualInt(keys[2], 1));
}

TEST(test_table, presized_move_and_mixed_numeric_sort_contracts) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./table/test_table_dynamic_keys.lua", {});
    for (auto backend: AllJitTypes()) {
        int64_t result = 0;
        Call(state.GetState(), backend, "test_presized_move_and_mixed_sort", result);
        EXPECT_EQ(result, 1) << JitTypeName(backend);
    }
}

TEST(test_table, zero_initialized_abi_table_rebuilds_sequence_cache) {
    FakeluaStateGuard state;
    auto *s = state.GetState();
    // Generated C also allocates zero-initialized ABI tables without calling
    // TableHelper::CreateTable; their initial sequence cache is invalid.
    VarTable raw{};
    raw.free_list_idx_ = VarTable::INVALID_INDEX;
    CVar value{};
    value.type_ = static_cast<int>(VarType::Table);
    value.data_.t = &raw;
    table::TableHelper::SetTableInt(s, value, 1, inter::NativeToFakeluaInt(s, 10));
    table::TableHelper::SetTableInt(s, value, 2, inter::NativeToFakeluaInt(s, 20));
    EXPECT_EQ(table::TableHelper::GetTableLen(value), 2);
    EXPECT_EQ(raw.seq_len_valid_, 1u);
    table::TableHelper::SetTableInt(s, value, 1, inter::NativeToFakeluaNil(s));
    EXPECT_EQ(table::TableHelper::GetTableLen(value), 0);
    table::TableHelper::SetTableInt(s, value, 1, inter::NativeToFakeluaInt(s, 30));
    EXPECT_EQ(table::TableHelper::GetTableLen(value), 2);
}

TEST(test_table, abi_string_keys_use_bytes_across_quick_and_bucket_storage) {
    FakeluaStateGuard string_owner;
    const int64_t string_id = string_owner.GetState()->GetConstString().Alloc("ab");
    for (bool bucketed: {false, true}) {
        for (auto representation: {VarType::String, VarType::StringId}) {
            for (auto backend: AllJitTypes()) {
                SCOPED_TRACE(::testing::Message() << bucketed << " " << static_cast<int>(representation) << " " << JitTypeName(backend));
                FakeluaStateGuard state;
                VarTable raw{};
                raw.count_ = 1;
                raw.free_list_idx_ = VarTable::INVALID_INDEX;
                std::array<VarTable::TableNode, 3> nodes{};
                std::array<uint32_t, 3> active{};
                const auto hash = VarString::HashOf("ab");
                auto *entry = &raw.quick_data_[0];
                if (bucketed) {
                    raw.bucket_count_ = 2;
                    raw.nodes_ = nodes.data();
                    raw.active_list_ = active.data();
                    active[0] = hash & (raw.bucket_count_ - 1);
                    entry = &nodes[active[0]].entry;
                    for (auto &node: nodes) {
                        node.next = VarTable::INVALID_INDEX;
                        node.active_pos = VarTable::INVALID_INDEX;
                    }
                    nodes[active[0]].active_pos = 0;
                    raw.free_list_idx_ = 2;
                }
                // Host-provided ABI tables can retain their own string storage.
                // The owning State outlives every consumer and owns the bytes.
                entry->key.type_ = static_cast<int>(representation);
                if (representation == VarType::String) entry->key.data_.s = reinterpret_cast<VarString *>(string_id);
                else
                    entry->key.data_.i = string_id;
                entry->val.SetInt(42);
                entry->hash = hash;
                CVar wrapped{};
                wrapped.type_ = static_cast<int>(VarType::Table);
                wrapped.data_.t = &raw;
                EXPECT_EQ(inter::CVarToInteger(table::TableHelper::GetTableStrId(state.GetState(), wrapped, "ab")), 42);
                RegisterNativeFunction(state.GetState(), "legacy_abi_table", 0, false, [wrapped](State *, CVar *, int) { return wrapped; });
                CompileString(state.GetState(), R"(
                    function test_legacy_abi_table()
                        local value = legacy_abi_table()
                        assert(value.ab == 42)
                        assert(value.bA == nil and value.zz == nil)
                        value.ab = 99
                        assert(value.ab == 99)
                        return 1
                    end
                )",
                              {});
                int64_t result = 0;
                Call(state.GetState(), backend, "test_legacy_abi_table", result);
                EXPECT_EQ(result, 1);
                EXPECT_EQ(entry->val.GetInt(), 99);
                EXPECT_EQ(inter::CVarToInteger(table::TableHelper::GetTableStrId(state.GetState(), wrapped, "ab")), 99);
            }
        }
    }
}

TEST(test_table, sparse_prefix_hash_resize_keeps_integer_and_string_entries) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./table/test_table_sparse_prefix_resize.lua", {});
    int64_t result = 0;
    CallAll(state.GetState(), "test_sparse_prefix_resize", result);
    EXPECT_EQ(result, 1);
}

TEST(test_table, mixed_constructor_and_dynamic_store_contracts) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./table/test_table_dynamic_keys.lua", {});
    int64_t result = 0;
    CallAll(state.GetState(), "test_mixed_constructor_and_dynamic_store_contracts", result);
    EXPECT_EQ(result, 1);
}

TEST(test_table, loaded_source_comparator_and_unpack_type_contract) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./table/test_table_dynamic_keys.lua", {});
    int64_t result = 0;
    CallAll(state.GetState(), "test_loaded_sort_comparator", result);
    EXPECT_EQ(result, 1);
}
