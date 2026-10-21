// flua -- the FakeLua script host.
//
// Standard lifecycle: init once -> update repeatedly -> fini once before exit.
//
// Why every frame has to be its own top-level Call: fakelua has no GC, so the
// frame arena is only reset when a top-level Call starts (reentrant count back
// to zero). A `while true` loop inside one Lua Call keeps that arena alive
// forever and per-request tables/strings pile up without bound (memory leak).
// update therefore does one frame of work (pump IO/timers, dispatch requests);
// the loop and the frame boundary are driven from here.
//
// Pacing is up to the script: the host adds no sleep and calls update again as
// soon as it returns. A service that wants a fixed cadence calls os.sleep(ms)
// itself (fakelua's os.sleep takes milliseconds).
//
// Graceful exit -- both run fini first:
//   - update() returns non-zero: exit with that code;
//   - flua.set_exit(code) from Lua: flags the request, takes effect once the
//     current update returns.
// os.exit stays a hard exit that skips fini; use the above when cleanup matters.
// Signals are deliberately not handled here: what SIGINT/SIGTERM should do is the
// application's decision, so install a handler if you need one.
//
// Fixed entry points, always package "main" -- there are no flags to rename
// them, so every app is written the same way:
//     package "main"
//     function init() ... end      -- called once at startup
//     function update() ... end    -- called every frame
//     function fini() ... end      -- called once before exit
//
// Usage: flua <file.lua | dir/> [options]
//   --once           run init + fini only, without the update loop
//                    (one-shot script / self-check)
//   --jit_type=n     0=tcc 1=gcc 2=interp (default 0)
//   --debug          enable debug mode
//
// Services whose logic is entirely in Lua can use flua's built-in modules (e.g.
// sqlite.open for lookup tables) and need no C++ host of their own.
#include "fakelua.h"
#include "gflags/gflags.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace fakelua;

// Command-line flags
DEFINE_bool(debug, false, "enable debug mode");
DEFINE_bool(once, false, "run init + fini only, without the update loop (one-shot script)");
DEFINE_int32(jit_type, 0, "jit type: 0=tcc, 1=gcc, 2=interp");

namespace {

// Fixed paradigm: the entry points are hard-coded to the three functions of
// package "main"; no renaming flags are accepted.
constexpr const char *kInitEntry = "main.init";
constexpr const char *kUpdateEntry = "main.update";
constexpr const char *kFiniEntry = "main.fini";

std::atomic<bool> g_exit_requested{false};
std::atomic<int> g_exit_code{0};
bool ExitRequested() {
    return g_exit_requested.load(std::memory_order_acquire);
}

// Lua-side exit hook: flua.set_exit([code]). It only flags the request and
// returns; the current update frame runs to completion. The stored code is the
// process exit code when update itself returns 0.
void RegisterExitApi(State *s) {
    RegisterNativeFunction(s, "flua.set_exit", 0, true, [](State *state, CVar *args, int n) -> CVar {
        const int code = static_cast<int>(GetNativeIntArg(state, args, n, 0, 0));
        g_exit_code.store(code, std::memory_order_relaxed);
        g_exit_requested.store(true, std::memory_order_release);
        return NativeNil(state);
    });
}

// init/update/fini must return an integer. Call converts the result with
// FakeluaToNative<int>, so a missing return (nil) or any other type fails the
// process instead of being treated as 0.
int ReadEntryCode(State *s, JITType jit, const char *name) {
    int code = 0;
    Call(s, jit, name, code);
    return code;
}

// The argument is either a single .lua file or a directory (every .lua inside
// is compiled, in filename order).
std::vector<std::string> CollectLuaFiles(const std::string &path) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (fs::is_directory(path, ec)) {
        std::vector<std::string> files;
        for (const auto &entry: fs::directory_iterator(path, ec)) {
            if (entry.is_regular_file() && entry.path().extension() == ".lua") {
                files.push_back(entry.path().string());
            }
        }
        // Cross-file references resolve at runtime, so the order does not
        // change semantics; sorting is only for reproducible builds.
        std::sort(files.begin(), files.end());
        if (ec) {
            std::cerr << "flua: cannot read " << path << ": " << ec.message() << "\n";
            std::exit(1);
        }
        if (files.empty()) {
            std::cerr << "flua: no .lua file under " << path << "\n";
            std::exit(1);
        }
        return files;
    }
    return { path };
}

