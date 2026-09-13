#include "fakelua.h"
#include "test_jit.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_container, deque) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./container/test_container.lua", config);
    int64_t ret = 0;
    CallAll(s, "ContainerTest.test_deque", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_container, map) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./container/test_container.lua", config);
    int64_t ret = 0;
    CallAll(s, "ContainerTest.test_map", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_container, set) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./container/test_container.lua", config);
    int64_t ret = 0;
    CallAll(s, "ContainerTest.test_set", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_container, closed) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./container/test_container.lua", config);
    int64_t ret = 0;
    CallThrow(s, "ContainerTest.test_closed", ret);
    FakeluaDeleteState(s);
}

TEST(test_container, bad_table_value) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./container/test_container.lua", config);
    int64_t ret = 0;
    CallThrow(s, "ContainerTest.test_bad_table_value", ret);
    FakeluaDeleteState(s);
}

TEST(test_container, deque_set_oor) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./container/test_container.lua", config);
    int64_t ret = 0;
    CallThrow(s, "ContainerTest.test_deque_set_oor", ret);
    FakeluaDeleteState(s);
}

TEST(test_container, vector) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./container/test_container.lua", config);
    int64_t ret = 0;
    CallAll(s, "ContainerTest.test_vector", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_container, small_vector) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./container/test_container.lua", config);
    int64_t ret = 0;
    CallAll(s, "ContainerTest.test_small_vector", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_container, list) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./container/test_container.lua", config);
    int64_t ret = 0;
    CallAll(s, "ContainerTest.test_list", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_container, nested) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./container/test_container.lua", config);
    int64_t ret = 0;
    CallAll(s, "ContainerTest.test_nested", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_container, vector_set_oor) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./container/test_container.lua", config);
    int64_t ret = 0;
    CallThrow(s, "ContainerTest.test_vector_set_oor", ret);
    FakeluaDeleteState(s);
}

TEST(test_container, list_set_oor) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./container/test_container.lua", config);
    int64_t ret = 0;
    CallThrow(s, "ContainerTest.test_list_set_oor", ret);
    FakeluaDeleteState(s);
}

TEST(test_container, bad_fn_value) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./container/test_container.lua", config);
    int64_t ret = 0;
    CallThrow(s, "ContainerTest.test_bad_fn_value", ret);
    FakeluaDeleteState(s);
}
