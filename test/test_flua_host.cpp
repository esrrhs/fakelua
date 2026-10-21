#include "gtest/gtest.h"

// fork / pipe / poll 只在 POSIX 上有。Windows CI 没有 poll.h。
#if !defined(_WIN32)

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <poll.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#if defined(__APPLE__)
#include <cstdint>
#include <cstdlib>
#include <mach-o/dyld.h>
#endif

namespace {

std::string FluaBinary() {
#if defined(__linux__)
    char buf[4096];
    const ssize_t n = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n <= 0) {
        return "./flua";
    }
    buf[n] = '\0';
    return (std::filesystem::path(buf).parent_path() / "flua").string();
#elif defined(__APPLE__)
    char buf[4096];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) != 0) {
        return "./flua";
    }
    char resolved[4096];
    if (::realpath(buf, resolved) == nullptr) {
        return "./flua";
    }
    return (std::filesystem::path(resolved).parent_path() / "flua").string();
#else
    return "./flua";
#endif
}

struct FluaResult {
    int code = -1;
    std::string out;
    std::string err;
};

FluaResult RunFlua(const std::string &bin, const std::vector<std::string> &args) {
    int out_pipe[2] = {-1, -1};
    int err_pipe[2] = {-1, -1};
    EXPECT_EQ(::pipe(out_pipe), 0);
    EXPECT_EQ(::pipe(err_pipe), 0);

    const pid_t pid = ::fork();
    EXPECT_GE(pid, 0);
    if (pid == 0) {
        ::dup2(out_pipe[1], STDOUT_FILENO);
        ::dup2(err_pipe[1], STDERR_FILENO);
        ::close(out_pipe[0]);
        ::close(out_pipe[1]);
        ::close(err_pipe[0]);
        ::close(err_pipe[1]);
        std::vector<char *> argv;
        argv.push_back(const_cast<char *>(bin.c_str()));
        for (const auto &arg: args) {
            argv.push_back(const_cast<char *>(arg.c_str()));
        }
        argv.push_back(nullptr);
        ::execv(bin.c_str(), argv.data());
        std::perror("execv flua");
        _exit(127);
    }

    ::close(out_pipe[1]);
    ::close(err_pipe[1]);
    FluaResult result;
    bool out_open = true;
    bool err_open = true;
    while (out_open || err_open) {
        pollfd fds[2];
        nfds_t nfds = 0;
        int out_i = -1;
        int err_i = -1;
        if (out_open) {
            out_i = static_cast<int>(nfds);
            fds[nfds++] = pollfd{out_pipe[0], POLLIN, 0};
        }
        if (err_open) {
            err_i = static_cast<int>(nfds);
            fds[nfds++] = pollfd{err_pipe[0], POLLIN, 0};
        }
        if (::poll(fds, nfds, -1) < 0) {
            if (errno == EINTR) continue;
            break;
        }
        auto drain = [&](int idx, int fd, std::string &dst, bool &open) {
            if (idx < 0 || (fds[idx].revents & (POLLIN | POLLHUP | POLLERR)) == 0) return;
            char buf[512];
            const ssize_t n = ::read(fd, buf, sizeof(buf));
            if (n > 0) {
                dst.append(buf, static_cast<size_t>(n));
            } else {
                open = false;
                ::close(fd);
            }
        };
        drain(out_i, out_pipe[0], result.out, out_open);
        drain(err_i, err_pipe[0], result.err, err_open);
    }
    int status = 0;
    ::waitpid(pid, &status, 0);
    result.code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    return result;
}

std::filesystem::path WriteScript(const std::filesystem::path &dir, const char *name, const char *body) {
    std::filesystem::create_directories(dir);
    const auto path = dir / name;
    std::ofstream out(path);
    out << body;
    return path;
}

} // namespace

TEST(flua_host, set_exit_code_after_frames) {
    const auto flua = FluaBinary();
    ASSERT_TRUE(std::filesystem::exists(flua)) << flua;

    const auto dir = std::filesystem::temp_directory_path() / "fakelua_flua_host";
    const auto script = WriteScript(dir, "set_exit.lua", R"(package "main"
local st = container.map()
function init()
    return 0
end
function update()
    local n = (st:get("n") or 0) + 1
    st:set("n", n)
    if n >= 2 then
        flua.set_exit(4)
    end
    return 0
end
function fini()
    if st:get("n") ~= 2 then
        error("expected 2 updates, got " .. tostring(st:get("n")))
    end
    return 0
end
)");

    const auto result = RunFlua(flua, {"--jit_type=2", script.string()});
    EXPECT_EQ(result.code, 4) << result.err;
}

TEST(flua_host, missing_return_is_a_host_error) {
    const auto flua = FluaBinary();
    ASSERT_TRUE(std::filesystem::exists(flua)) << flua;
    const auto dir = std::filesystem::temp_directory_path() / "fakelua_flua_host";
    const auto script = WriteScript(dir, "missing_return.lua", R"(package "main"
function init()
end
function update()
    return 0
end
function fini()
    return 0
end
)");
    const auto result = RunFlua(flua, {"--jit_type=2", script.string()});
    EXPECT_EQ(result.code, 1) << result.out << result.err;
    EXPECT_NE(result.err.find("FakeluaToNativeInt failed"), std::string::npos) << result.err;
}

TEST(flua_host, update_return_wins_over_set_exit) {
    const auto flua = FluaBinary();
    ASSERT_TRUE(std::filesystem::exists(flua)) << flua;
    const auto dir = std::filesystem::temp_directory_path() / "fakelua_flua_host";
    const auto script = WriteScript(dir, "update_wins.lua", R"(package "main"
function init()
    return 0
end
function update()
    flua.set_exit(3)
    return 5
end
function fini()
    return 0
end
)");
    const auto result = RunFlua(flua, {"--jit_type=2", script.string()});
    EXPECT_EQ(result.code, 5) << result.err;
}

TEST(flua_host, set_exit_during_init_skips_update) {
    const auto flua = FluaBinary();
    ASSERT_TRUE(std::filesystem::exists(flua)) << flua;
    const auto dir = std::filesystem::temp_directory_path() / "fakelua_flua_host";
    const auto script = WriteScript(dir, "init_exit.lua", R"(package "main"
function init()
    flua.set_exit(9)
    return 0
end
function update()
    error("update must not run")
    return 0
end
function fini()
    return 0
end
)");
    const auto result = RunFlua(flua, {"--jit_type=2", script.string()});
    EXPECT_EQ(result.code, 9) << result.err;
    EXPECT_EQ(result.err.find("update must not run"), std::string::npos) << result.err;
}

TEST(flua_host, empty_directory_exits) {
    const auto flua = FluaBinary();
    ASSERT_TRUE(std::filesystem::exists(flua)) << flua;
    const auto dir = std::filesystem::temp_directory_path() / "fakelua_flua_empty";
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    const auto result = RunFlua(flua, {"--jit_type=2", dir.string()});
    EXPECT_EQ(result.code, 1);
    EXPECT_NE(result.err.find("no .lua file"), std::string::npos) << result.err;
    EXPECT_EQ(result.err.find("main.init"), std::string::npos) << result.err;
}

#endif
