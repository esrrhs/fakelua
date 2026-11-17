#include "fakelua.h"
#include "jit/vm.h"
#include "native/net/net_buffer.h"
#include "native/net/net_event.h"
#include "native/object/native_object.h"
#include "state/state.h"
#include "test_jit.h"
#include "var/var.h"
#include "gtest/gtest.h"
#include <chrono>
#include <cstring>
#include <thread>

using namespace fakelua;

// Lua 绑定的网络层接口测试（黑盒：只通过 fakelua.h 暴露的 API）

// 测试 1: server 创建/销毁
TEST(test_net, test_server_create_destroy) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_create_destroy.lua", config);

    int64_t ret = 0;
    CallAll(s, "NetCreate.test_server_create_destroy", ret);
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
    CallAll(s, "NetCreate.test_client_create_destroy", ret);
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
    CallAll(s, "NetTest.test_echo", std::tie(conn_count, recv_count, server_data, client_data));

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
    CallAll(s, "NetMulti.test_multi", std::tie(recv_count, last_data));

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
    CallAll(s, "NetMultiEndpoints.test_multi_servers_multi_clients", std::tie(conn_a, recv_a, data_a, echo_a, conn_b, recv_b, data_b, echo_b));

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
    CallAll(s, "NetMultiEndpoints.test_one_server_multi_clients", std::tie(conn_count, recv_count, echo1, echo2, echo3));

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
        CallAll(s, "NetCreate.test_server_create_destroy", ret);
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
    CallAll(s, "NetTest.test_close_in_recv", ret);
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
        CallAll(s, "NetFramerTest.test_framer_2be", std::tie(s_data, c_data));
        EXPECT_EQ(s_data, "hello_2be");
        EXPECT_EQ(c_data, "echo:hello_2be");
    }

    // 2. 测试 2 字节小端
    {
        std::string s_data, c_data;
        CallAll(s, "NetFramerTest.test_framer_2le", std::tie(s_data, c_data));
        EXPECT_EQ(s_data, "hello_2le");
        EXPECT_EQ(c_data, "echo:hello_2le");
    }

    // 3. 测试 4 字节小端
    {
        std::string s_data, c_data;
        CallAll(s, "NetFramerTest.test_framer_4le", std::tie(s_data, c_data));
        EXPECT_EQ(s_data, "hello_4le");
        EXPECT_EQ(c_data, "echo:hello_4le");
    }

    // 4. 测试 换行符定界 (line delimiter)
    {
        std::string s_data, c_data;
        CallAll(s, "NetFramerTest.test_framer_line", std::tie(s_data, c_data));
        EXPECT_EQ(s_data, "line_command_1");
        EXPECT_EQ(c_data, "echo:line_command_1");
    }

    // 5. 测试 固定长度 (fixed length)
    {
        std::string s_data;
        CallAll(s, "NetFramerTest.test_framer_fixed", s_data);
        EXPECT_EQ(s_data, "12345678");
    }

    // 6. 测试 自定义 Lua 解包函数
    {
        std::string s_data;
        CallAll(s, "NetFramerTest.test_framer_custom_lua", s_data);
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
    CallAll(s, "NetWsTest.test_ws_echo", std::tie(conn_count, recv_count, server_data, client_data));

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
    CallAll(s, "NetCreate.test_server_stop_restart", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

TEST(test_net, test_ws_path_crlf) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_create_destroy.lua", config);

    int64_t ret = 0;
    CallAll(s, "NetCreate.test_ws_path_crlf", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

TEST(test_net, test_ip_crlf) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_create_destroy.lua", config);

    int64_t ret = 0;
    CallAll(s, "NetCreate.test_ip_crlf", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

TEST(test_net, test_bad_backlog) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_create_destroy.lua", config);

    int64_t ret = 0;
    CallAll(s, "NetCreate.test_bad_backlog", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

TEST(test_net, test_bad_fixed_len) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_create_destroy.lua", config);

    int64_t ret = 0;
    CallAll(s, "NetCreate.test_bad_fixed_len", ret);
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
    CallAll(s, "NetTest.test_slot_reuse_repeated_connect", ret);
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
    CallAll(s, "NetTest.test_client_connect_fail", ret);
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
    CallAll(s, "NetTest.test_send_buffer_full", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

TEST(test_net, test_wss_echo_lua) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_wss.lua", config);

    int64_t conn_count = 0, recv_count = 0;
    std::string server_data, client_data;
    CallAll(s, "NetWssTest.test_wss_echo", std::tie(conn_count, recv_count, server_data, client_data));

    EXPECT_GE(conn_count, 1) << "wss server should accept connection";
    EXPECT_GE(recv_count, 1) << "wss server should receive data";
    EXPECT_EQ(server_data, "hello wss");
    EXPECT_EQ(client_data, "echo:hello wss");

    FakeluaDeleteState(s);
}

TEST(test_net, test_udp_echo_lua) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_udp.lua", config);

    int64_t ret = 0;
    CallAll(s, "NetUdpTest.test_udp_echo", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

// 回归（P1-6）：在 on_event 回调里直接 obj:send() 必须可靠送达。
// 派发上下文中的 send 入队，由本轮 tick 派发完后统一泵出。
// 注意：C++ → Lua 回调按 VM 查表固定派发 TCC 编译的函数，文件级可变状态在各后端
// 有独立副本，驱动端必须与回调同后端（TCC），因此这里不走 CallAll。
TEST(test_net, test_send_in_callback) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);

    CompileConfig config;
    CompileFile(s, "./net/test_net_server_client.lua", config);

    int64_t ret = 0;
    Call(s, JIT_TCC, "NetTest.test_send_in_callback", ret);
    EXPECT_EQ(ret, 1);

    FakeluaDeleteState(s);
}

TEST(test_net, circular_buffer_wrap_regions_and_zero_capacity) {
    using namespace fakelua::net;
    CircularBuffer buffer(8);
    EXPECT_TRUE(buffer.Empty());
    EXPECT_EQ(buffer.ReadableRegion().second, 0u);
    EXPECT_EQ(buffer.Write("abcdef", 6), 6u);
    char bytes[8]{};
    ASSERT_EQ(buffer.Read(bytes, 3), 3u);
    EXPECT_EQ(std::string(bytes, 3), "abc");
    ASSERT_EQ(buffer.Write("GHIJK", 5), 5u);
    EXPECT_TRUE(buffer.Full());
    EXPECT_EQ(buffer.WritableRegion().second, 0u);
    EXPECT_EQ(buffer.Write("x", 1), 0u);
    ASSERT_EQ(buffer.Peek(bytes, sizeof(bytes)), 8u);
    EXPECT_EQ(std::string(bytes, 8), "defGHIJK");
    EXPECT_EQ(buffer.Size(), 8u);
    ASSERT_EQ(buffer.Read(bytes, sizeof(bytes)), 8u);
    EXPECT_EQ(std::string(bytes, 8), "defGHIJK");
    EXPECT_EQ(buffer.Read(bytes, sizeof(bytes)), 0u);
    auto writable = buffer.WritableRegion();
    ASSERT_GE(writable.second, 5u);
    std::memcpy(writable.first, "12345", 5);
    buffer.CommitWrite(5);
    auto readable = buffer.ReadableRegion();
    ASSERT_EQ(readable.second, 5u);
    EXPECT_EQ(std::string(readable.first, readable.second), "12345");
    buffer.CommitRead(readable.second);
    EXPECT_TRUE(buffer.Empty());
    buffer.Clear();
    EXPECT_EQ(buffer.WritableRegion().second, buffer.Capacity());
    EXPECT_EQ(buffer.Peek(bytes, 1), 0u);
    CircularBuffer zero(0);
    EXPECT_EQ(zero.Write("x", 1), 0u);
    EXPECT_EQ(zero.Read(bytes, 1), 0u);
    EXPECT_EQ(zero.Peek(bytes, 1), 0u);
    EXPECT_EQ(zero.WritableRegion().second, 0u);
    EXPECT_EQ(zero.ReadableRegion().second, 0u);
}

TEST(test_net, packet_framers_partial_input_and_binary_roundtrip) {
    using namespace fakelua::net;
    const std::string payload("a\0b\0c", 5);
    for (auto framer:
         {FramerType::Header4BigEndian, FramerType::Header4LittleEndian, FramerType::Header2BigEndian, FramerType::Header2LittleEndian, FramerType::LineDelimiter, FramerType::FixedLength}) {
        SCOPED_TRACE(static_cast<int>(framer));
        NetConfig config;
        config.framer = framer;
        config.fixed_packet_len = payload.size();
        config.max_packet_len = 64;
        CircularBuffer output(64), input(64);
        ASSERT_TRUE(WritePacket(output, config, payload.data(), payload.size()));
        std::string encoded(output.Size(), '\0');
        ASSERT_EQ(output.Read(encoded.data(), encoded.size()), encoded.size());
        const char *data = nullptr;
        uint32_t length = 0;
        bool error = true;
        EXPECT_FALSE(TryParsePacket(input, config, data, length, error));
        EXPECT_FALSE(error);
        ASSERT_EQ(input.Write(encoded.data(), encoded.size() - 1), encoded.size() - 1);
        EXPECT_FALSE(TryParsePacket(input, config, data, length, error));
        EXPECT_FALSE(error);
        ASSERT_EQ(input.Write(encoded.data() + encoded.size() - 1, 1), 1u);
        ASSERT_TRUE(TryParsePacket(input, config, data, length, error));
        EXPECT_FALSE(error);
        EXPECT_EQ(std::string(data, length), payload);
        EXPECT_TRUE(input.Empty());
    }
}

TEST(test_net, packet_header_rejects_lengths_larger_than_limit_or_capacity) {
    using namespace fakelua::net;
    for (auto framer: {FramerType::Header4BigEndian, FramerType::Header4LittleEndian, FramerType::Header2BigEndian, FramerType::Header2LittleEndian}) {
        NetConfig config;
        config.framer = framer;
        config.max_packet_len = 64;
        CircularBuffer output(64);
        ASSERT_TRUE(WritePacket(output, config, "12345678", 8));
        const size_t header = (framer == FramerType::Header4BigEndian || framer == FramerType::Header4LittleEndian) ? 4 : 2;
        char bytes[4]{};
        ASSERT_EQ(output.Read(bytes, header), header);
        for (const auto capacity: {size_t{4}, size_t{64}}) {
            CircularBuffer input(capacity);
            config.max_packet_len = capacity == 64 ? 3 : 64;
            input.Write(bytes, header);
            const char *data = nullptr;
            uint32_t length = 0;
            bool error = false;
            EXPECT_FALSE(TryParsePacket(input, config, data, length, error));
            EXPECT_TRUE(error);
            EXPECT_EQ(input.Size(), header);
        }
    }
}

TEST(test_net, packet_line_fixed_and_raw_limits) {
    using namespace fakelua::net;
    NetConfig config;
    config.framer = FramerType::LineDelimiter;
    config.max_packet_len = 3;
    const char *data = nullptr;
    uint32_t length = 0;
    bool error = false;
    CircularBuffer input(64);
    input.Write("a\r\n12345678", 11);
    ASSERT_TRUE(TryParsePacket(input, config, data, length, error));
    EXPECT_EQ(std::string(data, length), "a");
    EXPECT_FALSE(TryParsePacket(input, config, data, length, error));
    EXPECT_TRUE(error);
    input.Clear();
    config.framer = FramerType::FixedLength;
    config.fixed_packet_len = 0;
    EXPECT_FALSE(TryParsePacket(input, config, data, length, error));
    EXPECT_FALSE(error);
    config.fixed_packet_len = 65;
    EXPECT_FALSE(TryParsePacket(input, config, data, length, error));
    EXPECT_TRUE(error);
    config.fixed_packet_len = 4;
    config.max_packet_len = 64;
    ASSERT_TRUE(WritePacket(input, config, "xy", 2));
    ASSERT_TRUE(TryParsePacket(input, config, data, length, error));
    EXPECT_EQ(std::string(data, length), std::string("xy\0\0", 4));
    ASSERT_TRUE(WritePacket(input, config, "abcdef", 6));
    ASSERT_TRUE(TryParsePacket(input, config, data, length, error));
    EXPECT_EQ(std::string(data, length), "abcd");
    config.framer = FramerType::RawStream;
    config.max_packet_len = 2;
    input.Write("123", 3);
    ASSERT_TRUE(TryParsePacket(input, config, data, length, error));
    EXPECT_EQ(std::string(data, length), "12");
    ASSERT_TRUE(TryParsePacket(input, config, data, length, error));
    EXPECT_EQ(std::string(data, length), "3");
    EXPECT_FALSE(TryParsePacket(input, config, data, length, error));
}

TEST(test_net, custom_packet_encoder_atomicity_and_parser_progress) {
    using namespace fakelua::net;
    NetConfig config;
    config.framer = FramerType::Custom;
    config.custom_encoder_fn = [](CircularBuffer &buffer, const char *data, size_t length) {
        buffer.Write("[", 1);
        buffer.Write(data, length);
        buffer.Write("]", 1);
    };
    CircularBuffer too_small(3), enough(8);
    EXPECT_FALSE(WritePacket(too_small, config, "abc", 3));
    EXPECT_TRUE(too_small.Empty());
    EXPECT_TRUE(WritePacket(enough, config, "abc", 3));
    char bytes[8]{};
    ASSERT_EQ(enough.Read(bytes, sizeof(bytes)), 5u);
    EXPECT_EQ(std::string(bytes, 5), "[abc]");
    config.custom_parser_fn = [](CircularBuffer &, const char *&data, uint32_t &length) {
        data = "x";
        length = 1;
        return true;
    };
    enough.Write("x", 1);
    const char *data = nullptr;
    uint32_t length = 0;
    bool error = false;
    EXPECT_FALSE(TryParsePacket(enough, config, data, length, error));
    EXPECT_TRUE(error);
    EXPECT_EQ(enough.Size(), 1u);
    config.custom_parser_fn = [](CircularBuffer &buffer, const char *&data, uint32_t &length) {
        buffer.Skip(1);
        data = "x";
        length = 1;
        return true;
    };
    EXPECT_TRUE(TryParsePacket(enough, config, data, length, error));
    EXPECT_FALSE(error);
    EXPECT_EQ(std::string(data, length), "x");
    EXPECT_TRUE(enough.Empty());
}

TEST(test_net, native_udp_callbacks_and_explicit_destination) {
    for (auto backend: AllJitTypes()) {
        SCOPED_TRACE(JitTypeName(backend));
        FakeluaStateGuard state;
        int server_receives = 0;
        int client_receives = 0;
        RegisterNativeFunction(state.GetState(), "cpp_udp_echo", 5, false, [&](State *s, CVar *args, int n) {
            EXPECT_EQ(n, 5);
            if (inter::CVarToString(args[0], "") != "recv") return inter::NativeToFakeluaNil(s);
            ++server_receives;
            EXPECT_EQ(inter::CVarToString(args[2], ""), std::string("a\0b", 3));
            EXPECT_EQ(inter::CVarToInteger(args[3]), 3);
            CVar response = inter::AllocMultiCVar(s, 2);
            inter::SetMultiCVarElement(response, 0, inter::NativeToFakeluaString(s, "echo"));
            inter::SetMultiCVarElement(response, 1, args[2]);
            return response;
        });
        RegisterNativeFunction(state.GetState(), "cpp_udp_receive", 5, false, [&](State *s, CVar *args, int) {
            if (inter::CVarToString(args[0], "") == "recv") {
                ++client_receives;
                EXPECT_EQ(inter::CVarToString(args[2], ""), std::string("a\0b", 3));
            }
            return inter::NativeToFakeluaNil(s);
        });
        CompileFile(state.GetState(), "./net/test_net_native_udp_callbacks.lua", {});
        int64_t result = 0;
        Call(state.GetState(), backend, "test_native_udp_callbacks", result);
        EXPECT_EQ(result, 1);
        EXPECT_EQ(server_receives, 1);
        EXPECT_EQ(client_receives, 1);
    }
}

TEST(test_net, configuration_ranges_tls_spellings_and_dispatch_contracts) {
    for (auto backend: AllJitTypes()) {
        SCOPED_TRACE(JitTypeName(backend));
        FakeluaStateGuard state;
        CompileFile(state.GetState(), "./net/test_net_configuration_contracts.lua", {});
        int64_t result = 0;
        Call(state.GetState(), backend, "test_net_configuration_contracts", result);
        EXPECT_EQ(result, 1);
    }
}

TEST(test_net, native_custom_parser_gcc_callback_and_dispatch_failure_recovery) {
    FakeluaStateGuard state;
    std::vector<std::string> received;
    RegisterNativeFunction(state.GetState(), "native_three_byte_parser", 1, false, [](State *s, CVar *args, int) {
        const std::string input = inter::CVarToString(args[0], "");
        if (input.size() < 3) return inter::NativeToFakeluaNil(s);
        return inter::NativeToFakeluaString(s, input.substr(0, 3));
    });
    RegisterNativeFunction(state.GetState(), "native_parser_host_check", 1, false, [](State *s, CVar *args, int) {
        if (inter::CVarToString(args[0], "") == "err") throw std::runtime_error("network dispatch failure");
        return inter::NativeToFakeluaNil(s);
    });
    RegisterNativeFunction(state.GetState(), "native_parser_client_event", 5, false, [&](State *s, CVar *args, int) {
        if (inter::CVarToString(args[0], "") == "recv") received.push_back(inter::CVarToString(args[2], ""));
        else {
            EXPECT_EQ(args[2].type_, static_cast<int>(VarType::Nil));
            EXPECT_EQ(args[3].type_, static_cast<int>(VarType::Nil));
        }
        return inter::NativeToFakeluaNil(s);
    });
    RegisterNativeFunction(state.GetState(), "native_parser_received_count", 0, false, [&](State *s, CVar *, int) { return inter::NativeToFakeluaInt(s, received.size()); });
    CompileConfig config;
    config.disable_jit[JIT_TCC] = true;
    CompileFile(state.GetState(), "./net/test_net_native_parser.lua", config);
    int64_t result = 0;
    Call(state.GetState(), JIT_GCC, "test_native_parser_and_dispatch_recovery", result);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(received, (std::vector<std::string>{"abc", "def", "ghi"}));
}

namespace {
bool PollNetPair(net::TcpServer &server, net::TcpClient &client, std::vector<net::ConnEvent> &server_events, std::vector<net::ConnEvent> &client_events, const std::function<bool()> &done) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    do {
        server.DrainEventsWith([&](const auto &event) { server_events.push_back(event); });
        client.DrainEventsWith([&](const auto &event) { client_events.push_back(event); });
        if (done()) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } while (std::chrono::steady_clock::now() < deadline);
    return false;
}

bool HasNetEvent(const std::vector<net::ConnEvent> &events, net::EventKind kind) {
    return std::any_of(events.begin(), events.end(), [&](const auto &event) { return event.kind == kind; });
}

const std::string kWebSocketRequest =
        "GET /socket HTTP/1.1\r\nHost: localhost\r\nUpgrade: websocket\r\nConnection: keep-alive, Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n";
}// namespace

TEST(test_net, websocket_rejects_invalid_upgrade_path_key_tokens_and_version) {
    const std::vector<std::pair<std::string, std::string>> replacements{{"/socket", "/wrong"},
                                                                        {"Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n", ""},
                                                                        {"Upgrade: websocket", "Upgrade: h2c"},
                                                                        {"Connection: keep-alive, Upgrade", "Connection: keep-alive, invalid"},
                                                                        {"Version: 13", "Version: 12"}};
    for (const auto &[from, to]: replacements) {
        SCOPED_TRACE(from);
        FakeluaStateGuard state;
        net::NetConfig server_config;
        server_config.port = 19972;
        server_config.framer = net::FramerType::WebSocket;
        server_config.ws_path = "/socket";
        net::TcpServer server(server_config, state.GetState());
        server.Start();
        ASSERT_TRUE(server.Running());
        auto client_config = server_config;
        client_config.framer = net::FramerType::RawStream;
        net::TcpClient client(client_config, state.GetState());
        EXPECT_FALSE(client.Connected());
        client.Connect();
        std::vector<net::ConnEvent> server_events, client_events;
        ASSERT_TRUE(PollNetPair(server, client, server_events, client_events, [&] { return client.Connected(); }));
        std::string request = kWebSocketRequest;
        request.replace(request.find(from), from.size(), to);
        ASSERT_TRUE(client.Send(request.data(), request.size()));
        ASSERT_TRUE(PollNetPair(server, client, server_events, client_events, [&] { return HasNetEvent(client_events, net::EventKind::Close); }));
        EXPECT_FALSE(HasNetEvent(server_events, net::EventKind::Connect));
        EXPECT_FALSE(client.Connected());
    }
}

TEST(test_net, websocket_fragmented_extended_frame_control_frame_and_close) {
    FakeluaStateGuard state;
    net::NetConfig server_config;
    server_config.port = 19973;
    server_config.framer = net::FramerType::WebSocket;
    server_config.ws_path = "/socket";
    net::TcpServer server(server_config, state.GetState());
    server.Start();
    ASSERT_TRUE(server.Running());
    auto client_config = server_config;
    client_config.framer = net::FramerType::RawStream;
    net::TcpClient client(client_config, state.GetState());
    client.Connect();
    std::vector<net::ConnEvent> server_events, client_events;
    ASSERT_TRUE(PollNetPair(server, client, server_events, client_events, [&] { return client.Connected(); }));
    ASSERT_TRUE(client.Send(kWebSocketRequest.data(), kWebSocketRequest.size()));
    ASSERT_TRUE(PollNetPair(server, client, server_events, client_events, [&] { return HasNetEvent(server_events, net::EventKind::Connect) && HasNetEvent(client_events, net::EventKind::Recv); }));
    std::string handshake;
    for (const auto &event: client_events)
        if (event.kind == net::EventKind::Recv) handshake += event.data;
    ASSERT_NE(handshake.find("HTTP/1.1 101"), std::string::npos);
    client_events.clear();
    server_events.clear();
    const std::string ping("\x89\x80\x01\x02\x03\x04", 6);
    const std::string payload(300, 'x');
    std::string frame("\x81\xfe\x01\x2c\x01\x02\x03\x04", 8);
    for (size_t i = 0; i < payload.size(); ++i) frame.push_back(payload[i] ^ static_cast<char>(i % 4 + 1));
    ASSERT_TRUE(client.Send(ping.data(), ping.size()));
    ASSERT_TRUE(client.Send(frame.data(), 2));
    // No complete data frame is available yet; polling must retain its prefix.
    for (int i = 0; i < 3; ++i) {
        server.DrainEventsWith([&](const auto &event) { server_events.push_back(event); });
        client.DrainEventsWith([&](const auto &event) { client_events.push_back(event); });
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    EXPECT_FALSE(HasNetEvent(server_events, net::EventKind::Recv));
    ASSERT_TRUE(client.Send(frame.data() + 2, frame.size() - 2));
    ASSERT_TRUE(PollNetPair(server, client, server_events, client_events, [&] { return HasNetEvent(server_events, net::EventKind::Recv); }));
    for (const auto &event: server_events)
        if (event.kind == net::EventKind::Recv) EXPECT_EQ(event.data, payload);
    ASSERT_TRUE(server.Send(0, payload.data(), payload.size()));
    ASSERT_TRUE(PollNetPair(server, client, server_events, client_events, [&] {
        size_t bytes = 0;
        for (const auto &event: client_events)
            if (event.kind == net::EventKind::Recv) bytes += event.data.size();
        return bytes >= payload.size() + 4;
    }));
    std::string response;
    for (const auto &event: client_events)
        if (event.kind == net::EventKind::Recv) response += event.data;
    EXPECT_EQ(response, std::string("\x81\x7e\x01\x2c", 4) + payload);
    std::string too_large(65536, 'x');
    EXPECT_FALSE(server.Send(0, too_large.data(), too_large.size()));
    const std::string close("\x88\x80\x01\x02\x03\x04", 6);
    ASSERT_TRUE(client.Send(close.data(), close.size()));
    ASSERT_TRUE(PollNetPair(server, client, server_events, client_events, [&] { return HasNetEvent(client_events, net::EventKind::Close); }));
    EXPECT_FALSE(client.Connected());
}

TEST(test_net, websocket_client_rejects_status_and_missing_accept_header) {
    for (const std::string response: {"HTTP/1.1 400 Bad Request\r\n\r\n", "HTTP/1.1 101 Switching Protocols\r\n\r\n"}) {
        FakeluaStateGuard state;
        net::NetConfig server_config;
        server_config.port = 19974;
        server_config.framer = net::FramerType::RawStream;
        net::TcpServer server(server_config, state.GetState());
        server.Start();
        ASSERT_TRUE(server.Running());
        auto client_config = server_config;
        client_config.framer = net::FramerType::WebSocket;
        net::TcpClient client(client_config, state.GetState());
        client.Connect();
        std::vector<net::ConnEvent> server_events, client_events;
        ASSERT_TRUE(PollNetPair(server, client, server_events, client_events, [&] { return HasNetEvent(server_events, net::EventKind::Recv); }));
        ASSERT_TRUE(server.Send(0, response.data(), response.size()));
        ASSERT_TRUE(PollNetPair(server, client, server_events, client_events, [&] { return HasNetEvent(client_events, net::EventKind::Close); }));
        EXPECT_FALSE(client.Connected());
        EXPECT_FALSE(HasNetEvent(client_events, net::EventKind::Connect));
    }
}

TEST(test_net, tls_client_reports_invalid_ca_without_starting_connection) {
    FakeluaStateGuard state;
    net::NetConfig config;
    config.tls = true;
    config.tls_ca = "./tls/nonexistent_validation_ca.pem";
    net::TcpClient client(config, state.GetState());
    client.Connect();
    int closed = 0;
    client.DrainEventsWith([&](const auto &event) {
        if (event.kind == net::EventKind::Close) ++closed;
    });
    EXPECT_EQ(closed, 1);
    EXPECT_FALSE(client.Connected());
}

TEST(test_net, explicit_group_destruction_unregisters_live_wrapper) {
    FakeluaStateGuard state;
    CompileString(state.GetState(), R"(function live_wrapper_factory() return net.server({ip = "127.0.0.1", port = 19979}) end)", {});
    CVar value = CallByNameArgs(state.GetState(), JIT_INTERP, "live_wrapper_factory", 0, nullptr);
    auto *object = NativeObject::Unwrap(value);
    ASSERT_NE(object, nullptr);
    const auto group = object->GetGroupId();
    EXPECT_EQ(state.GetState()->GetNativeObjectManager().DestroyGroup(group), 1u);
    EXPECT_EQ(state.GetState()->GetNativeObjectManager().DestroyGroup(group), 0u);
}

TEST(test_net, custom_parser_return_contracts_and_multivalue_close_with_gcc_callbacks) {
    FakeluaStateGuard state;
    int received = 0;
    RegisterNativeFunction(state.GetState(), "custom_contract_seen", 1, false, [&](State *s, CVar *args, int) {
        EXPECT_EQ(inter::CVarToString(args[0], ""), "abc");
        ++received;
        return inter::NativeToFakeluaNil(s);
    });
    RegisterNativeFunction(state.GetState(), "custom_contract_received", 0, false, [&](State *s, CVar *, int) { return inter::NativeToFakeluaInt(s, received); });
    CompileConfig config;
    config.disable_jit[JIT_TCC] = true;
    CompileFile(state.GetState(), "./net/test_net_native_parser.lua", config);
    int64_t result = 0;
    Call(state.GetState(), JIT_GCC, "test_custom_parser_return_contracts", result);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(received, 2);
}

TEST(test_net, callback_send_queue_limit_and_deferred_invalid_destination) {
    FakeluaStateGuard state;
    CVar server;
    bool dispatched = false;
    RegisterNativeFunction(state.GetState(), "cpp_queue_pressure", 5, false, [&](State *s, CVar *args, int) {
        if (inter::CVarToString(args[0], "") == "conn") {
            CVar parameters[] = {server, args[1]};
            CallByNameArgs(s, JIT_INTERP, "queue_limit_contract", 2, parameters);
            dispatched = true;
        }
        return inter::NativeToFakeluaNil(s);
    });
    RegisterNativeFunction(state.GetState(), "queue_pressure_done", 0, false, [&](State *s, CVar *, int) { return inter::NativeToFakeluaBool(s, dispatched); });
    CompileString(state.GetState(), R"(
        function queue_limit_contract(server, connid)
            assert(server:send(999999, "invalid destination"))
            for i = 1, 4095 do assert(server:send(connid, "")) end
            assert(not server:send(connid, "queue is bounded"))
        end
        function queue_server_factory()
            local server = net.server({ip = "127.0.0.1", port = 19989, framer = "raw"})
            server:dispatch("cpp_queue_pressure")
            return server
        end
        function queue_client_contract()
            local client = net.client({ip = "127.0.0.1", port = 19989, framer = "raw"})
            for i = 1, 1000 do
                runtime.tick()
                if queue_pressure_done() then break end
                os.sleep(1)
            end
            client:close()
            assert(queue_pressure_done())
            return 1
        end
    )",
                  {});
    server = CallByNameArgs(state.GetState(), JIT_INTERP, "queue_server_factory", 0, nullptr);
    int64_t result = 0;
    Call(state.GetState(), JIT_INTERP, "queue_client_contract", result);
    EXPECT_EQ(result, 1);
    EXPECT_TRUE(dispatched);
}

TEST(test_net, connected_udp_callback_echo_directive) {
    FakeluaStateGuard state;
    bool received = false;
    RegisterNativeFunction(state.GetState(), "connected_udp_seen", 0, false, [&](State *s, CVar *, int) {
        received = true;
        return inter::NativeToFakeluaNil(s);
    });
    RegisterNativeFunction(state.GetState(), "connected_udp_done", 0, false, [&](State *s, CVar *, int) { return inter::NativeToFakeluaBool(s, received); });
    CompileString(state.GetState(), R"(
        function udp_server_directive(kind, connid, data, length, reason)
            if kind == "recv" then
                if data == "start" then return "echo", "server-reply" end
                assert(data == "client-echo")
                connected_udp_seen()
                return "close", nil
            end
            return nil
        end
        function udp_client_directive(kind, connid, data, length, reason)
            if kind == "recv" then
                assert(data == "server-reply")
                return "echo", "client-echo"
            end
            return nil
        end
        function connected_udp_directive_contract()
            local server = net.udp_server({ip = "127.0.0.1", port = 0})
            local client = net.udp_client({ip = "127.0.0.1", port = server:get_port()})
            server:dispatch("udp_server_directive")
            client:dispatch("udp_client_directive")
            assert(client:send("start"))
            for i = 1, 1000 do
                runtime.tick()
                if connected_udp_done() then break end
                os.sleep(1)
            end
            client:close()
            server:close()
            assert(connected_udp_done())
            return 1
        end
    )",
                  {});
    int64_t result = 0;
    Call(state.GetState(), JIT_INTERP, "connected_udp_directive_contract", result);
    EXPECT_EQ(result, 1);
    EXPECT_TRUE(received);
}
