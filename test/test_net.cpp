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
#if defined(_WIN32)
        closesocket(accepted);
#else
        ::close(accepted);
#endif
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int ret = ::connect(client_fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr));
    std::cerr << "client connect ret=" << ret << std::endl;

    const char *msg = "hello";
    int sent = (int)::send(client_fd, msg, 5, 0);
    std::cerr << "client sent " << sent << " bytes" << std::endl;

    server_thread.join();

#if defined(_WIN32)
    closesocket(client_fd);
    closesocket(listen_fd);
#else
    ::close(client_fd);
    ::close(listen_fd);
#endif
    net_shutdown();
}

// 测试 2: TcpServer + TcpClient 非阻塞 echo
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

    // client 发送数据
    bool sent = client.send("hello", 5);
    std::cout << "client.send returned " << sent << std::endl;

    // 等数据到达
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

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

    std::cout << "final: conn=" << conn_count << " recv=" << recv_count << " data=[" << last_data << "]" << std::endl;

    client.disconnect();
    server.stop();
    net_shutdown();

    EXPECT_EQ(conn_count, 1);
    EXPECT_EQ(recv_count, 1);
    EXPECT_EQ(last_data, "hello");
}

// ─────────────────────────────────────────────────────────────────────────────
// Lua 绑定的网络层接口测试
// ─────────────────────────────────────────────────────────────────────────────

// 测试 3: server 创建/销毁
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

// 测试 4: client 创建/销毁
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

// 测试 5: echo 收发（核心：验证 C++ → Lua 按函数名回调机制）
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

    int64_t count = 0;
    std::string p1, p2, p3;
    Call(s, JIT_TCC, "NetMulti.test_multi", std::tie(count, p1, p2, p3));

    EXPECT_EQ(count, 3) << "should receive 3 packets";
    EXPECT_EQ(p1, "packet1");
    EXPECT_EQ(p2, "packet2");
    EXPECT_EQ(p3, "packet3");

    FakeluaDeleteState(s);
}
