#include "fakelua.h"
#include "jit/vm.h"
#include "native/http/native_http.h"
#include "native/net/net_event.h"
#include "native/object/native_object.h"
#include "native/table/native_table.h"
#include "state/state.h"
#include "test_jit.h"
#include "gtest/gtest.h"
#include <chrono>
#include <thread>

using namespace fakelua;

TEST(test_http, echo) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./http/test_http.lua", config);
    int64_t ret = 0;
    CallAll(s, "HttpTest.test_echo", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_http, connect_fail) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./http/test_http.lua", config);
    int64_t ret = 0;
    CallAll(s, "HttpTest.test_connect_fail", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_http, tls_echo) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./http/test_http.lua", config);
    int64_t ret = 0;
    CallAll(s, "HttpTest.test_tls_echo", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_http, crlf_reject) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./http/test_http.lua", config);
    int64_t ret = 0;
    CallAll(s, "HttpTest.test_crlf_reject", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_http, bad_port) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./http/test_http.lua", config);
    int64_t ret = 0;
    CallAll(s, "HttpTest.test_bad_port", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_http, timeout_ms_range) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./http/test_http.lua", config);
    int64_t ret = 0;
    CallAll(s, "HttpTest.test_timeout_ms_range", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_http, method_inject) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./http/test_http.lua", config);
    int64_t ret = 0;
    CallAll(s, "HttpTest.test_method_inject", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_http, client_bad_port) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./http/test_http.lua", config);
    int64_t ret = 0;
    CallAll(s, "HttpTest.test_client_bad_port", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_http, chunked_response_and_request_framing_contracts) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./http/test_http_chunked_framing.lua", {});
    for (auto backend: AllJitTypes()) {
        int64_t result = 0;
        Call(state.GetState(), backend, "test_http_chunked_framing", result);
        EXPECT_EQ(result, 1) << JitTypeName(backend);
    }
}

TEST(test_http, configuration_validation_and_unsupported_urls) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./http/test_http_chunked_framing.lua", {});
    for (auto backend: AllJitTypes()) {
        int64_t result = 0;
        Call(state.GetState(), backend, "test_http_configuration_contracts", result);
        EXPECT_EQ(result, 1) << JitTypeName(backend);
    }
}

TEST(test_http, deferred_reply_validation_and_gcc_callback_fallback) {
    FakeluaStateGuard state;
    int64_t connection_id = 0;
    RegisterNativeFunction(state.GetState(), "remember_http_connection", 1, false, [&](State *s, CVar *args, int) {
        connection_id = inter::CVarToInteger(args[0]);
        return inter::NativeToFakeluaNil(s);
    });
    RegisterNativeFunction(state.GetState(), "last_http_connection", 0, false, [&](State *s, CVar *, int) { return inter::NativeToFakeluaLonglong(s, connection_id); });
    CompileConfig config;
    config.disable_jit[JIT_TCC] = true;
    CompileFile(state.GetState(), "./http/test_http_chunked_framing.lua", config);
    int64_t result = 0;
    Call(state.GetState(), JIT_GCC, "test_http_deferred_reply", result);
    EXPECT_EQ(result, 1);
    EXPECT_GT(connection_id, 0);
}

