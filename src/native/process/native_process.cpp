#include "native/process/native_process.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "util/utf8_io.h"

#include <boost/filesystem.hpp>
#include <boost/nowide/convert.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/system/error_code.hpp>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <format>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
extern char **environ;
#endif

namespace fakelua::process {

using table::TableHelper;

static constexpr size_t kMaxOutput = 8 * 1024 * 1024;

static boost::filesystem::path MakeProcTempPath() {
    return boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("fakelua-proc-%%%%-%%%%.tmp");
}

static std::string ReadCappedFile(const boost::filesystem::path &p) {
    std::string out;
    boost::nowide::ifstream in(p.string(), std::ios::binary);
    if (!in) return out;
    char buf[4096];
    while (in && out.size() < kMaxOutput) {
        in.read(buf, sizeof(buf));
        const auto n = static_cast<size_t>(in.gcount());
        if (n == 0) break;
        const auto room = kMaxOutput - out.size();
        out.append(buf, n < room ? n : room);
    }
    if (out.size() >= kMaxOutput) {
        char extra = 0;
        if (in.read(&extra, 1) && in.gcount() > 0) {
            ThrowFakeluaException("process.run: output exceeds 8MB limit");
        }
    }
    return out;
}

static std::string CVarToStringLocal(CVar v) {
    return inter::FakeluaToNativeString(nullptr, v);
}

static std::vector<std::string> CurrentEnviron() {
    std::vector<std::string> out;
#if defined(_WIN32)
    if (wchar_t *block = GetEnvironmentStringsW()) {
        for (wchar_t *p = block; *p; p += wcslen(p) + 1) {
            out.push_back(boost::nowide::narrow(p));
        }
        FreeEnvironmentStringsW(block);
    }
#else
    for (char **e = environ; e && *e; ++e) out.emplace_back(*e);
#endif
    return out;
}

static void ApplyEnvOverrides(std::vector<std::string> &env, const std::unordered_map<std::string, std::string> &overrides) {
    for (const auto &[key, val]: overrides) {
        std::string prefix = key + "=";
        bool found = false;
        for (auto &entry: env) {
            if (entry.size() >= prefix.size() && entry.compare(0, prefix.size(), prefix) == 0) {
                entry = prefix + val;
                found = true;
                break;
            }
        }
        if (!found) env.push_back(prefix + val);
    }
}

static std::vector<std::string> TableToArgv(State *s, CVar tbl) {
    std::vector<std::string> argv;
    int64_t len = TableHelper::GetTableLen(tbl);
    argv.reserve(static_cast<size_t>(len));
    for (int64_t i = 1; i <= len; ++i) {
        CVar item = TableHelper::GetTableInt(s, tbl, i);
        CheckStringArg(item, 1, "process.run");
        argv.push_back(CVarToStringLocal(item));
    }
    return argv;
}

static bool IsExecutablePath(const boost::filesystem::path &p) {
    boost::system::error_code ec;
    if (!boost::filesystem::exists(p, ec) || boost::filesystem::is_directory(p, ec)) return false;
#if defined(_WIN32)
    return true;
#else
    return ::access(p.string().c_str(), X_OK) == 0;
#endif
}

static std::string FindExecutable(const std::string &name) {
    boost::filesystem::path p(name);
    if (p.has_parent_path()) return name;
    const char *path_env = std::getenv("PATH");
    if (!path_env) return name;
    std::string paths = path_env;
#if defined(_WIN32)
    const char sep = ';';
    const char *pathext = std::getenv("PATHEXT");
    std::vector<std::string> exts;
    if (pathext) {
        std::string e = pathext;
        size_t i = 0;
        while (i < e.size()) {
            auto n = e.find(';', i);
            exts.push_back(e.substr(i, n == std::string::npos ? n : n - i));
            if (n == std::string::npos) break;
            i = n + 1;
        }
    }
    if (exts.empty()) exts = {".EXE", ".BAT", ".CMD"};
#else
    const char sep = ':';
#endif
    size_t i = 0;
    while (i <= paths.size()) {
        auto n = paths.find(sep, i);
        std::string dir = paths.substr(i, n == std::string::npos ? std::string::npos : n - i);
        if (!dir.empty()) {
            boost::filesystem::path cand = boost::filesystem::path(dir) / name;
            if (IsExecutablePath(cand)) return cand.string();
#if defined(_WIN32)
            for (const auto &ext: exts) {
                auto c2 = cand;
                c2 += ext;
                if (IsExecutablePath(c2)) return c2.string();
            }
#endif
        }
        if (n == std::string::npos) break;
        i = n + 1;
    }
    return name;
}

#if defined(_WIN32)
struct FileStdio {
    HANDLE hin = INVALID_HANDLE_VALUE;
    HANDLE hout = INVALID_HANDLE_VALUE;
    HANDLE herr = INVALID_HANDLE_VALUE;

