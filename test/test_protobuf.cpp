#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

// ─────────────────────────────────────────────────────────────────────────────
// Protobuf 绑定测试
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_protobuf, test_load) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_load.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "ProtobufTest.test_load", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_scalar) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_scalar.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "ProtobufTest.test_scalar", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_wrong_wire_type) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_scalar.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "ProtobufTest.test_wrong_wire_type", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_message) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_message.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "ProtobufTest.test_message", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_repeated) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_repeated.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "ProtobufTest.test_repeated", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_map) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_map.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "ProtobufTest.test_map", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_enum) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_enum.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "ProtobufTest.test_enum", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_map_enum) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_enum.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "ProtobufTest.test_map_enum", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_cycle_throw) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_message.lua", config);
    int64_t ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "ProtobufTest.test_cycle_throw", ret), std::exception);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_decode_too_deep) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_message.lua", config);
    int64_t ret = 0;
    EXPECT_THROW(Call(s, JIT_GCC, "ProtobufTest.test_decode_too_deep", ret), std::exception);
    FakeluaDeleteState(s);
}

TEST(test_protobuf, test_optional_nil) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./protobuf/test_protobuf_message.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "ProtobufTest.test_optional_nil", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
