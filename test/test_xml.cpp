#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_xml, decode_element) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./xml/test_xml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "XmlTest.test_decode_element", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_xml, decode_attribute) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./xml/test_xml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "XmlTest.test_decode_attribute", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_xml, decode_nested) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./xml/test_xml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "XmlTest.test_decode_nested", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_xml, decode_error) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./xml/test_xml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "XmlTest.test_decode_error", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_xml, encode_basic) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./xml/test_xml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "XmlTest.test_encode_basic", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_xml, roundtrip) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./xml/test_xml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "XmlTest.test_roundtrip", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}
