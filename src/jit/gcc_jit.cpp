#include "jit/gcc_jit.h"
#include "jit/gcc_handle.h"
#include "state/state.h"
#include "util/file_util.h"
#include "util/logging.h"
#include <fstream>
#include <iterator>
#include <vector>

#if !defined(_WIN32)
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace fakelua {

namespace {
constexpr size_t kCExtLen = 2;// ".c"
constexpr int kExecFailedStatus = 127;
}

GccJitter::GccJitter(State *s) : s_(s) {
}

void GccJitter::Compile(CompileResult &cr, const CompileConfig &cfg) {
#if defined(_WIN32)
    ThrowFakeluaException(std::format("GCC JIT is not supported on Windows for {}", cr.file_name));
#else
    const std::string c_file = GenerateTmpFilename("fakelua_jit_", ".c");
    const std::string so_file = c_file.substr(0, c_file.size() - kCExtLen) + ".so";
    const std::string log_file = c_file.substr(0, c_file.size() - kCExtLen) + ".gcc.log";

    if (std::ofstream ofs(c_file); !ofs.is_open()) {
        ThrowFakeluaException(std::format("GCC compile failed, cannot open c file {}", c_file));
    } else {
        ofs << cr.c_code;
        ofs.close();
    }

    std::vector<std::string> args;
    args.emplace_back("gcc");
    args.emplace_back("-x");
    args.emplace_back("c");
    args.emplace_back("-shared");
    args.emplace_back("-fPIC");
    args.emplace_back(cfg.debug_mode ? "-O0" : "-O3");
    args.emplace_back("-DFAKELUA_JIT_TYPE=" + std::to_string(static_cast<int>(JIT_GCC)));
    for (const auto &path: s_->GetStateConfig().gcc_config.include_paths) {
        args.emplace_back("-I" + path);
    }
    for (const auto &path: s_->GetStateConfig().gcc_config.library_paths) {
        args.emplace_back("-L" + path);
    }
    for (const auto &lib: s_->GetStateConfig().gcc_config.libraries) {
        args.emplace_back("-l" + lib);
    }
    args.emplace_back("-o");
    args.emplace_back(so_file);
    args.emplace_back(c_file);

    std::vector<char *> argv;
    argv.reserve(args.size() + 1);
    for (auto &arg: args) {
        argv.emplace_back(arg.data());
    }
    argv.emplace_back(nullptr);

    const int log_fd = open(log_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (log_fd < 0) {
        ThrowFakeluaException(std::format("GCC compile failed, cannot open log file {}", log_file));
    }

    const pid_t pid = fork();
    if (pid < 0) {
        close(log_fd);
        ThrowFakeluaException(std::format("GCC compile failed, fork failed for {}", cr.file_name));
    }

    if (pid == 0) {
        dup2(log_fd, STDOUT_FILENO);
        dup2(log_fd, STDERR_FILENO);
        close(log_fd);
        execvp("gcc", argv.data());
        _exit(kExecFailedStatus);
    }
    close(log_fd);

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        ThrowFakeluaException(std::format("GCC compile failed, waitpid failed for {}", cr.file_name));
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        std::string gcc_log;
        if (std::ifstream ifs(log_file); ifs.is_open()) {
            gcc_log.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
        }
        ThrowFakeluaException(
                std::format("GCC compile failed for {} (log: {})\n{}", cr.file_name, log_file, gcc_log));
    }

    void *dl_handle = dlopen(so_file.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!dl_handle) {
        ThrowFakeluaException(std::format("GCC compile failed, dlopen failed for {}: {}", so_file, dlerror()));
    }
    const auto handle = std::make_shared<GCCHandle>(c_file, so_file, dl_handle);

    for (const auto &[name, params_count]: cr.function_names) {
        void *func_ptr = dlsym(dl_handle, name.c_str());
        if (!func_ptr) {
            ThrowFakeluaException(std::format("GCC compile failed, dlsym failed for symbol {} in {}", name, so_file));
        }
        s_->GetVM().RegisterFunction(VmFunction(name, params_count, JIT_GCC, func_ptr, handle));
        LOG_INFO("Registered gcc function {} with {} params at address {}", name, params_count, func_ptr);
    }

    LOG_INFO("GCC JIT compilation finished for {}", cr.file_name);
#endif
}

}// namespace fakelua
