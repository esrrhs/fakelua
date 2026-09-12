#include "native/process/native_process.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "util/utf8_io.h"

#include <boost/asio/detail/config.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#if defined(BOOST_ASIO_HAS_PIPE)
#include <boost/asio/buffer.hpp>
#include <boost/asio/readable_pipe.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/writable_pipe.hpp>
#include <boost/process/stdio.hpp>
#endif
#include <boost/filesystem.hpp>
#include <boost/nowide/convert.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/process/environment.hpp>
#include <boost/process/process.hpp>
#include <boost/process/start_dir.hpp>

#include <chrono>
#include <format>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
extern char **environ;
#endif

namespace fakelua::process {

namespace bp = boost::process::v2;
namespace asio = boost::asio;

using table::TableHelper;

static constexpr size_t kMaxOutput = 8 * 1024 * 1024;

static boost::filesystem::path MakeProcTempPath() {
    return boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("fakelua-proc-%%%%-%%%%.tmp");
}

static std::string ReadCappedFile(const boost::filesystem::path &p) {
    std::string out;
    boost::nowide::ifstream in(p.string(), std::ios::binary);
    if (!in) {
        return out;
    }
    char buf[4096];
    while (in && out.size() < kMaxOutput) {
        in.read(buf, sizeof(buf));
        const auto n = static_cast<size_t>(in.gcount());
        if (n == 0) {
            break;
        }
        const auto room = kMaxOutput - out.size();
        out.append(buf, n < room ? n : room);
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
    for (char **e = environ; e && *e; ++e) {
        out.emplace_back(*e);
    }
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

#if defined(_WIN32) && !defined(BOOST_ASIO_HAS_PIPE)
// 工程在 WIN32 上全局 BOOST_ASIO_DISABLE_IOCP，Asio 没有 pipe 类型，
// boost/process/v2/stdio.hpp 仍会实例化 basic_readable_pipe 而编不过。
struct FileStdio {
    HANDLE hin = INVALID_HANDLE_VALUE;
    HANDLE hout = INVALID_HANDLE_VALUE;
    HANDLE herr = INVALID_HANDLE_VALUE;

    FileStdio() = default;
    FileStdio(const FileStdio &) = delete;
    FileStdio &operator=(const FileStdio &) = delete;

    ~FileStdio() {
        Close();
    }

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

    boost::system::error_code on_setup(bp::windows::default_launcher &launcher, const bp::filesystem::path &, const std::wstring &) {
        launcher.startup_info.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
        launcher.startup_info.StartupInfo.hStdInput = hin;
        launcher.startup_info.StartupInfo.hStdOutput = hout;
        launcher.startup_info.StartupInfo.hStdError = herr;
        launcher.inherited_handles.push_back(hin);
        launcher.inherited_handles.push_back(hout);
        launcher.inherited_handles.push_back(herr);
        return {};
    }
};
#endif

template<typename Stdio>
static bp::process LaunchProcess(asio::io_context &ctx, const auto &exe, const std::vector<std::string> &child_args, Stdio &stdio,
                                 const std::string &cwd, const std::unordered_map<std::string, std::string> &env_overrides, const std::string &argv0) {
    try {
        if (!cwd.empty() && !env_overrides.empty()) {
            auto env_list = CurrentEnviron();
            ApplyEnvOverrides(env_list, env_overrides);
            return bp::process(ctx, exe, child_args, stdio, bp::process_start_dir(cwd), bp::process_environment(env_list));
        }
        if (!cwd.empty()) {
            return bp::process(ctx, exe, child_args, stdio, bp::process_start_dir(cwd));
        }
        if (!env_overrides.empty()) {
            auto env_list = CurrentEnviron();
            ApplyEnvOverrides(env_list, env_overrides);
            return bp::process(ctx, exe, child_args, stdio, bp::process_environment(env_list));
        }
        return bp::process(ctx, exe, child_args, stdio);
    } catch (const std::exception &e) {
        ThrowFakeluaException(std::format("process.run: failed to spawn '{}': {}", argv0, e.what()));
    }
}

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

    auto exe = bp::environment::find_executable(argv[0]);
    if (exe.empty()) {
        exe = argv[0];
    }
    std::vector<std::string> child_args(argv.begin() + 1, argv.end());

    // concurrency_hint=1：与 State::IoContext 相同，不给 Asio 再开一条 timer 线程。
    asio::io_context ctx{1};
    std::string stdout_s;
    std::string stderr_s;
    int exit_code = -1;
    bool timed_out = false;

#if defined(BOOST_ASIO_HAS_PIPE)
    asio::readable_pipe out_pipe{ctx};
    asio::readable_pipe err_pipe{ctx};
    std::unique_ptr<asio::writable_pipe> in_pipe;
    bp::process_stdio stdio;
    if (!stdin_data.empty()) {
        in_pipe = std::make_unique<asio::writable_pipe>(ctx);
        stdio.in = *in_pipe;
    } else {
        stdio.in = nullptr;
    }
    stdio.out = out_pipe;
    stdio.err = err_pipe;
    bp::process proc = LaunchProcess(ctx, exe, child_args, stdio, cwd, env_overrides, argv[0]);

    struct PipeReader {
        asio::readable_pipe *p;
        std::string *dst;
        char tmp[4096]{};
        PipeReader(asio::readable_pipe *pipe, std::string *out) : p(pipe), dst(out) {}
        void start() {
            if (!p || !dst) {
                return;
            }
            p->async_read_some(asio::buffer(tmp), [this](const boost::system::error_code &ec, std::size_t n) {
                if (n > 0 && dst->size() < kMaxOutput) {
                    dst->append(tmp, n);
                }
                if (!ec && dst->size() < kMaxOutput) start();
            });
        }
    };
    PipeReader out_reader{&out_pipe, &stdout_s};
    PipeReader err_reader{&err_pipe, &stderr_s};
    out_reader.start();
    err_reader.start();
    if (in_pipe) {
        asio::async_write(*in_pipe, asio::buffer(stdin_data), [in = in_pipe.get()](const boost::system::error_code &, std::size_t) {
            boost::system::error_code ec;
            in->close(ec);
        });
    }
    asio::steady_timer timer(ctx);
    asio::steady_timer drain(ctx);
    auto cancel_reads = [&]() {
        boost::system::error_code ec;
        out_pipe.cancel(ec);
        err_pipe.cancel(ec);
    };
    if (timeout_ms > 0) {
        timer.expires_after(std::chrono::milliseconds(timeout_ms));
        timer.async_wait([&](const boost::system::error_code &ec) {
            if (ec) return;
            timed_out = true;
            boost::system::error_code tec;
            proc.terminate(tec);
            drain.expires_after(std::chrono::milliseconds(10));
            drain.async_wait([&](const boost::system::error_code &) { cancel_reads(); });
        });
    }
    proc.async_wait([&](const boost::system::error_code &, int code) {
        exit_code = code;
        timer.cancel();
        drain.expires_after(std::chrono::milliseconds(10));
        drain.async_wait([&](const boost::system::error_code &) { cancel_reads(); });
    });
    // POSIX-only (BOOST_ASIO_HAS_PIPE). Windows never compiles this branch:
    // DISABLE_IOCP removes pipes and object_handle wait, so process.run uses
    // Win32 WaitForSingleObject instead of ctx.run().
    ctx.run();
#else
    const auto out_path = MakeProcTempPath();
    const auto err_path = MakeProcTempPath();
    boost::filesystem::path in_path;
    {
        boost::nowide::ofstream{out_path.string(), std::ios::binary};
        boost::nowide::ofstream{err_path.string(), std::ios::binary};
    }
    FileStdio stdio;
    if (!stdin_data.empty()) {
        in_path = MakeProcTempPath();
        boost::nowide::ofstream in(in_path.string(), std::ios::binary);
        in.write(stdin_data.data(), static_cast<std::streamsize>(stdin_data.size()));
        in.close();
        stdio.hin = FileStdio::Open(in_path.wstring().c_str(), true);
    } else {
        stdio.hin = FileStdio::Open(L"NUL", true);
    }
    stdio.hout = FileStdio::Open(out_path.wstring().c_str(), false);
    stdio.herr = FileStdio::Open(err_path.wstring().c_str(), false);
    bp::process proc = LaunchProcess(ctx, exe, child_args, stdio, cwd, env_overrides, argv[0]);
    // Parent must drop the inheritable stdio handles so the child can see EOF.
    stdio.Close();
    // BOOST_ASIO_DISABLE_IOCP makes windows::object_handle::wait() hang: it waits
    // on an IOCP completion that never arrives. Do not call process::wait/terminate
    // (both go through that path). Wait and reap with Win32, then detach.
    HANDLE ph = proc.native_handle();
    DWORD wait_ms = INFINITE;
    if (timeout_ms > 0) {
        wait_ms = timeout_ms >= static_cast<int64_t>(INFINITE) ? INFINITE - 1 : static_cast<DWORD>(timeout_ms);
    }
    DWORD wr = WAIT_FAILED;
    if (ph != nullptr && ph != INVALID_HANDLE_VALUE) {
        wr = ::WaitForSingleObject(ph, wait_ms);
    }
    if (wr == WAIT_TIMEOUT) {
        timed_out = true;
        ::TerminateProcess(ph, 9);
        ::WaitForSingleObject(ph, INFINITE);
    }
    DWORD code = 0;
    if (ph != nullptr && ph != INVALID_HANDLE_VALUE && ::GetExitCodeProcess(ph, &code) && code != STILL_ACTIVE) {
        exit_code = static_cast<int>(code);
    } else if (timed_out) {
        exit_code = 9;
    }
    proc.detach();
    ctx.stop();
    stdout_s = ReadCappedFile(out_path);
    stderr_s = ReadCappedFile(err_path);
    boost::system::error_code rec;
    boost::filesystem::remove(out_path, rec);
    boost::filesystem::remove(err_path, rec);
    if (!in_path.empty()) {
        boost::filesystem::remove(in_path, rec);
    }
#endif

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