[[noreturn]] void ReportMissingEntry(const char *name) {
    std::cerr << "flua: entry '" << name << "' not found.\n"
              << "      flua always calls init / update / fini from package \"main\"; the script needs:\n"
              << "        package \"main\"\n"
              << "        function init() ... end\n"
              << "        function update() ... end\n"
              << "        function fini() ... end\n";
    std::exit(1);
}

// Runs init -> update* -> fini and returns the process exit code. A failed init
// (non-zero return or exception) skips fini.
int RunLifecycle(State *s, JITType jit) {
    int exit_code = 0;

    // Fixed paradigm: all three entries are mandatory (except update under
    // --once).
    if (!HasEntryFunction(s, jit, kInitEntry)) {
        ReportMissingEntry(kInitEntry);
    }
    const bool loop = !FLAGS_once;
    if (loop && !HasEntryFunction(s, jit, kUpdateEntry)) {
        ReportMissingEntry(kUpdateEntry);
    }
    if (!HasEntryFunction(s, jit, kFiniEntry)) {
        ReportMissingEntry(kFiniEntry);
    }

    bool init_ok = false;
    try {
        const int init_code = ReadEntryCode(s, jit, kInitEntry);
        if (init_code != 0) {
            exit_code = init_code;
            std::cerr << "flua: init returned " << init_code << ", exit\n";
        } else {
            init_ok = true;
        }
    } catch (const std::exception &e) {
        std::cerr << "flua: init failed: " << e.what() << "\n";
        exit_code = 1;
    }

    if (init_ok && loop) {
        while (!ExitRequested()) {
            int update_code = 0;
            try {
                update_code = ReadEntryCode(s, jit, kUpdateEntry);
            } catch (const std::exception &e) {
                std::cerr << "flua: update failed: " << e.what() << "\n";
                exit_code = 1;
                break;
            }
            if (update_code != 0) {
                exit_code = update_code;
                break;
            }
        }
    }
    // set_exit(code) supplies the exit code when the frame itself returned 0.
    // A non-zero return from update wins over set_exit.
    if (init_ok && exit_code == 0 && ExitRequested()) {
        exit_code = g_exit_code.load(std::memory_order_relaxed);
    }

    // init succeeded -> run fini: cleanup is needed on update errors and on
    // requested exits alike. fini must still return an integer, but that value
    // is not the process exit code. A missing return or any other exception
    // fails the process.
    if (init_ok) {
        try {
            (void) ReadEntryCode(s, jit, kFiniEntry);
        } catch (const std::exception &e) {
            std::cerr << "flua: fini failed: " << e.what() << "\n";
            exit_code = 1;
        }
    }
    return exit_code;
}

} // namespace

int main(int argc, char **argv) {
    gflags::SetUsageMessage("usage: ./flua <file.lua | dir/> [options]\n"
                            "\n"
                            "Fixed entry points, always package \"main\":\n"
                            "  package \"main\"\n"
                            "  function init() ... end      called once at startup\n"
                            "  function update() ... end    called every frame\n"
                            "  function fini() ... end      called once before exit\n"
                            "\n"
                            "update is called back-to-back; the script paces itself if it needs\n"
                            "to (os.sleep takes milliseconds). --once runs init + fini only.\n"
                            "\n"
                            "Graceful exit, runs fini first:\n"
                            "  return a non-zero code from update\n"
                            "  call flua.set_exit(code) from Lua\n"
                            "\n"
                            "Signals are not handled by flua; install a handler in the application\n"
                            "if it needs to react to SIGINT/SIGTERM.\n");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, (char ***) &argv, true);

    if (argc < 2) {
        std::cout << gflags::ProgramUsage() << std::endl;
        return 0;
    }

    const FakeluaStateGuard guard;
    const auto s = guard.GetState();

    // Lua convention: an uncaught runtime error prints the message and exits
    // with code 1, instead of letting the C++ exception escape into libc++abi
    // and abort the process (exit code 134, FakeLua Phase16 Bug 4).
    // No signal handling here on purpose: catching SIGINT/SIGTERM and deciding
    // what to do with them is the application's business, not the host's.
    try {
        RegisterExitApi(s);

        CompileConfig cfg;
        cfg.debug_mode = FLAGS_debug;
        for (const auto &file: CollectLuaFiles(argv[1])) {
            CompileFile(s, file, cfg);
        }

        return RunLifecycle(s, static_cast<JITType>(FLAGS_jit_type));
    } catch (const std::exception &e) {
        std::cerr << "flua: " << e.what() << std::endl;
        return 1;
    }
}
