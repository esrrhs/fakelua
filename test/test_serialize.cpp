#include "fakelua.h"
#include "test_jit.h"
#include "gtest/gtest.h"

using namespace fakelua;

// 二进制序列化 Lua 绑定测试

TEST(test_serialize, test_int) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_int.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_int", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_float) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_float.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_float", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_bool_nil) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_bool_nil.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_bool_nil", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_string) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_string.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_string", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_table.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_table", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_array_9) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_table.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_array_9", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_cycle_throw) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_table.lua", config);
    int64_t ret = 0;
    CallThrow(s, "SerializeTest.test_cycle_throw", ret);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_decode_too_deep) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_table.lua", config);
    int64_t ret = 0;
    CallThrow(s, "SerializeTest.test_decode_too_deep", ret);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_decode_huge_table) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_table.lua", config);
    int64_t ret = 0;
    CallThrow(s, "SerializeTest.test_decode_huge_table", ret);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_skip) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_skip.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_skip", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_text_roundtrip) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_boost.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_text_roundtrip", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_xml_roundtrip) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_boost.lua", config);
    int64_t ret = 0;
    CallAll(s, "SerializeTest.test_xml_roundtrip", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_text_cycle) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_boost.lua", config);
    int64_t ret = 0;
    CallThrow(s, "SerializeTest.test_text_cycle", ret);
    FakeluaDeleteState(s);
}

TEST(test_serialize, test_text_bad) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./serialize/test_serialize_boost.lua", config);
    int64_t ret = 0;
    CallThrow(s, "SerializeTest.test_text_bad", ret);
    FakeluaDeleteState(s);
}