TEST(test_http, close_delimited_response_waits_for_eof_and_preserves_body) {
    FakeluaStateGuard state;
    auto *s = state.GetState();
    bool done = false;
    std::string body, error;
    RegisterNativeFunction(s, "record_close_delimited_http", 3, false, [&](State *st, CVar *args, int) {
        done = true;
        error = inter::CVarToString(args[1], "");
        body = inter::CVarToString(table::TableHelper::GetTableStrId(st, args[2], "body"), "");
        return inter::NativeToFakeluaNil(st);
    });
    CompileConfig config;
    config.disable_jit[JIT_TCC] = true;
    CompileString(s, R"(
        function close_delimited_http_done(req, err, response) record_close_delimited_http(req, err, response) end
        function start_close_delimited_http() return http.get("http://127.0.0.1:19975/", "close_delimited_http_done") end
    )",
                  config);
    net::NetConfig server_config;
    server_config.port = 19975;
    server_config.framer = net::FramerType::RawStream;
    net::TcpServer server(server_config, s);
    server.Start();
    ASSERT_TRUE(server.Running());
    (void) CallByNameArgs(s, JIT_GCC, "start_close_delimited_http", 0, nullptr);
    bool request_seen = false;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!request_seen && std::chrono::steady_clock::now() < deadline) {
        server.DrainEventsWith([&](const auto &event) {
            if (event.kind == net::EventKind::Recv) request_seen = true;
        });
        http::TickAll(s);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    ASSERT_TRUE(request_seen);
    const std::string response = "HTTP/1.0 200 OK\r\nX-Test: retained\r\n\r\nclose-delimited body";
    ASSERT_TRUE(server.Send(0, response.data(), response.size()));
    // Drain the nonblocking event base to write the queued response before the
    // graceful close; without EOF the HTTP client must not finish this body.
    server.DrainEventsWith([](const auto &) {});
    http::TickAll(s);
    EXPECT_FALSE(done);
    ASSERT_TRUE(server.CloseConnection(0));
    while (!done && std::chrono::steady_clock::now() < deadline) {
        http::TickAll(s);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    ASSERT_TRUE(done);
    EXPECT_TRUE(error.empty()) << error;
    EXPECT_EQ(body, "close-delimited body");
}

TEST(test_http, bodyless_requests_scalar_reply_and_unknown_status_reason) {
    FakeluaStateGuard state;
    auto *s = state.GetState();
    CompileConfig config;
    config.disable_jit[JIT_TCC] = true;
    CompileString(s, R"(
        function bodyless_http_handler(kind, connid, request)
            assert(kind == "request" and request.method == "GET" and request.body == "")
            if request.path == "/scalar" then return "scalar body" end
            return {status = 599, body = "custom body"}
        end
        function start_bodyless_http_server()
            local server = http.server({ip = "127.0.0.1", port = 19976})
            server:dispatch("bodyless_http_handler")
            return 1
        end
    )",
                  config);
    int64_t started = 0;
    Call(s, JIT_GCC, "start_bodyless_http_server", started);
    ASSERT_EQ(started, 1);
    for (const auto &[path, expected]: std::vector<std::pair<std::string, std::string>>{{"/scalar", "scalar body"}, {"/custom", "custom body"}}) {
        net::NetConfig client_config;
        client_config.port = 19976;
        client_config.framer = net::FramerType::RawStream;
        net::TcpClient client(client_config, s);
        client.Connect();
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        auto poll = [&](std::string &response) {
            http::TickAll(s);
            client.DrainEventsWith([&](const auto &event) {
                if (event.kind == net::EventKind::Recv) response += event.data;
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        };
        std::string response;
        while (!client.Connected() && std::chrono::steady_clock::now() < deadline) poll(response);
        ASSERT_TRUE(client.Connected());
        const std::string request = "GET " + path + " HTTP/1.1\r\nHost: localhost\r\n\r\n";
        ASSERT_TRUE(client.Send(request.data(), request.size()));
        while (response.find(expected) == std::string::npos && std::chrono::steady_clock::now() < deadline) poll(response);
        EXPECT_NE(response.find(expected), std::string::npos);
        EXPECT_NE(response.find(path == "/scalar" ? "200 OK" : "599 OK"), std::string::npos);
    }
}

TEST(test_http, explicit_group_destruction_unregisters_live_wrapper) {
    FakeluaStateGuard state;
    CompileString(state.GetState(), R"(function live_wrapper_factory() return http.server({ip = "127.0.0.1", port = 0}) end)", {});
    CVar value = CallByNameArgs(state.GetState(), JIT_INTERP, "live_wrapper_factory", 0, nullptr);
    auto *object = NativeObject::Unwrap(value);
    ASSERT_NE(object, nullptr);
    const auto group = object->GetGroupId();
    EXPECT_EQ(state.GetState()->GetNativeObjectManager().DestroyGroup(group), 1u);
    EXPECT_EQ(state.GetState()->GetNativeObjectManager().DestroyGroup(group), 0u);
}
