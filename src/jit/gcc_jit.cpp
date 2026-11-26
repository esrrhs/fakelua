#include "jit/gcc_jit.h"
#include "jit/code_binding.h"
#include "jit/gcc_cache.h"
#include "jit/gcc_handle.h"
#include "state/state.h"
#include "util/file_util.h"
#include "util/logging.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <sstream>
#include <vector>

#if defined(_WIN32)
#define NOMINMAX
#include <process.h>
#include <windows.h>
#else
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace fakelua {

namespace {
constexpr size_t kCExtLen = 2;// 文件扩展名 ".c" 的长度
constexpr int kExecFailedStatus = 127;

std::string JoinCommand(const std::vector<std::string> &args) {
    std::ostringstream oss;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i) {
            oss << ' ';
        }
        oss << args[i];
    }
    return oss.str();
}

#if defined(_WIN32)
std::string WinErrToString(const DWORD err) {
    if (err == 0) {
        return "unknown error";
    }
    LPSTR msg = nullptr;
    const DWORD len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                     reinterpret_cast<LPSTR>(&msg), 0, nullptr);
    if (len == 0 || !msg) {
        return std::format("error code {}", err);
    }
    std::string ret(msg, len);
    LocalFree(msg);
    return ret;
}

std::vector<std::string> GetWindowsDefaultLibraryPaths() {
    std::vector<std::string> ret;
    HMODULE module = nullptr;
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCSTR>(&GetWindowsDefaultLibraryPaths), &module) || !module) {
        return ret;
    }
    char module_path[MAX_PATH] = {0};
    if (const DWORD len = GetModuleFileNameA(module, module_path, MAX_PATH); len == 0 || len >= MAX_PATH) {
        return ret;
    }
    const std::filesystem::path dll_path(module_path);
    const auto dll_dir = dll_path.parent_path();
    ret.emplace_back(dll_dir.string());
    if (dll_dir.has_parent_path()) {
        ret.emplace_back((dll_dir.parent_path() / "lib").string());
    }
    return ret;
}
#endif
}// namespace

GccJitter::GccJitter(State *s) : s_(s) {
}

