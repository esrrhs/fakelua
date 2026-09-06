#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

// ─────────────────────────────────────────────────────────────────────────────
// Lua 绑定的网络层接口测试（黑盒：只通过 fakelua.h 暴露的 API）
// ─────────────────────────────────────────────────────────────────────────────

// 测试 1: server 创建/销毁
TEST(test_net, test_server_create_destroy) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_create_destroy.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "NetCreate.test_server_create_destroy", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

// 测试 2: client 创建/销毁
TEST(test_net, test_client_create_destroy) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_create_destroy.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "NetCreate.test_client_create_destroy", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

// 测试 3: echo 收发（核心：验证 C++ → Lua 按函数名回调 + 纯函数返回指令）
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

// 测试 4: 多包收发
TEST(test_net, test_multiple_packets) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_multi_packets.lua", config);

    int64_t recv_count = 0;
    std::string last_data;
    Call(s, JIT_TCC, "NetMulti.test_multi", std::tie(recv_count, last_data));

    EXPECT_EQ(recv_count, 3) << "should receive 3 packets";
    EXPECT_EQ(last_data, "packet3") << "last echoed data should be packet3";

    FakeluaDeleteState(s);
}

// 测试 5: 多 server + 多 client 同时运行，相互独立不干扰
TEST(test_net, test_multi_servers_multi_clients) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_multi_endpoints.lua", config);

    int64_t conn_a = 0, recv_a = 0, conn_b = 0, recv_b = 0;
    std::string data_a, echo_a, data_b, echo_b;
    Call(s, JIT_TCC, "NetMultiEndpoints.test_multi_servers_multi_clients",
         std::tie(conn_a, recv_a, data_a, echo_a, conn_b, recv_b, data_b, echo_b));

    // server_a 应接收来自 client_a 的数据
    EXPECT_GE(conn_a, 1) << "server_a should accept connection";
    EXPECT_GE(recv_a, 1) << "server_a should receive data";
    EXPECT_EQ(data_a, "hello_a") << "server_a should receive hello_a";
    EXPECT_EQ(echo_a, "from_a:hello_a") << "client_a should receive echo from server_a";

    // server_b 应接收来自 client_b 的数据
    EXPECT_GE(conn_b, 1) << "server_b should accept connection";
    EXPECT_GE(recv_b, 1) << "server_b should receive data";
    EXPECT_EQ(data_b, "hello_b") << "server_b should receive hello_b";
    EXPECT_EQ(echo_b, "from_b:hello_b") << "client_b should receive echo from server_b";

    FakeluaDeleteState(s);
}

// 测试 6: 1个 server + 多个 client 同时连接，server 能区分处理各连接
TEST(test_net, test_one_server_multi_clients) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_multi_endpoints.lua", config);

    int64_t conn_count = 0, recv_count = 0;
    std::string echo1, echo2, echo3;
    Call(s, JIT_TCC, "NetMultiEndpoints.test_one_server_multi_clients",
         std::tie(conn_count, recv_count, echo1, echo2, echo3));

    // server 应接受 3 个连接
    EXPECT_EQ(conn_count, 3) << "server should accept 3 connections";
    // server 应收到至少 3 个包
    EXPECT_GE(recv_count, 3) << "server should receive at least 3 packets";

    // 每个 client 收到的 echo 应与各自发送的消息对应
    EXPECT_EQ(echo1, "from_a:msg1") << "client1 should receive echo of msg1";
    EXPECT_EQ(echo2, "from_a:msg2") << "client2 should receive echo of msg2";
    EXPECT_EQ(echo3, "from_a:msg3") << "client3 should receive echo of msg3";

    FakeluaDeleteState(s);
}

// 测试 7: server :close() 后 NativeObject group 不再泄漏（DestroyGroup 生效）
TEST(test_net, test_server_close_releases_native_object) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_create_destroy.lua", config);

    // 多次创建并关闭 server，每次 close 都应释放 NativeObject
    for (int i = 0; i < 5; ++i) {
        int64_t ret = 0;
        Call(s, JIT_TCC, "NetCreate.test_server_create_destroy", ret);
        EXPECT_EQ(ret, 1);
    }

    FakeluaDeleteState(s);
}

