// Standalone release harness for GCC JIT compilation and execution timings.
#include "fakelua.h"
#include <chrono>
#include <filesystem>
#include <iostream>
using namespace fakelua;
using Clock = std::chrono::steady_clock;

static double Millis(Clock::time_point a, Clock::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

int main(int argc, char **argv) {
    const std::string mode = argc > 1 ? argv[1] : "warm";
    const std::string directory = argc > 2 ? argv[2] : "";
    if (mode == "cold" && (directory.empty() || !std::filesystem::path(directory).filename().string().starts_with("fakelua-cache-benchmark-"))) {
        std::cerr << "cold mode requires a disposable fakelua-cache-benchmark-* directory\n";
        return 1;
    }
    StateConfig sc;
    sc.log_level = LogLevel::Off;
#ifndef FAKELUA_CACHE_BASELINE
    sc.gcc_config.enable_cache = mode != "off";
    sc.gcc_config.cache_directory = directory;
#endif
    CompileConfig cc;
    DisableAllJit(cc);
    cc.disable_jit[JIT_GCC] = false;
    cc.debug_mode = false;
    const std::string source = R"(
local tag = "same"
function arithmetic(n)
 local sum=0
 for i=1,n do sum=sum+i end
 return sum
end
function fields(n,key)
 local t={a=1,b=2,c=3,d=4,e=5,f=6,g=7,h=8,i=9,j=10,k=11,l=12,m=13,n=14,o=15,p=16}
 local sum=0
 for i=1,n do sum=sum+t[key] end
 return sum
end
function interned_fields(n)
 local t={a=1,b=2,c=3,d=4,e=5,f=6,g=7,h=8,i=9,j=10,k=11,l=12,m=13,n=14,o=15,p=16}
 local keys={"a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p"}
 local sum=0
 for i=1,n do sum=sum+t[keys[((i-1)%16)+1]] end
 return sum
end
function strings(n,key)
 local sum=0
 for i=1,n do if key==tag then sum=sum+1 end end
 return sum
end
)";
    std::cout << "compile_ms arithmetic_ms fields_ms strings_ms interned_fields_ms\n";
    for (int sample = 0; sample < 9; ++sample) {
        // Only remove the explicitly supplied benchmark-owned directory.
        if (mode == "cold" && !directory.empty()) std::filesystem::remove_all(directory);
        FakeluaStateGuard state(sc);
        auto *s = state.GetState();
        auto a = Clock::now();
        CompileString(s, source, cc);
        auto b = Clock::now();
        int64_t result = 0;
        for (int j = 0; j < 1000; ++j) Call(s, JIT_GCC, "arithmetic", result, 100000);
        auto c = Clock::now();
        if (result != 5000050000LL) return 2;
        for (int j = 0; j < 1000; ++j) Call(s, JIT_GCC, "fields", result, 10000, std::string("p"));
        auto d = Clock::now();
        if (result != 160000) return 3;
        for (int j = 0; j < 1000; ++j) Call(s, JIT_GCC, "strings", result, 10000, std::string("same"));
        auto e = Clock::now();
        if (result != 10000) return 4;
        for (int j = 0; j < 1000; ++j) Call(s, JIT_GCC, "interned_fields", result, 10000);
        auto f = Clock::now();
        if (result != 85000) return 5;
        std::cout << Millis(a, b) << ' ' << Millis(b, c) << ' ' << Millis(c, d) << ' ' << Millis(d, e) << ' ' << Millis(e, f) << '\n';
    }
}
