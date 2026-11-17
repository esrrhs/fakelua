#include "fakelua.h"
#include "test_jit.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(test_process, echo) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./process/test_process.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProcessTest.test_echo", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_process, stdin_cat) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./process/test_process.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProcessTest.test_stdin", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_process, timeout) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./process/test_process.lua", config);
    int64_t ret = 0;
    CallAll(s, "ProcessTest.test_timeout", ret);
    EXPECT_EQ(ret, 1);
    FakeluaDeleteState(s);
}

TEST(test_process, missing_exe) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./process/test_process.lua", config);
    int64_t ret = 0;
    CallThrow(s, "ProcessTest.test_missing", ret);
    FakeluaDeleteState(s);
}

TEST(test_process, output_exact_8mb) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./process/test_process.lua", config);
    int64_t ret = 0;
    for (const auto jit_type: ExceptionJitTypes()) {
        SCOPED_TRACE(::testing::Message() << "jit=" << JitTypeName(jit_type));
        try {
            Call(s, jit_type, "ProcessTest.test_output_exact_8mb", ret);
            EXPECT_EQ(ret, 1);
        } catch (const std::exception &e) {
            const std::string msg = e.what();
            EXPECT_EQ(msg.find("output exceeds 8MB limit"), std::string::npos) << msg;
        }
    }
    FakeluaDeleteState(s);
}

TEST(test_process, output_over_8mb) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    CompileFile(s, "./process/test_process.lua", config);
    int64_t ret = 0;
    CallThrow(s, "ProcessTest.test_output_over_8mb", ret);
    FakeluaDeleteState(s);
}

TEST(test_process, argument_validation_environment_override_and_working_directory) {
    FakeluaStateGuard state;
    CompileFile(state.GetState(), "./process/test_process_options.lua", {});
    for (auto backend: AllJitTypes()) {
        SCOPED_TRACE(JitTypeName(backend));
        int64_t result = 0;
        Call(state.GetState(), backend, "test_process_options", result);
        EXPECT_EQ(result, 1);
    }
}


#if !defined(_WIN32)
TEST(test_process, completed_and_signaled_children_with_and_without_deadlines) {
    FakeluaStateGuard state;
    CompileString(state.GetState(), R"(
        function child_exit_contracts()
            local output, errors, code = process.run({"sh", "-c", "exit 7"}, {timeout_ms = 1000})
            assert(output == "" and errors == "" and code == 7)
            output, errors, code = process.run({"sh", "-c", "kill -TERM $$"}, {timeout_ms = 1000})
            assert(output == "" and errors == "" and code == 143)
            output, errors, code = process.run({"sh", "-c", "sleep 0.05; kill -TERM $$"}, {timeout_ms = 0})
            assert(output == "" and errors == "" and code == 143)
            return 1
        end
    )",
                  {});
    int64_t result = 0;
    CallAll(state.GetState(), "child_exit_contracts", result);
    EXPECT_EQ(result, 1);
}
#endif
