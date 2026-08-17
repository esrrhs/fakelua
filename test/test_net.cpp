#include "fakelua.h"
#include "gtest/gtest.h"
#include "var/var_type.h"
#include "var/var_string.h"

#include <string>

using namespace fakelua;

// ─────────────────────────────────────────────────────────────────────────────
// 测试 1: server 创建/销毁
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_net, test_server_create_destroy) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileString(s, R"(
        package "NetCreate"
        function run()
            local srv = net.server({port = 19999, maxconn = 10})
            srv:close()
            return 1
        end
    )", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "NetCreate.run", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// 测试 2: client 创建/销毁
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_net, test_client_create_destroy) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileString(s, R"(
        package "NetClientCreate"
        function run()
            local c = net.client({port = 19998})
            c:close()
            return 1
        end
    )", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "NetClientCreate.run", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// 测试 3: echo 收发（核心：验证 C++ → Lua 按函数名回调机制）
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_net, test_echo_basic) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_server_client.lua", config);

    int64_t conn_count = 0, recv_count = 0;
    std::string server_data, client_data;
    Call(s, JIT_TCC, "NetTest.test_echo", std::tie(conn_count, recv_count, server_data, client_data));

    EXPECT_GE(conn_count, 1) << "server should accept connection";
    EXPECT_GE(recv_count, 1) << "server should receive data";
    EXPECT_EQ(server_data, "hello fakelua") << "server should receive exact data";
    EXPECT_EQ(client_data, "echo:hello fakelua") << "client should receive echo";

    FakeluaDeleteState(s);
}

// ─────────────────────────────────────────────────────────────────────────────
// 测试 4: 多包收发
// ─────────────────────────────────────────────────────────────────────────────

TEST(test_net, test_multiple_packets) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_multi_packets.lua", config);

    int64_t count = 0;
    std::string p1, p2, p3;
    Call(s, JIT_TCC, "NetMulti.test_multi", std::tie(count, p1, p2, p3));

    EXPECT_EQ(count, 3) << "should receive 3 packets";
    EXPECT_EQ(p1, "packet1");
    EXPECT_EQ(p2, "packet2");
    EXPECT_EQ(p3, "packet3");

    FakeluaDeleteState(s);
}
