#include "jit/gcc_jit.h"
#include "jit/gcc_handle.h"
#include "state/state.h"
#include "util/file_util.h"
#include "util/logging.h"
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

#if defined(_WIN32)
#define NOMINMAX
#include <process.h>
#include <windows.h>
#else
#include <dlfcn.h>
#include <fcntl.h>
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
    const std::string c_file = GenerateTmpFilename("fakelua_jit_", ".c");
    const std::string so_file = c_file.substr(0, c_file.size() - kCExtLen) +
#if defined(_WIN32)
                                ".dll";
#elif defined(__APPLE__)
                                ".dylib";
#else
                                ".so";
#endif
    const std::string log_file = c_file.substr(0, c_file.size() - kCExtLen) + ".gcc.log";

    // RAII cleanup for temp files on early error paths (before GCCHandle takes ownership).
    const auto cleanup_on_error = [&]() {
        std::error_code ec;
        std::filesystem::remove(c_file, ec);
        std::filesystem::remove(so_file, ec);
        std::filesystem::remove(log_file, ec);
    };

    if (std::ofstream ofs(c_file); !ofs.is_open()) {
        ThrowFakeluaException(std::format("GCC compile failed, cannot open c file {}", c_file));
    } else {
        ofs << gr.c_code;
        ofs.close();
    }

    std::vector<std::string> args;
    args.emplace_back("gcc");
    args.emplace_back("-x");
    args.emplace_back("c");
    args.emplace_back("-shared");
#if !defined(_WIN32)
    args.emplace_back("-fPIC");
#endif
#if defined(__APPLE__)
    // macOS 的 .dylib 需要 -undefined dynamic_lookup 选项，
    // 允许运行时由宿主进程提供未解析的符号。
    args.emplace_back("-undefined");
    args.emplace_back("dynamic_lookup");
#endif
    args.emplace_back(cfg.debug_mode ? "-O0" : "-O3");
    args.emplace_back("-DFAKELUA_JIT_TYPE=" + std::to_string(static_cast<int>(JIT_GCC)));
    for (const auto &path: s_->GetStateConfig().gcc_config.include_paths) {
        args.emplace_back("-I" + path);
    }
#if defined(_WIN32)
    for (const auto &path: GetWindowsDefaultLibraryPaths()) {
        args.emplace_back("-L" + path);
    }
    args.emplace_back("-lfakelua");
#endif
    for (const auto &path: s_->GetStateConfig().gcc_config.library_paths) {
        args.emplace_back("-L" + path);
    }
    for (const auto &lib: s_->GetStateConfig().gcc_config.libraries) {
        args.emplace_back("-l" + lib);
    }
    args.emplace_back("-o");
    args.emplace_back(so_file);
    args.emplace_back(c_file);

// Helper: build null-terminated argv from std::vector<std::string>.
    auto build_argv = [&]() {
        std::vector<char *> argv;
        argv.reserve(args.size() + 1);
        for (auto &arg: args) {
            argv.emplace_back(arg.data());
        }
        argv.emplace_back(nullptr);
        return argv;
    };

#if defined(_WIN32)
    auto argv = build_argv();
    const int compile_status = _spawnvp(_P_WAIT, "gcc", argv.data());
    if (compile_status == -1) {
        cleanup_on_error();
        ThrowFakeluaException(std::format("GCC compile failed for {}: cannot execute gcc (errno {}: {}). cmd: {}", pr.file_name, errno, std::strerror(errno), JoinCommand(args)));
    }
    if (compile_status != 0) {
        cleanup_on_error();
        ThrowFakeluaException(std::format("GCC compile failed for {} with exit code {}. cmd: {}", pr.file_name, compile_status, JoinCommand(args)));
    }

    HMODULE module_handle = LoadLibraryA(so_file.c_str());
    if (!module_handle) {
        cleanup_on_error();
        ThrowFakeluaException(std::format("GCC compile failed, LoadLibrary failed for {}: {}", so_file, WinErrToString(GetLastError())));
    }
    const auto handle = std::make_shared<GCCHandle>(c_file, so_file, log_file, module_handle);

    // Common post-load: register compiled functions + call init.
    auto dlsym_lambda = [&](const std::string &sym) -> void * {
        FARPROC p = GetProcAddress(module_handle, sym.c_str());
        return p ? reinterpret_cast<void *>(p) : nullptr;
    };
#else
    auto argv = build_argv();
    const int log_fd = open(log_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (log_fd < 0) {
        cleanup_on_error();
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
        cleanup_on_error();
        ThrowFakeluaException(std::format("GCC compile failed, fork failed for {}", pr.file_name));
    }

    if (pid == 0) {
        // Restore default signal mask in child before exec.
        pthread_sigmask(SIG_SETMASK, &old_mask, nullptr);
        dup2(log_fd, STDOUT_FILENO);
        dup2(log_fd, STDERR_FILENO);
        close(log_fd);
        execvp("gcc", argv.data());
        _exit(kExecFailedStatus);
    }
    close(log_fd);

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        pthread_sigmask(SIG_SETMASK, &old_mask, nullptr);
        cleanup_on_error();
        ThrowFakeluaException(std::format("GCC compile failed, waitpid failed for {}", pr.file_name));
    }
    pthread_sigmask(SIG_SETMASK, &old_mask, nullptr);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        std::string gcc_log;
        if (std::ifstream ifs(log_file); ifs.is_open()) {
            gcc_log.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
        }
        cleanup_on_error();
        ThrowFakeluaException(std::format("GCC compile failed for {} (log: {})\n{}", pr.file_name, log_file, gcc_log));
    }

    void *dl_handle = dlopen(so_file.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!dl_handle) {
        cleanup_on_error();
        ThrowFakeluaException(std::format("GCC compile failed, dlopen failed for {}: {}", so_file, dlerror()));
    }
    const auto handle = std::make_shared<GCCHandle>(c_file, so_file, log_file, dl_handle);

    auto dlsym_lambda = [&](const std::string &sym) -> void * {
        return dlsym(dl_handle, sym.c_str());
    };
#endif

    // ---- Common post-load: register compiled functions + call init ----
    for (const auto &[name, info]: gr.function_names) {
        const std::string &sym = info.c_symbol_name.empty() ? name : info.c_symbol_name;
        void *func_ptr = dlsym_lambda(sym);
        if (!func_ptr) {
            ThrowFakeluaException(std::format("GCC compile failed, symbol resolution failed for {} in {}", name, so_file));
        }
        s_->GetVM().RegisterFunction(VmFunction(name, info.params_count, JIT_GCC, func_ptr, handle, info.is_vararg));
        LOG_INFO("Registered gcc function {} with {} params (vararg: {}) at address {}", name, info.params_count, info.is_vararg, func_ptr);
    }

    void *init_ptr = dlsym_lambda(kInitFunctionName);
    if (init_ptr) {
        inter::DispatchCall(init_ptr, nullptr, 0);
    }

#if !defined(_WIN32)
    LOG_INFO("GCC JIT compilation finished for {}", pr.file_name);
#endif
}

}// namespace fakelua
