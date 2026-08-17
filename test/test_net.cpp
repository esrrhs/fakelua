#include "fakelua.h"
#include "gtest/gtest.h"
#include "native/net/net_internal.h"
#include "var/var_type.h"
#include "var/var_string.h"

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#include <atomic>
#include <chrono>
#include <thread>
#include <iostream>

using namespace fakelua;
using namespace fakelua::net;

// ─────────────────────────────────────────────────────────────────────────────
// C++ Native 网络引擎底层测试
// ─────────────────────────────────────────────────────────────────────────────

// 测试 1: 纯阻塞 TCP 验证基础网络
TEST(test_net, test_minimal_tcp) {
    net_init();

    socket_t listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(listen_fd, INVALID_SOCKET_VAL);

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(19933);
    addr.sin_addr.s_addr = INADDR_ANY;
    ASSERT_EQ(bind(listen_fd, (sockaddr *)&addr, sizeof(addr)), 0);
    ASSERT_EQ(listen(listen_fd, 5), 0);

    socket_t client_fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(client_fd, INVALID_SOCKET_VAL);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(19933);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    std::thread server_thread([&]() {
        sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        socket_t accepted = accept(listen_fd, (sockaddr *)&client_addr, &len);
        if (accepted == INVALID_SOCKET_VAL) return;
        char buf[256];
        int n = (int)::recv(accepted, buf, sizeof(buf), 0);
        std::cerr << "server recv " << n << " bytes: [" << std::string(buf, std::max(0, n)) << "]" << std::endl;
        ::close(accepted);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int ret = ::connect(client_fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr));
    std::cerr << "client connect ret=" << ret << std::endl;

    const char *msg = "hello";
    int sent = (int)::send(client_fd, msg, 5, 0);
    std::cerr << "client sent " << sent << " bytes" << std::endl;

    server_thread.join();

    ::close(client_fd);
    ::close(listen_fd);
    net_shutdown();
}

// 测试 2: TcpServer + TcpClient 非阻塞 echo（纯 C++ 引擎层）
TEST(test_net, test_echo_cpp) {
    net_init();

    std::atomic<int> conn_count{0};
    std::atomic<int> recv_count{0};
    std::string last_data;

    NetConfig srv_cfg{};
    srv_cfg.port = 19955;
    srv_cfg.max_conn = 10;
    TcpServer server(srv_cfg);
    server.start();
    ASSERT_TRUE(server.running());

    NetConfig cli_cfg{};
    cli_cfg.ip = "127.0.0.1";
    cli_cfg.port = 19955;
    TcpClient client(cli_cfg);
    client.connect();

    // 连接建立
    for (int i = 0; i < 50; ++i) {
        server.tick(
            [&conn_count](int) { conn_count++; },
            [&recv_count, &last_data](int, const char *data, size_t len) {
                recv_count++;
                last_data.assign(data, len);
            },
            [](int) {});
        client.tick(
            [](const char *, size_t) {},
            []() {});
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    std::cout << "conn=" << conn_count << " connected=" << client.connected() << std::endl;
    ASSERT_GE(conn_count, 1);

    // client 发送数据
    bool sent = client.send("hello", 5);
    ASSERT_TRUE(sent);

    // 等数据到达
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 驱动收发
    for (int i = 0; i < 100; ++i) {
        server.tick(
            [&conn_count](int) { conn_count++; },
            [&recv_count, &last_data](int, const char *data, size_t len) {
                recv_count++;
                last_data.assign(data, len);
            },
            [](int) {});
        client.tick(
            [](const char *, size_t) {},
            []() {});
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    std::cout << "final: recv=" << recv_count << " data=[" << last_data << "]" << std::endl;

    ASSERT_GE(recv_count, 1);
    ASSERT_EQ(last_data, "hello");

    client.disconnect();
    server.stop();
    net_shutdown();
}

// 测试 2b: 多 TcpServer + 多 TcpClient（纯 C++ 引擎层）
TEST(test_net, test_multi_server_client_cpp) {
    net_init();

    // 服务器 A: port 19956
    std::string recv_a, recv_b;
    int conn_a = 0, conn_b = 0;

    NetConfig srv_a_cfg{};
    srv_a_cfg.port = 19956;
    srv_a_cfg.max_conn = 10;
    TcpServer server_a(srv_a_cfg);
    server_a.start();
    ASSERT_TRUE(server_a.running());

    // 服务器 B: port 19957
    NetConfig srv_b_cfg{};
    srv_b_cfg.port = 19957;
    srv_b_cfg.max_conn = 10;
    TcpServer server_b(srv_b_cfg);
    server_b.start();
    ASSERT_TRUE(server_b.running());

    // client_a → server_a (19956)
    NetConfig cli_a_cfg{};
    cli_a_cfg.ip = "127.0.0.1";
    cli_a_cfg.port = 19956;
    TcpClient client_a(cli_a_cfg);
    client_a.connect();

    // client_b → server_b (19957)
    NetConfig cli_b_cfg{};
    cli_b_cfg.ip = "127.0.0.1";
    cli_b_cfg.port = 19957;
    TcpClient client_b(cli_b_cfg);
    client_b.connect();

    // 驱动连接建立
    for (int i = 0; i < 50; ++i) {
        server_a.tick([&conn_a](int) { conn_a++; }, [](int, const char *, size_t) {}, [](int) {});
        server_b.tick([&conn_b](int) { conn_b++; }, [](int, const char *, size_t) {}, [](int) {});
        client_a.tick([](const char *, size_t) {}, []() {});
        client_b.tick([](const char *, size_t) {}, []() {});
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    ASSERT_GE(conn_a, 1) << "server_a should accept connection";
    ASSERT_GE(conn_b, 1) << "server_b should accept connection";

    // 各自发送
    client_a.send("hello_a", 7);
    client_b.send("hello_b", 7);

    // 驱动收发
    for (int i = 0; i < 100; ++i) {
        server_a.tick([](int) {}, [&recv_a](int, const char *d, size_t l) { recv_a.assign(d, l); }, [](int) {});
        server_b.tick([](int) {}, [&recv_b](int, const char *d, size_t l) { recv_b.assign(d, l); }, [](int) {});
        client_a.tick([](const char *, size_t) {}, []() {});
        client_b.tick([](const char *, size_t) {}, []() {});
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    std::cout << "server_a recv=[" << recv_a << "] server_b recv=[" << recv_b << "]" << std::endl;
    EXPECT_EQ(recv_a, "hello_a") << "server_a should receive hello_a from client_a";
    EXPECT_EQ(recv_b, "hello_b") << "server_b should receive hello_b from client_b";

    client_a.disconnect();
    client_b.disconnect();
    server_a.stop();
    server_b.stop();
    net_shutdown();
}



// ─────────────────────────────────────────────────────────────────────────────
// Lua 绑定的网络层接口测试
// ─────────────────────────────────────────────────────────────────────────────

// 测试 3: server 创建/销毁
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

// 测试 4: client 创建/销毁
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

// 测试 5: echo 收发（核心：验证 C++ → Lua 按函数名回调 + 纯函数返回指令）
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

// 测试 6: 多包收发
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

// 测试 7: 多 server + 多 client 同时运行，相互独立不干扰
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

// 测试 8: 1个 server + 多个 client 同时连接，server 能区分处理各连接
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
