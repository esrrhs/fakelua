#include "native/process/native_process.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "util/utf8_io.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/readable_pipe.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/writable_pipe.hpp>
#include <boost/nowide/convert.hpp>
#include <boost/process.hpp>
#include <boost/process/environment.hpp>
#include <boost/process/execute.hpp>
#include <boost/process/start_dir.hpp>
#include <boost/process/stdio.hpp>

#include <chrono>
#include <format>
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

    asio::io_context ctx;
    asio::readable_pipe out_pipe{ctx};
    asio::readable_pipe err_pipe{ctx};
    std::unique_ptr<asio::writable_pipe> in_pipe;
    if (!stdin_data.empty()) {
        in_pipe = std::make_unique<asio::writable_pipe>(ctx);
    }

    bp::process_stdio stdio;
    if (in_pipe) {
        stdio.in = *in_pipe;
    } else {
        stdio.in = nullptr;
    }
    stdio.out = out_pipe;
    stdio.err = err_pipe;

    bp::process proc = [&]() {
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
            ThrowFakeluaException(std::format("process.run: failed to spawn '{}': {}", argv[0], e.what()));
        }
    }();

    std::string stdout_s;
    std::string stderr_s;
    int exit_code = -1;
    bool timed_out = false;

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

    ctx.run();

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