    FileStdio() = default;
    FileStdio(const FileStdio &) = delete;
    FileStdio &operator=(const FileStdio &) = delete;
    ~FileStdio() { Close(); }

    void Close() {
        auto close_one = [](HANDLE &h) {
            if (h != INVALID_HANDLE_VALUE) {
                ::CloseHandle(h);
                h = INVALID_HANDLE_VALUE;
            }
        };
        close_one(hin);
        close_one(hout);
        close_one(herr);
    }

    static HANDLE Open(const wchar_t *path, bool for_read) {
        SECURITY_ATTRIBUTES sa{};
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE;
        HANDLE h = ::CreateFileW(path, for_read ? GENERIC_READ : GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa,
                                   for_read ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) {
            ThrowFakeluaException(std::format("process.run: CreateFileW failed ({})", static_cast<unsigned>(::GetLastError())));
        }
        return h;
    }
};
#endif

static CVar ProcessRun(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "process.run", "argv table expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    if (a0.type_ != static_cast<int>(VarType::Table)) {
        ThrowBadArgument(1, "process.run", "argv table expected");
    }
    auto argv = TableToArgv(s, a0);
    if (argv.empty() || argv[0].empty()) {
        ThrowFakeluaException("process.run: argv must be a non-empty table");
    }

    std::string stdin_data;
    std::string cwd;
    int64_t timeout_ms = 0;
    std::unordered_map<std::string, std::string> env_overrides;

    if (n >= 2) {
        CVar opts = inter::GetNativeArg(s, args, n, 1);
        if (opts.type_ != static_cast<int>(VarType::Nil)) {
            if (opts.type_ != static_cast<int>(VarType::Table)) {
                ThrowBadArgument(2, "process.run", "options table expected");
            }
            CVar stdin_v = TableHelper::GetTableStrId(s, opts, "stdin");
            if (stdin_v.type_ != static_cast<int>(VarType::Nil)) {
                CheckStringArg(stdin_v, 2, "process.run");
                stdin_data = CVarToStringLocal(stdin_v);
            }
            CVar cwd_v = TableHelper::GetTableStrId(s, opts, "cwd");
            if (cwd_v.type_ != static_cast<int>(VarType::Nil)) {
                CheckStringArg(cwd_v, 2, "process.run");
                cwd = CVarToStringLocal(cwd_v);
            }
            CVar to_v = TableHelper::GetTableStrId(s, opts, "timeout_ms");
            if (to_v.type_ != static_cast<int>(VarType::Nil)) {
                timeout_ms = CheckIntegerArg(to_v, 2, "process.run");
                if (timeout_ms < 0) timeout_ms = 0;
            }
            CVar env_v = TableHelper::GetTableStrId(s, opts, "env");
            if (env_v.type_ != static_cast<int>(VarType::Nil)) {
                if (env_v.type_ != static_cast<int>(VarType::Table)) {
                    ThrowFakeluaException("process.run: env must be a table");
                }
                TableHelper::ForEachKV(env_v, [&](CVar key, CVar val) {
                    if (key.type_ == static_cast<int>(VarType::Nil)) return;
                    CheckStringArg(key, 2, "process.run");
                    CheckStringArg(val, 2, "process.run");
                    env_overrides[CVarToStringLocal(key)] = CVarToStringLocal(val);
                });
            }
        }
    }