void GccJitter::Compile(const ParseResult &pr, const GenResult &gr, const CompileConfig &cfg) {
    LOG_INFO(s_, "engine", "GCC JIT compile start: {}, {} functions", pr.file_name, gr.function_names.size());

    struct Workspace {
        std::string path;
        bool transferred = false;
        bool created = false;

        ~Workspace() {
            if (created && !transferred && !path.empty()) {
                std::error_code ec;
                std::filesystem::remove_all(path, ec);
            }
        }
    } workspace;
    std::string c_file;
    std::string so_file;
    std::string log_file;
    std::string obj_file;
    bool c_written = false;

    auto ensure_workspace = [&]() {
        if (workspace.created || !c_file.empty()) return;
#if defined(USE_COV)
        // Coverage builds: put JIT temps under the CMake build tree so .gcno/.gcda are
        // visible to `lcov --directory build`. Keep random names to avoid collisions.
        {
            const std::filesystem::path cov_dir(FAKELUA_JIT_COV_DIR);
            std::error_code ec;
            std::filesystem::create_directories(cov_dir, ec);
            std::mt19937 rng{std::random_device{}()};
            std::uniform_int_distribution<int> dist(100000, 999999);
            do {
                c_file = (cov_dir / ("fakelua_jit_" + std::to_string(dist(rng)) + ".c")).string();
            } while (std::ifstream(c_file));
        }
#else
        // An exclusive private directory prevents temp-file collisions/symlink
        // races and ensures every dlopen/LoadLibrary sees a distinct image inode.
        workspace.path = GenerateTmpFilename("fakelua_jit_", ".dir");
#if defined(_WIN32)
        if (!std::filesystem::create_directory(workspace.path))
#else
        if (::mkdir(workspace.path.c_str(), 0700) != 0)
#endif
            ThrowFakeluaException("GCC compile failed, cannot create private workspace");
        workspace.created = true;
        c_file = (std::filesystem::path(workspace.path) / "module.c").string();
#endif
        so_file = c_file.substr(0, c_file.size() - kCExtLen) +
#if defined(_WIN32)
                  ".dll";
#elif defined(__APPLE__)
                  ".dylib";
#else
                  ".so";
#endif
        log_file = c_file.substr(0, c_file.size() - kCExtLen) + ".gcc.log";
        obj_file = c_file.substr(0, c_file.size() - kCExtLen) + ".o";
    };

    // RAII cleanup for temp files on early error paths (before GCCHandle takes ownership).
    const auto CleanupOnError = [&]() {
        std::error_code ec;
        if (!c_file.empty()) std::filesystem::remove(c_file, ec);
        if (!obj_file.empty()) std::filesystem::remove(obj_file, ec);
        if (!so_file.empty()) std::filesystem::remove(so_file, ec);
        if (!log_file.empty()) std::filesystem::remove(log_file, ec);
    };

    auto ensure_c_and_log = [&]() {
        ensure_workspace();
        if (!c_written) {
            if (std::ofstream ofs(c_file); !ofs.is_open()) {
                ThrowFakeluaException(std::format("GCC compile failed, cannot open c file {}", c_file));
            } else {
                ofs << gr.c_code;
                ofs.close();
            }
            c_written = true;
        }
        std::ofstream trunc(log_file, std::ios::trunc);
    };

    std::string compiler = "gcc";
    if (const char *env_cc = std::getenv("FAKELUA_CC"); env_cc && *env_cc) {
        compiler = env_cc;
    }
#if defined(__APPLE__)
    else {
        // macOS 上 Apple Clang 比外部 Homebrew gcc 快 2~3 倍，默认优先使用 clang
        compiler = "clang";
    }
#endif

    auto cache_config = s_->GetStateConfig().gcc_config;
#if defined(USE_COV)
    // Instrumented images own per-compilation gcno/gcda paths. Never reuse them;
    // keep the filesystem cache itself instrumented and directly testable.
    cache_config.enable_cache = false;
#endif
    GccCache cache(cache_config, compiler, gr.c_code, cfg.debug_mode);
    std::string load_path;
    bool direct_cache_image = false;
    bool cache_hit = cache.RestoreForLoad(so_file, load_path, direct_cache_image, [&]() {
        ensure_workspace();
    });
    std::string dep_file;

    struct DependencyCleanup {
        const std::string &path;

        ~DependencyCleanup() {
            if (!path.empty()) {
                std::error_code ec;
                std::filesystem::remove(path, ec);
            }
        }
    } dependency_cleanup{dep_file};

    const auto opt_flag = cfg.debug_mode ? "-O0" : "-O3";
    const std::string jit_type_def = "-DFAKELUA_JIT_TYPE=" + std::to_string(static_cast<int>(JIT_GCC));

    auto append_includes_and_libs = [&](std::vector<std::string> &a, bool for_link) {
        for (const auto &path: s_->GetStateConfig().gcc_config.include_paths) {
            a.emplace_back("-I" + path);
        }
        if (!for_link) {
            return;
        }
#if defined(_WIN32)
        for (const auto &path: GetWindowsDefaultLibraryPaths()) {
            a.emplace_back("-L" + path);
        }
        a.emplace_back("-lfakelua");
#endif
        for (const auto &path: s_->GetStateConfig().gcc_config.library_paths) {
            a.emplace_back("-L" + path);
        }
        for (const auto &lib: s_->GetStateConfig().gcc_config.libraries) {
            a.emplace_back("-l" + lib);
        }
    };

    // Helper: build null-terminated argv from std::vector<std::string>.
    auto build_argv = [](std::vector<std::string> &a) {
        std::vector<char *> argv;
        argv.reserve(a.size() + 1);
        for (auto &arg: a) {
            argv.emplace_back(arg.data());
        }
        argv.emplace_back(nullptr);
        return argv;
    };

    auto run_gcc = [&](std::vector<std::string> &a) {
#if defined(_WIN32)
        auto argv = build_argv(a);
        const int compile_status = _spawnvp(_P_WAIT, compiler.c_str(), argv.data());
        if (compile_status == -1) {
            CleanupOnError();
            ThrowFakeluaException(std::format("GCC compile failed for {}: cannot execute gcc (errno {}: {}). cmd: {}", pr.file_name, errno, std::strerror(errno), JoinCommand(a)));
        }
        if (compile_status != 0) {
            CleanupOnError();
            ThrowFakeluaException(std::format("GCC compile failed for {} with exit code {}. cmd: {}", pr.file_name, compile_status, JoinCommand(a)));
        }
#else
        auto argv = build_argv(a);
        const int log_fd = open(log_file.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (log_fd < 0) {
            CleanupOnError();
            ThrowFakeluaException(std::format("GCC compile failed, cannot open log file {}", log_file));
        }

        // Block SIGCHLD before fork to prevent the default handler from turning
        // the child into a zombie before we get to waitpid.
        sigset_t old_mask, block_mask;
        sigemptyset(&block_mask);
        sigaddset(&block_mask, SIGCHLD);
        pthread_sigmask(SIG_BLOCK, &block_mask, &old_mask);

        const pid_t pid = fork();
        if (pid < 0) {
            pthread_sigmask(SIG_SETMASK, &old_mask, nullptr);
            close(log_fd);
            CleanupOnError();
            ThrowFakeluaException(std::format("GCC compile failed, fork failed for {}", pr.file_name));
        }

        if (pid == 0) {
            // Restore default signal mask in child before exec.
            pthread_sigmask(SIG_SETMASK, &old_mask, nullptr);
            dup2(log_fd, STDOUT_FILENO);
            dup2(log_fd, STDERR_FILENO);
            close(log_fd);
            execvp(compiler.c_str(), argv.data());
            _exit(kExecFailedStatus);
        }
        close(log_fd);

        int status = 0;
        if (waitpid(pid, &status, 0) < 0) {
            pthread_sigmask(SIG_SETMASK, &old_mask, nullptr);
            CleanupOnError();
            ThrowFakeluaException(std::format("GCC compile failed, waitpid failed for {}", pr.file_name));
        }
        pthread_sigmask(SIG_SETMASK, &old_mask, nullptr);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            std::string gcc_log;
            if (std::ifstream ifs(log_file); ifs.is_open()) {
                gcc_log.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
            }
            CleanupOnError();
            ThrowFakeluaException(std::format("GCC compile failed for {} (log: {})\n{}", pr.file_name, log_file, gcc_log));
        }
#endif
    };

    auto compile_image = [&]() {
        ensure_c_and_log();
        dep_file = c_file + ".d";
#if defined(USE_COV)
        // Two-step compile so gcov notes/data are named `<stem>.gcno/.gcda` (not
        // `<stem>.so-<stem>.gcno` from one-shot -shared of a .c file).
        {
            std::vector<std::string> compile_args{compiler, "-x", "c", "-c"};
#if !defined(_WIN32)
            compile_args.emplace_back("-fPIC");
#endif
            compile_args.emplace_back(opt_flag);
            compile_args.emplace_back("-fprofile-arcs");
            compile_args.emplace_back("-ftest-coverage");
            compile_args.emplace_back("-fprofile-update=atomic");
            compile_args.emplace_back(jit_type_def);
            append_includes_and_libs(compile_args, false);
            compile_args.emplace_back("-o");
            compile_args.emplace_back(obj_file);
            compile_args.emplace_back(c_file);
            run_gcc(compile_args);
        }
        {
            std::vector<std::string> link_args{compiler, "-shared"};
#if !defined(_WIN32)
            link_args.emplace_back("-fPIC");
#endif
#if defined(__APPLE__)
            link_args.emplace_back("-undefined");
            link_args.emplace_back("dynamic_lookup");
            link_args.emplace_back("-Wl,-adhoc_codesign");
#endif
            link_args.emplace_back("--coverage");
            append_includes_and_libs(link_args, true);
            link_args.emplace_back("-o");
            link_args.emplace_back(so_file);
            link_args.emplace_back(obj_file);
            run_gcc(link_args);
        }
        {
            // Object file not needed after link; keep .c/.gcno for lcov.
            std::error_code ec;
            std::filesystem::remove(obj_file, ec);
        }
#else
        {
            std::vector<std::string> args{compiler, "-x", "c", "-shared"};
#if !defined(_WIN32)
            args.emplace_back("-fPIC");
#endif
#if defined(__APPLE__)
            // macOS 的 .dylib 需要 -undefined dynamic_lookup 选项，
            // 允许运行时由宿主进程提供未解析的符号。
            args.emplace_back("-undefined");
            args.emplace_back("dynamic_lookup");
            args.emplace_back("-Wl,-adhoc_codesign");
#endif
            args.emplace_back("-MD");
            args.emplace_back("-MF");
            args.emplace_back(dep_file);
            args.emplace_back("-MT");
            args.emplace_back("flua");
            args.emplace_back(opt_flag);
            args.emplace_back(jit_type_def);
            append_includes_and_libs(args, true);
            args.emplace_back("-o");
            args.emplace_back(so_file);
            args.emplace_back(c_file);
            run_gcc(args);
        }
#endif
    };
    bool published_before_load = false;
    if (!cache_hit) {
        compile_image();
        load_path = so_file;
#if !defined(_WIN32) && !defined(USE_COV)
        // Publish before dlopen on POSIX so the OS code-signing trust cache
        // (e.g. macOS syspolicyd) is warmed on the stable cache path on launch 1,
        // making launch 2 fast as well. If dlopen or symbol checks fail below,
        // cache.Invalidate() immediately removes the entry.
        cache.Publish(so_file, dep_file, c_file);
        published_before_load = true;
        if (auto direct_path = cache.TryClaimPublishedImage(); !direct_path.empty()) {
            load_path = std::move(direct_path);
            direct_cache_image = true;
        }
#endif
    }

#if defined(_WIN32)
    HMODULE module_handle = LoadLibraryA(load_path.c_str());
    if (!module_handle && cache_hit) {
        cache.Invalidate();
        cache_hit = false;
        direct_cache_image = false;
        compile_image();
        load_path = so_file;
        module_handle = LoadLibraryA(load_path.c_str());
    }
    if (!module_handle) {
        CleanupOnError();
        ThrowFakeluaException(std::format("GCC compile failed, LoadLibrary failed for {}: {}", load_path, WinErrToString(GetLastError())));
    }
    const auto handle = std::make_shared<GCCHandle>(s_, c_file, so_file, log_file, module_handle, workspace.path);
    workspace.transferred = true;

    // Common post-load: register compiled functions + call init.
    auto dlsym_lambda = [&](const std::string &sym) -> void * {
        FARPROC p = GetProcAddress(module_handle, sym.c_str());
        return p ? reinterpret_cast<void *>(p) : nullptr;
    };
#else
    void *dl_handle = dlopen(load_path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!dl_handle && cache_hit) {
        cache.Invalidate();
        cache_hit = false;
        direct_cache_image = false;
        compile_image();
        load_path = so_file;
        dl_handle = dlopen(load_path.c_str(), RTLD_NOW | RTLD_LOCAL);
    }
    if (!dl_handle && direct_cache_image && !so_file.empty()) {
        // Fallback if the published cache file was concurrently evicted before dlopen.
        direct_cache_image = false;
        load_path = so_file;
        dl_handle = dlopen(load_path.c_str(), RTLD_NOW | RTLD_LOCAL);
    }
    if (!dl_handle) {
        // 在高并发多进程测试场景下，macOS 内核 AMFI 代码签名检查偶现缓存同步微延迟，重试以确保平滑加载
        for (int retry = 0; retry < 5 && !dl_handle; ++retry) {
            usleep(2000 * (retry + 1));
            dl_handle = dlopen(load_path.c_str(), RTLD_NOW | RTLD_LOCAL);
        }
    }
    if (!dl_handle) {
        if (published_before_load) cache.Invalidate();
        CleanupOnError();
        ThrowFakeluaException(std::format("GCC compile failed, dlopen failed for {}: {}", load_path, dlerror()));
    }
    // When loaded directly from the persistent cache image, do not pass the cache
    // path to GCCHandle (so ~GCCHandle never deletes cached artifacts); temporary
    // workspace (if created during cold compile) is still cleaned up on scope exit.
    const auto handle = direct_cache_image
                            ? std::make_shared<GCCHandle>(s_, std::string{}, std::string{}, std::string{}, dl_handle, std::string{})
                            : std::make_shared<GCCHandle>(s_, c_file, so_file, log_file, dl_handle, workspace.path);
    if (!direct_cache_image) workspace.transferred = true;

    auto dlsym_lambda = [&](const std::string &sym) -> void * { return dlsym(dl_handle, sym.c_str()); };
#endif

    // Own this exact image's location before init: init may throw or compile
    // another module recursively, so GetLuaLocs().back() is not reliable.
    handle->SetLuaLoc(BindGeneratedCode(dlsym_lambda("__fakelua_bind"), s_, JIT_GCC, pr.file_name));
    // Validate before publication/registration. Runtime init executes separately
    // for every private image, including cache hits.
    for (const auto &[name, info]: gr.function_names) {
        const auto &symbol = info.c_symbol_name.empty() ? name : info.c_symbol_name;
        if (!dlsym_lambda(symbol)) {
            cache.Invalidate();
            ThrowFakeluaException("GCC image missing function " + symbol);
        }
    }
    if (!cache_hit && !published_before_load) cache.Publish(so_file, dep_file, c_file);
    cache.Release();// init may call native code which recursively compiles Lua

    // ---- Common post-load: register compiled functions + call init ----
    LOG_DEBUG(s_, "engine", "GCC: registering {} functions from {}", gr.function_names.size(), so_file);
    for (const auto &[name, info]: gr.function_names) {
        const std::string &sym = info.c_symbol_name.empty() ? name : info.c_symbol_name;
        void *func_ptr = dlsym_lambda(sym);
        if (!func_ptr) {
            ThrowFakeluaException(std::format("GCC compile failed, symbol resolution failed for {} in {}", name, so_file));
        }
        s_->GetVM().RegisterFunction(VmFunction(name, info.params_count, JIT_GCC, func_ptr, handle, info.is_vararg));
        LOG_DEBUG(s_, "engine", "Registered gcc function {} with {} params (vararg: {}) at address {}", name, info.params_count, info.is_vararg, func_ptr);
    }

    void *init_ptr = dlsym_lambda(kInitFunctionName);
    if (init_ptr) {
        State::ConstAllocScope const_alloc(s_);
        inter::DispatchCall(s_, init_ptr, nullptr, 0, JIT_GCC);
    }

#if !defined(_WIN32)
    LOG_INFO(s_, "engine", "GCC JIT compilation finished for {}", pr.file_name);
#endif
}

}// namespace fakelua
