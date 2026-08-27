#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_xml, decode_element) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./xml/test_xml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "XmlTest.test_decode_element", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_xml, decode_attribute) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./xml/test_xml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "XmlTest.test_decode_attribute", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_xml, decode_nested) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./xml/test_xml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "XmlTest.test_decode_nested", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_xml, decode_error) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./xml/test_xml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "XmlTest.test_decode_error", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_xml, encode_basic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./xml/test_xml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "XmlTest.test_encode_basic", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_xml, roundtrip) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./xml/test_xml_basic.lua", config);
    int64_t ret = 0;
    Call(s, JIT_TCC, "XmlTest.test_roundtrip", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}