    std::string exe = FindExecutable(argv[0]);
    if (!IsExecutablePath(boost::filesystem::path(exe))) {
        ThrowFakeluaException(std::format("process.run: failed to spawn '{}': No such file or directory", argv[0]));
    }
    std::string stdout_s;
    std::string stderr_s;
    int exit_code = -1;
    bool timed_out = false;

    const auto out_path = MakeProcTempPath();
    const auto err_path = MakeProcTempPath();
    boost::filesystem::path in_path;
    {
        boost::nowide::ofstream{out_path.string(), std::ios::binary};
        boost::nowide::ofstream{err_path.string(), std::ios::binary};
    }
    if (!stdin_data.empty()) {
        in_path = MakeProcTempPath();
        boost::nowide::ofstream in(in_path.string(), std::ios::binary);
        in.write(stdin_data.data(), static_cast<std::streamsize>(stdin_data.size()));
        in.close();
    }

#if defined(_WIN32)
    auto QuoteWinArg = [](const std::wstring &arg) -> std::wstring {
        std::wstring out = L"\"";
        int backslashes = 0;
        for (wchar_t c: arg) {
            if (c == L'\\') {
                ++backslashes;
            } else if (c == L'"') {
                out.append(static_cast<size_t>(backslashes * 2 + 1), L'\\');
                out.push_back(L'"');
                backslashes = 0;
            } else {
                if (backslashes) out.append(static_cast<size_t>(backslashes), L'\\');
                backslashes = 0;
                out.push_back(c);
            }
        }
        if (backslashes) out.append(static_cast<size_t>(backslashes * 2), L'\\');
        out.push_back(L'"');
        return out;
    };

    FileStdio stdio;
    stdio.hin = FileStdio::Open(in_path.empty() ? L"NUL" : in_path.wstring().c_str(), true);
    stdio.hout = FileStdio::Open(out_path.wstring().c_str(), false);
    stdio.herr = FileStdio::Open(err_path.wstring().c_str(), false);

    std::wstring cmd = QuoteWinArg(boost::nowide::widen(exe));
    for (size_t i = 1; i < argv.size(); ++i) {
        cmd += L" " + QuoteWinArg(boost::nowide::widen(argv[i]));
    }
    std::vector<wchar_t> cmd_buf(cmd.begin(), cmd.end());
    cmd_buf.push_back(0);

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = stdio.hin;
    si.hStdOutput = stdio.hout;
    si.hStdError = stdio.herr;
    PROCESS_INFORMATION pi{};
    DWORD flags = CREATE_UNICODE_ENVIRONMENT;
    std::wstring env_block;
    std::unique_ptr<wchar_t[]> env_ptr;
    if (!env_overrides.empty()) {
        auto env_list = CurrentEnviron();
        ApplyEnvOverrides(env_list, env_overrides);
        std::wstring block;
        for (auto &e: env_list) {
            auto w = boost::nowide::widen(e);
            block.append(w);
            block.push_back(0);
        }
        block.push_back(0);
        env_block = std::move(block);
        env_ptr.reset();
    }
    std::wstring cwd_w = cwd.empty() ? std::wstring() : boost::nowide::widen(cwd);
    BOOL ok = ::CreateProcessW(boost::nowide::widen(exe).c_str(), cmd_buf.data(), nullptr, nullptr, TRUE, flags,
                                env_block.empty() ? nullptr : env_block.data(),
                                cwd_w.empty() ? nullptr : cwd_w.c_str(), &si, &pi);
    stdio.Close();
    if (!ok) {
        ThrowFakeluaException(std::format("process.run: failed to spawn '{}': {}", argv[0], static_cast<unsigned>(::GetLastError())));
    }
    DWORD wait_ms = INFINITE;
    if (timeout_ms > 0) {
        wait_ms = timeout_ms >= static_cast<int64_t>(INFINITE) ? INFINITE - 1 : static_cast<DWORD>(timeout_ms);
    }
    DWORD wr = ::WaitForSingleObject(pi.hProcess, wait_ms);
    if (wr == WAIT_TIMEOUT) {
        timed_out = true;
        ::TerminateProcess(pi.hProcess, 9);
        ::WaitForSingleObject(pi.hProcess, INFINITE);
    }
    DWORD code = 0;
    if (::GetExitCodeProcess(pi.hProcess, &code) && code != STILL_ACTIVE) exit_code = static_cast<int>(code);
    else if (timed_out) exit_code = 9;
    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);