// 测试 8: 在 recv 回调里 close — 应不爆且能延后到 tick 返回后真正关闭
TEST(test_net, test_close_in_recv) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_server_client.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "NetTest.test_close_in_recv", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

// 测试 9: Lua 层多 Framer 协议测试 (header2_be, header2_le, header4_le, line, fixed, custom_lua)
TEST(test_net, test_framer_lua_protocols) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_framer.lua", config);

    // 1. 测试 2 字节大端
    {
        std::string s_data, c_data;
        Call(s, JIT_TCC, "NetFramerTest.test_framer_2be", std::tie(s_data, c_data));
        EXPECT_EQ(s_data, "hello_2be");
        EXPECT_EQ(c_data, "echo:hello_2be");
    }

    // 2. 测试 2 字节小端
    {
        std::string s_data, c_data;
        Call(s, JIT_TCC, "NetFramerTest.test_framer_2le", std::tie(s_data, c_data));
        EXPECT_EQ(s_data, "hello_2le");
        EXPECT_EQ(c_data, "echo:hello_2le");
    }

    // 3. 测试 4 字节小端
    {
        std::string s_data, c_data;
        Call(s, JIT_TCC, "NetFramerTest.test_framer_4le", std::tie(s_data, c_data));
        EXPECT_EQ(s_data, "hello_4le");
        EXPECT_EQ(c_data, "echo:hello_4le");
    }

    // 4. 测试 换行符定界 (line delimiter)
    {
        std::string s_data, c_data;
        Call(s, JIT_TCC, "NetFramerTest.test_framer_line", std::tie(s_data, c_data));
        EXPECT_EQ(s_data, "line_command_1");
        EXPECT_EQ(c_data, "echo:line_command_1");
    }

    // 5. 测试 固定长度 (fixed length)
    {
        std::string s_data;
        Call(s, JIT_TCC, "NetFramerTest.test_framer_fixed", s_data);
        EXPECT_EQ(s_data, "12345678");
    }

    // 6. 测试 自定义 Lua 解包函数
    {
        std::string s_data;
        Call(s, JIT_TCC, "NetFramerTest.test_framer_custom_lua", s_data);
        EXPECT_EQ(s_data, "custom_msg_dollar");
    }

    FakeluaDeleteState(s);
}

// 测试 10: WebSocket echo（Lua 绑定）
TEST(test_net, test_websocket_echo_lua) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_websocket.lua", config);

    int64_t conn_count = 0, recv_count = 0;
    std::string server_data, client_data;
    Call(s, JIT_TCC, "NetWsTest.test_ws_echo", std::tie(conn_count, recv_count, server_data, client_data));

    EXPECT_GE(conn_count, 1) << "ws server should accept connection";
    EXPECT_GE(recv_count, 1) << "ws server should receive data";
    EXPECT_EQ(server_data, "hello websocket");
    EXPECT_EQ(client_data, "echo:hello websocket");

    FakeluaDeleteState(s);
}

// 测试 11: server stop 后再 start 能重新成功绑定监听（验证 stop 后重新打开 bug 修复）
TEST(test_net, test_server_stop_restart) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_create_destroy.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "NetCreate.test_server_stop_restart", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

// 测试 12: 反复 connect + close，验证 slot 自动释放并复用（maxconn=2 时成功服务 6 次客户端连接）
TEST(test_net, test_slot_reuse_repeated_connect) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_server_client.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "NetTest.test_slot_reuse_repeated_connect", ret);
    EXPECT_EQ(ret, 6);

    FakeluaDeleteState(s);
}

// 测试 13: 对未监听端口进行 client 连接 — 正确处理连接失败，send 返回 false
TEST(test_net, test_client_connect_fail) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_server_client.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "NetTest.test_client_connect_fail", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

// 测试 14: 发送数据超过缓冲区容量 — 返回 false 不截断
TEST(test_net, test_send_buffer_full) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_server_client.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "NetTest.test_send_buffer_full", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

