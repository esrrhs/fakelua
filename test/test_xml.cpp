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

TEST(test_xml, encode_multi_attrs) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./xml/test_xml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "XmlTest.test_encode_multi_attrs", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_xml, encode_text_node) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./xml/test_xml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "XmlTest.test_encode_text_node", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_xml, encode_scalar_types) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./xml/test_xml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "XmlTest.test_encode_scalar_types", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_xml, encode_array) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./xml/test_xml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "XmlTest.test_encode_array", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_xml, encode_cyclic) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./xml/test_xml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "XmlTest.test_encode_cyclic", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_xml, encode_nested) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./xml/test_xml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "XmlTest.test_encode_nested", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_xml, encode_top_scalar) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./xml/test_xml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "XmlTest.test_encode_top_scalar", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}

TEST(test_xml, encode_empty_table) {
    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        State *s = FakeluaNewState();
        ASSERT_NE(s, nullptr);
        CompileConfig config;
        CompileFile(s, "./xml/test_xml_basic.lua", config);
        int64_t ret = 0;
        Call(s, jit_type, "XmlTest.test_encode_empty_table", ret);
        EXPECT_EQ(ret, 1);
        FakeluaDeleteState(s);
    }
}