#else
    int in_fd = ::open(in_path.empty() ? "/dev/null" : in_path.string().c_str(), O_RDONLY);
    int out_fd = ::open(out_path.string().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    int err_fd = ::open(err_path.string().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (in_fd < 0 || out_fd < 0 || err_fd < 0) {
        if (in_fd >= 0) ::close(in_fd);
        if (out_fd >= 0) ::close(out_fd);
        if (err_fd >= 0) ::close(err_fd);
        ThrowFakeluaException("process.run: failed to open stdio files");
    }
    pid_t pid = ::fork();
    if (pid < 0) {
        ::close(in_fd);
        ::close(out_fd);
        ::close(err_fd);
        ThrowFakeluaException(std::format("process.run: failed to spawn '{}': {}", argv[0], strerror(errno)));
    }
    if (pid == 0) {
        if (!cwd.empty() && ::chdir(cwd.c_str()) != 0) _exit(127);
        dup2(in_fd, STDIN_FILENO);
        dup2(out_fd, STDOUT_FILENO);
        dup2(err_fd, STDERR_FILENO);
        if (in_fd > 2) ::close(in_fd);
        if (out_fd > 2) ::close(out_fd);
        if (err_fd > 2) ::close(err_fd);
        std::vector<char *> cargv;
        for (auto &a: argv) cargv.push_back(a.data());
        cargv.push_back(nullptr);
        auto env_list = CurrentEnviron();
        ApplyEnvOverrides(env_list, env_overrides);
        std::vector<char *> cenv;
        for (auto &e: env_list) cenv.push_back(e.data());
        cenv.push_back(nullptr);
        execve(exe.c_str(), cargv.data(), cenv.data());
        _exit(127);
    }
    ::close(in_fd);
    ::close(out_fd);
    ::close(err_fd);
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms > 0 ? timeout_ms : 0);
    for (;;) {
        int status = 0;
        pid_t r = ::waitpid(pid, &status, WNOHANG);
        if (r == pid) {
            if (WIFEXITED(status)) exit_code = WEXITSTATUS(status);
            else if (WIFSIGNALED(status)) exit_code = 128 + WTERMSIG(status);
            else exit_code = -1;
            break;
        }
        if (r < 0 && errno != EINTR) {
            exit_code = -1;
            break;
        }
        if (timeout_ms > 0 && std::chrono::steady_clock::now() >= deadline) {
            timed_out = true;
            ::kill(pid, SIGKILL);
            ::waitpid(pid, &status, 0);
            exit_code = 9;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (timeout_ms == 0) {
            r = ::waitpid(pid, &status, 0);
            if (r == pid) {
                if (WIFEXITED(status)) exit_code = WEXITSTATUS(status);
                else if (WIFSIGNALED(status)) exit_code = 128 + WTERMSIG(status);
            }
            break;
        }
    }
#endif

    stdout_s = ReadCappedFile(out_path);
    stderr_s = ReadCappedFile(err_path);
    boost::system::error_code rec;
    boost::filesystem::remove(out_path, rec);
    boost::filesystem::remove(err_path, rec);
    if (!in_path.empty()) boost::filesystem::remove(in_path, rec);

    if (timed_out && exit_code < 0) exit_code = 9;

    CVar multi = inter::AllocMultiCVar(s, 3);
    inter::SetMultiCVarElement(multi, 0, inter::NativeToFakeluaString(s, stdout_s));
    inter::SetMultiCVarElement(multi, 1, inter::NativeToFakeluaString(s, stderr_s));
    inter::SetMultiCVarElement(multi, 2, inter::NativeToFakeluaInt(s, exit_code));
    return multi;
}

void RegisterProcessLibraryApi(State *s) {
    if (!s) return;
    utf8_io::Init();
    RegisterNativeFunction(s, "process.run", 1, true, ProcessRun);
}

}// namespace fakelua::process
