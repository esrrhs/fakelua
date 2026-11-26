#include "compile/compiler.h"
#include "fakelua.h"
#include "jit/vm.h"
#include "util/debug.h"
#include "util/exception.h"
#include "util/string_util.h"
#include "gtest/gtest.h"
#include <cstring>
#include <tuple>

using namespace fakelua;

// 这些用例的全局初始化在编译期就会执行（__fakelua_init），求值过程可能触发运行时错误。
// 两个 JIT 后端都参与：TCC 的代码页没有 DWARF 展开表，错误经 jit_error_boundary 的
// 跳转边界回到 C++，因此不再需要像早期那样为它们单独关掉 JIT_TCC。
static inline void CompileFileBothJit(State *s, const std::string &file) {
    CompileFile(s, file, {});
}

// Lightweight mirror of test_infer.cpp's InferGetCCode: compile a Lua file with
// record_c_code=true (both JIT backends disabled so the step is cheap) and
// return the recorded C code.  Used below to preserve the code-generation
// structure assertions that the moved runtime-throw tests originally carried.
static std::string GetRecordedCCode(const std::string &lua_file) {
    const auto s = FakeluaNewState();
    if (!s) {
        throw std::runtime_error("GetRecordedCCode: FakeluaNewState returned null");
    }
    CompileConfig cfg;
    cfg.debug_mode = false;
    cfg.record_c_code = true;
    DisableAllJit(cfg);
    CompileFile(s, lua_file, cfg);
    const auto code = GetLastRecordedCCode(s);
    FakeluaDeleteState(s);
    std::cout << "\n=== C code for " << lua_file << " ===\n" << code << "=== end ===\n";
    return code;
}

TEST(exception, function_param_duplicate) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_function_param_duplicate.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("duplicate") != std::string::npos);
    }
}

TEST(exception, const_define_duplicate) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_const_define_duplicate.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("duplicate") != std::string::npos);
    }
}

TEST(exception, const_define_func_param_duplicate) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_const_define_func_param_duplicate.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("duplicate") != std::string::npos);
    }
}

TEST(exception, const_define_no_match) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_const_define_no_match.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("match") != std::string::npos);
    }
}

TEST(exception, function_call_exception) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    CompileFile(s, "./exception/test_function_call_exception.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            CVar ret;
            Call(s, jit_type, "test", ret, 1, 2, 3);
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("not match") != std::string::npos);
        }
    }

    for (auto jit_type: AllJitTypes()) {
        try {
            CVar ret;
            Call(s, jit_type, "test1", ret, 1, 2, 3);
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("not found") != std::string::npos);
        }
    }
}

TEST(exception, compile_fail) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_compile_fail.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("syntax error") != std::string::npos);
    }
}

TEST(exception, compile_no_file) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_no_file.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("open file") != std::string::npos);
    }
}

TEST(exception, debug_assert) {
    try {
        DEBUG_ASSERT(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("assert fail") != std::string::npos);
    }
}

TEST(exception, DebugAssertFail) {
    try {
        DebugAssertFail("false");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("assert fail") != std::string::npos);
    }
}

TEST(exception, ReplaceEscapeChars) {
    try {
        std::string s = "\\e";
        ReplaceEscapeChars(s);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("invalid escape sequence") != std::string::npos);
    }

    try {
        std::string s = "\\256";
        ReplaceEscapeChars(s);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("decimal escape too large") != std::string::npos);
    }
}

TEST(exception, return_type_error_bool) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            bool ret = 0;
            Call(s, jit_type, "test", ret, 1);
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeBool failed") != std::string::npos);
        }
    }
}

TEST(exception, return_type_error_char) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            char ret = 0;
            Call(s, jit_type, "test", ret, "1");
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeChar failed") != std::string::npos);
        }
    }
}

TEST(exception, return_type_error_uchar) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            unsigned char ret = 0;
            Call(s, jit_type, "test", ret, "1");
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUchar failed") != std::string::npos);
        }
    }
}

TEST(exception, return_type_error_short) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            short ret = 0;
            Call(s, jit_type, "test", ret, "1");
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeShort failed") != std::string::npos);
        }
    }
}

TEST(exception, return_type_error_ushort) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            unsigned short ret = 0;
            Call(s, jit_type, "test", ret, "1");
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUshort failed") != std::string::npos);
        }
    }
}

TEST(exception, return_type_error_int) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            int ret = 0;
            Call(s, jit_type, "test", ret, "1");
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeInt failed") != std::string::npos);
        }
    }
}

TEST(exception, return_type_error_uint) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            unsigned int ret = 0;
            Call(s, jit_type, "test", ret, "1");
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUint failed") != std::string::npos);
        }
    }
}

TEST(exception, return_type_error_long) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            long ret = 0;
            Call(s, jit_type, "test", ret, "1");
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeLong failed") != std::string::npos);
        }
    }
}

TEST(exception, return_type_error_ulong) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            unsigned long ret = 0;
            Call(s, jit_type, "test", ret, "1");
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUlong failed") != std::string::npos);
        }
    }
}

TEST(exception, return_type_error_long_long) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            long long ret = 0;
            Call(s, jit_type, "test", ret, "1");
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeLonglong failed") != std::string::npos);
        }
    }
}

TEST(exception, return_type_error_ulong_long) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            unsigned long long ret = 0;
            Call(s, jit_type, "test", ret, "1");
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUlonglong failed") != std::string::npos);
        }
    }
}

TEST(exception, return_type_error_float) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            float ret = 0;
            Call(s, jit_type, "test", ret, "1");
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeFloat failed") != std::string::npos);
        }
    }
}

TEST(exception, return_type_error_double) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            double ret = 0;
            Call(s, jit_type, "test", ret, "1");
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeDouble failed") != std::string::npos);
        }
    }
}

TEST(exception, return_type_error_string) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            std::string ret;
            Call(s, jit_type, "test", ret, 123);
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeString failed") != std::string::npos);
        }
    }
}

TEST(exception, return_type_error_stringview) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            std::string_view ret;
            Call(s, jit_type, "test", ret, 123);
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeStringview failed") != std::string::npos);
        }
    }
}

TEST(exception, const_define_variadic) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_const_define_variadic.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("not supported") != std::string::npos);
    }
}

TEST(exception, test_const_binop_plus_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_plus_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("attempt to perform arithmetic") != std::string::npos);
    }
}

TEST(exception, ToInteger) {
    ASSERT_ANY_THROW(ToInteger("9223372036854775808"));
    ASSERT_ANY_THROW(ToInteger("-9223372036854775809"));
    ASSERT_ANY_THROW(ToInteger("abc"));
}

TEST(exception, ToFloat) {
    ASSERT_ANY_THROW(ToFloat("f"));
    ASSERT_ANY_THROW(ToFloat("1.7976931348623157e+309"));
}

TEST(exception, test_const_binop_minus_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_minus_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("attempt to perform arithmetic") != std::string::npos);
    }
}

TEST(exception, test_const_binop_star_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_star_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("attempt to perform arithmetic") != std::string::npos);
    }
}

TEST(exception, test_const_binop_slash_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_slash_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("attempt to perform arithmetic") != std::string::npos);
    }
}

TEST(exception, test_const_binop_double_slash_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_double_slash_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("attempt to perform arithmetic") != std::string::npos);
    }
}

TEST(exception, test_const_binop_pow_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_pow_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("attempt to perform arithmetic") != std::string::npos);
    }
}

TEST(exception, test_const_binop_mod_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_mod_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("attempt to perform arithmetic") != std::string::npos);
    }
}

TEST(exception, test_const_binop_bitand_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_bitand_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("number has no integer representation") != std::string::npos);
    }
}

TEST(exception, test_const_binop_xor_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_xor_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("number has no integer representation") != std::string::npos);
    }
}

TEST(exception, test_const_binop_bitor_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_bitor_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("number has no integer representation") != std::string::npos);
    }
}

TEST(exception, test_const_binop_right_shift_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_right_shift_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("number has no integer representation") != std::string::npos);
    }
}

TEST(exception, test_const_binop_left_shift_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_left_shift_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("number has no integer representation") != std::string::npos);
    }
}

TEST(exception, test_const_binop_less_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_less_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("attempt to perform arithmetic") != std::string::npos);
    }
}

TEST(exception, test_const_binop_less_equal_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_less_equal_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("attempt to perform arithmetic") != std::string::npos);
    }
}

TEST(exception, test_const_binop_more_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_more_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("attempt to perform arithmetic") != std::string::npos);
    }
}

TEST(exception, test_const_binop_more_equal_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_binop_more_equal_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("attempt to perform arithmetic") != std::string::npos);
    }
}

TEST(exception, test_const_unop_len_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_unop_len_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("attempt to get length") != std::string::npos);
    }
}

TEST(exception, test_const_unop_bitnot_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFileBothJit(s, "./exception/test_const_unop_bitnot_error.lua");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("attempt to perform bitwise") != std::string::npos);
    }
}

TEST(exception, goto_skip_local) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    EXPECT_THROW(CompileFile(s, "./exception/test_goto_skip_single_local.lua", {}), std::exception);
}

TEST(exception, goto_skip_multiple_locals) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    EXPECT_THROW(CompileFile(s, "./exception/test_goto_skip_local.lua", {}), std::exception);
}

TEST(exception, goto_nonexistent_label) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    EXPECT_THROW(CompileFile(s, "./exception/test_goto_nonexistent_label.lua", {}), std::exception);
}

TEST(exception, goto_cross_function) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    EXPECT_THROW(CompileFile(s, "./exception/test_goto_cross_function.lua", {}), std::exception);
}

TEST(exception, goto_sibling_nested) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    EXPECT_THROW(CompileFile(s, "./exception/test_goto_sibling_nested.lua", {}), std::exception);
}

TEST(exception, goto_elseif_nonexistent) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    EXPECT_THROW(CompileFile(s, "./exception/test_goto_elseif_nonexistent.lua", {}), std::exception);
}

TEST(exception, goto_nested_skip_local) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    EXPECT_THROW(CompileFile(s, "./exception/test_goto_nested_skip_local.lua", {}), std::exception);
}

TEST(exception, duplicate_label) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    EXPECT_THROW(CompileFile(s, "./exception/test_duplicate_label.lua", {}), std::exception);
}

TEST(exception, goto_skip_local_function) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    EXPECT_THROW(CompileFile(s, "./exception/test_goto_skip_local_function.lua", {}), std::exception);
}

// 文件级只承载声明：local 定义、函数定义，以及可选的首行 package 声明。
// 其余可执行语句在 SemanticAnalysis::CheckFileLevelStmts 里就应该被拒绝，
// 不能被悄悄搬进 __fakelua_init。
TEST(exception, file_level_stmt_rejected) {
    for (const char *file:
         {"./exception/test_top_level_if.lua", "./exception/test_top_level_while.lua", "./exception/test_top_level_repeat.lua", "./exception/test_top_level_for.lua",
          "./exception/test_top_level_for_in.lua", "./exception/test_top_level_do_end.lua", "./exception/test_top_level_assign.lua", "./exception/test_top_level_table_assign.lua",
          "./exception/test_top_level_function_call.lua", "./exception/test_top_level_return.lua", "./exception/test_top_level_goto.lua", "./exception/test_top_level_package_not_first.lua"}) {
        SCOPED_TRACE(file);
        FakeluaStateGuard sg;
        auto s = sg.GetState();
        ASSERT_NE(s, nullptr);
        SetDebugLogLevel(s, 0);
        EXPECT_THROW(CompileFile(s, file, {}), std::exception);
    }
}

// Lua 的短字符串里不允许出现裸换行，\n 和 \r 都会报 unfinished string。
// 我们的 lexer 早先只排除了 \n，差分 fuzz 抓到过带裸 \r 的输入被 fakelua 接受、
// 被 Lua 拒绝。这里的脚本用 CompileString 传入，避免裸 \r 落到磁盘上被工具改写。
TEST(exception, raw_newline_in_short_string) {
    struct Case {
        const char *name;
        const char *script;
    };

    for (const Case &c: {Case{"double quoted with CR", "function f()\n  local s = \"a\rb\"\n  return s\nend"}, Case{"single quoted with CR", "function f()\n  local s = 'a\rb'\n  return s\nend"},
                         Case{"double quoted with LF", "function f()\n  local s = \"a\nb\"\n  return s\nend"}, Case{"single quoted with LF", "function f()\n  local s = 'a\nb'\n  return s\nend"}}) {
        SCOPED_TRACE(c.name);
        FakeluaStateGuard sg;
        auto s = sg.GetState();
        ASSERT_NE(s, nullptr);
        SetDebugLogLevel(s, 0);
        EXPECT_THROW(CompileString(s, c.script, {}), std::exception);
    }
}

// Lua 读数字时会把紧跟的字母、数字、点一并吞下再校验，所以 3and、1local、5..2
// 在 Lua 里都是 malformed number。差分 fuzz 反复撞到这一类：只删掉数字和关键字
// 之间的空格就能构造出来。
TEST(exception, malformed_number) {
    for (const char *body: {"local a = 3and 1", "local a = 3or 1", "local a = 1local b = 2", "local a = 5..2", "local a = 0x1g", "local a = 1e2x", "local a = 12abc"}) {
        SCOPED_TRACE(body);
        FakeluaStateGuard sg;
        auto s = sg.GetState();
        ASSERT_NE(s, nullptr);
        SetDebugLogLevel(s, 0);
        const std::string script = std::string("function f()\n  ") + body + "\n  return 0\nend";
        EXPECT_THROW(CompileString(s, script, {}), std::exception);
    }
}

// --[==[ 是 Lua 的长注释开头，必须由同层级的 ]==] 收尾，否则报 unfinished long comment。
// 早先 fakelua 只认 --[[ ... --]]，把 --[==[ 当成了行注释，比 Lua 宽松。
TEST(exception, unfinished_long_comment) {
    for (const char *body: {"--[[ never closed", "--[==[ never closed", "--[=[ closed at wrong level ]==]"}) {
        SCOPED_TRACE(body);
        FakeluaStateGuard sg;
        auto s = sg.GetState();
        ASSERT_NE(s, nullptr);
        SetDebugLogLevel(s, 0);
        const std::string script = std::string("function f()\n  ") + body + "\n  return 0\nend";
        EXPECT_THROW(CompileString(s, script, {}), std::exception);
    }
}

// Lua 没有一元加号，+1 会报 unexpected symbol。数字字面量因此不能自带正号。
TEST(exception, no_unary_plus) {
    for (const char *body: {"local a = +1", "local a = + 1", "local a = 1 + +2", "local a = +0x10"}) {
        SCOPED_TRACE(body);
        FakeluaStateGuard sg;
        auto s = sg.GetState();
        ASSERT_NE(s, nullptr);
        SetDebugLogLevel(s, 0);
        const std::string script = std::string("function f()\n  ") + body + "\n  return 0\nend";
        EXPECT_THROW(CompileString(s, script, {}), std::exception);
    }
}

// <const> 变量不能再被赋值，<const> / <close> 之外的属性名 Lua 也不认。
TEST(exception, local_attrib_errors) {
    for (const char *body: {"local x <const> = 1\n  x = 2", "local x <const> = 1\n  if true then x = 2 end", "local x <const> = 1\n  local y = 0\n  y, x = 1, 2", "local x <weird> = 1"}) {
        SCOPED_TRACE(body);
        FakeluaStateGuard sg;
        auto s = sg.GetState();
        ASSERT_NE(s, nullptr);
        SetDebugLogLevel(s, 0);
        const std::string script = std::string("function f()\n  ") + body + "\n  return 0\nend";
        EXPECT_THROW(CompileString(s, script, {}), std::exception);
    }
}

// string.sub 的参数校验必须和 Lua 一样严：缺参、非整数下标、非数字字符串都要报错，
// 不能悄悄返回空串（差分 fuzz 已抓到过）。
TEST(exception, string_sub_arg_errors) {
    for (const char *body: {"return string.sub()", "return string.sub(3)", "return string.sub(\"hello\")", "return string.sub(\"hello\", 10.5, 3)", "return string.sub(\"hello\", \"x\", 3)",
                            "return string.sub(\"hello\", nil, 3)", "local s = \"hello\"\n  return string.sub(s, s, 3)"}) {
        SCOPED_TRACE(body);
        FakeluaStateGuard sg;
        auto s = sg.GetState();
        ASSERT_NE(s, nullptr);
        SetDebugLogLevel(s, 0);
        const std::string script = std::string("function f()\n  ") + body + "\nend";
        ASSERT_NO_THROW(CompileString(s, script, {}));
        for (const auto jit_type: AllJitTypes()) {
            CVar ret;
            EXPECT_THROW(Call(s, jit_type, "f", ret), std::exception);
        }
    }
}

// Lua 的 .. 只接受字符串和数字；nil / bool / table 拼接必须报错。
TEST(exception, concat_non_string_value) {
    for (const char *body: {"return nil .. \"x\"", "return \"x\" .. nil", "return true .. \"x\"", "return {} .. \"x\""}) {
        SCOPED_TRACE(body);
        FakeluaStateGuard sg;
        auto s = sg.GetState();
        ASSERT_NE(s, nullptr);
        SetDebugLogLevel(s, 0);
        const std::string script = std::string("function f()\n  ") + body + "\nend";
        ASSERT_NO_THROW(CompileString(s, script, {}));
        for (const auto jit_type: AllJitTypes()) {
            CVar ret;
            EXPECT_THROW(Call(s, jit_type, "f", ret), std::exception);
        }
    }
}

// Lua 的 block ::= {stat} [retstat]：return 只能是所在块的最后一条语句。
// FakeLua 不能比 Lua 更宽松，否则差分 fuzz 会把它报成 fakelua 接受、Lua 拒绝的分歧。
TEST(exception, return_must_be_last_in_block) {
    for (const char *file: {"./exception/test_return_not_last.lua", "./exception/test_return_twice.lua", "./exception/test_return_before_nested_block.lua"}) {
        SCOPED_TRACE(file);
        FakeluaStateGuard sg;
        auto s = sg.GetState();
        ASSERT_NE(s, nullptr);
        SetDebugLogLevel(s, 0);
        EXPECT_THROW(CompileFile(s, file, {}), std::exception);
    }
}

// 文件级语句被拒绝时，报错信息应指明是文件级语句问题并带上出错位置。
TEST(exception, file_level_stmt_error_message) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_top_level_if.lua", {});
        FAIL() << "expected CompileFile to throw";
    } catch (const std::exception &e) {
        const std::string msg = e.what();
        EXPECT_NE(msg.find("unsupported file-level statement If"), std::string::npos) << msg;
        EXPECT_NE(msg.find("test_top_level_if.lua:3:1"), std::string::npos) << msg;
    }
}

TEST(exception, const_func_call_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    ASSERT_NO_THROW(CompileFile(s, "./exception/test_const_func_call_error.lua", {}));
}

TEST(exception, no_define_lvalue_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_no_define_lvalue_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("compile") != std::string::npos || std::string(e.what()).find("undeclared") != std::string::npos);
    }
}

// fakelua 刻意不支持 Lua 隐式全局：函数体内未声明的简单名（读 / 写 / 先写后读 /
// 点号字段的未声明根名）必须在语义分析阶段直接报错，并带上文件名、行、列，
// 而不是把读编成 kNil、把写编成对 const kNil 的赋值。
TEST(exception, undeclared_var_error) {
    struct Case {
        const char *file;
        const char *name;
        int line;
    };

    const Case cases[] = {
            {"./exception/test_undeclared_var_read_error.lua", "unknown_global", 2},
            {"./exception/test_undeclared_var_write_read_error.lua", "flag", 2},
            {"./exception/test_undeclared_field_base_error.lua", "unknown_object", 2},
            {"./exception/test_no_define_lvalue_error.lua", "b", 2},
    };
    for (const auto &c: cases) {
        SCOPED_TRACE(c.file);
        FakeluaStateGuard sg;
        auto s = sg.GetState();
        ASSERT_NE(s, nullptr);
        SetDebugLogLevel(s, 0);

        try {
            CompileFile(s, c.file, {});
            FAIL() << "expected CompileFile to throw for " << c.file;
        } catch (const std::exception &e) {
            const std::string msg = e.what();
            EXPECT_NE(msg.find("no implicit globals"), std::string::npos) << msg;
            EXPECT_NE(msg.find(c.name), std::string::npos) << msg;
            EXPECT_NE(msg.find(std::string(c.file) + ":" + std::to_string(c.line) + ":"), std::string::npos) << msg;
        }
    }
}

// 模块名不是脚本变量：点号原生库调用与模块常量必须正常编译，
// 点号链 / 调用链上的模块根名不应被误判成未声明变量。
TEST(exception, undeclared_var_check_allows_module_names) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    const std::string script = R"(
function f(x)
    local t = {}
    table.insert(t, math.floor(x))
    local p = math.pi
    local c = string.charpattern
    local s = tostring(math.maxinteger)
    if t[1] == 3 and p > 3.0 and #c == 1 and #s > 0 then
        return s
    end
    return "bad"
end
)";
    ASSERT_NO_THROW(CompileString(s, script, {}));
}

TEST(exception, global_duplicate_lvalue_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_global_duplicate_lvalue_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("conflicts with global constant") != std::string::npos);
    }
}

TEST(exception, test_break_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_break_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        const std::string msg = e.what();
        std::cout << msg << std::endl;
        // Diagnosed in semantic analysis (same loop_depth walk as continue),
        // not by a later C-compiler "compile error" or a stack-trace path.
        ASSERT_TRUE(msg.find("'break' statement not inside a loop") != std::string::npos) << msg;
    }
}

TEST(exception, test_continue_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_continue_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("continue") != std::string::npos);
    }
}

TEST(exception, test_continue_skip_local) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    EXPECT_THROW(CompileFile(s, "./exception/test_continue_skip_local.lua", {}), std::exception);
}

TEST(exception, function_too_many_params) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_function_too_many_params.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("function input parameters exceed limit 32") != std::string::npos);
    }
}

// 非数值参数传入数学特化函数：入口不再直接硬失败，而是去优化到 __dyn 动态
// 变体执行函数体；函数体内的算术仍抛错（可 pcall），三后端消息一致，
// 对齐 INTERP 后端历来行为。
TEST(exception, math_param_non_numeric_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_math_param_non_numeric_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        try {
            CVar ret;
            Call(s, jit_type, "test", ret, "hello", 1);
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("attempt to perform arithmetic on non-numeric value") != std::string::npos);
        }
    }
}

TEST(exception, const_no_init) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    EXPECT_THROW(CompileFile(s, "./exception/test_const_no_init.lua", {}), std::exception);
}

// 文件级数值字面量是常量，函数里再赋值是编译期错误。
TEST(exception, const_reassign) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    try {
        CompileFile(s, "./exception/test_const_reassign.lua", {});
        FAIL() << "reassigning file-level constant a should fail";
    } catch (const std::exception &e) {
        const std::string msg = e.what();
        EXPECT_NE(msg.find("cannot reassign file-level constant 'a'"), std::string::npos);
        EXPECT_NE(msg.find("test_const_reassign.lua"), std::string::npos);
    }
}

TEST(exception, top_level_bare_local) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    EXPECT_THROW(CompileFile(s, "./exception/test_top_level_bare_local.lua", {}), std::exception);
}

TEST(exception, test_spec_duplicate_keys) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    EXPECT_THROW(CompileFile(s, "./exception/test_spec_duplicate_keys.lua", {}), std::exception);
}

// ============================================================================
// Migrated from test_jitter.cpp – these are all compile-time expectations that
// were mixed into the JIT runtime suite.  They belong here because they only
// verify that compilation fails with a specific error; the throw happens during
// preprocessing / semantic-analysis / codegen, before any JIT code runs, so a
// single backend (the default) is sufficient.
// ============================================================================

TEST(exception, multi_name) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_multi_name_func.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("Unsupported function name") != std::string::npos);
    }
}

TEST(exception, multi_col_name) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_multi_col_name_func.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("Unsupported function name with method definition") != std::string::npos);
    }
}

TEST(exception, assign_not_match) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_assign_not_match.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("not match") != std::string::npos);
    }
}

TEST(exception, table_var_func_call) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    EXPECT_NO_THROW(CompileFile(s, "./exception/test_table_var_func_call.lua", {}));
}

TEST(exception, vararg_nested_function) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_vararg_with_nested_function.lua", {});
        FAIL() << "expected CompileFile to throw for nested named function statement";
    } catch (const std::exception &e) {
        const std::string msg = e.what();
        std::cout << msg << std::endl;
        // Rejected in semantic analysis (before codegen's internal "not support stmt type").
        EXPECT_NE(msg.find("no implicit globals"), std::string::npos) << msg;
        EXPECT_NE(msg.find("named function statement"), std::string::npos) << msg;
        EXPECT_NE(msg.find("test_vararg_with_nested_function.lua"), std::string::npos) << msg;
    }
}

// <const> 通过 FunctionDef / LocalFunction 捕获外层词法 const 时仍禁止赋值；
// 同名 local 遮蔽后允许赋值；块作用域遮蔽结束后外层 const 仍受保护。
TEST(exception, const_assign_through_nested_functions) {
    for (const char *file: {"./exception/test_const_assign_funcdef.lua", "./exception/test_const_assign_localfunction.lua", "./exception/test_const_assign_block_scope.lua"}) {
        SCOPED_TRACE(file);
        FakeluaStateGuard sg;
        auto s = sg.GetState();
        ASSERT_NE(s, nullptr);
        SetDebugLogLevel(s, 0);
        try {
            CompileFile(s, file, {});
            FAIL() << "expected CompileFile to throw for " << file;
        } catch (const std::exception &e) {
            const std::string msg = e.what();
            EXPECT_NE(msg.find("attempt to assign to const variable"), std::string::npos) << msg;
            EXPECT_NE(msg.find(file), std::string::npos) << msg;
        }
    }

    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    EXPECT_NO_THROW(CompileFile(s, "./exception/test_const_assign_nested_shadow_ok.lua", {}));
}

TEST(exception, vararg_nested_localfunction) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    EXPECT_NO_THROW(CompileFile(s, "./exception/test_vararg_with_nested_localfunction.lua", {}));
}

TEST(exception, vararg_funcdef) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    EXPECT_NO_THROW(CompileFile(s, "./exception/test_vararg_with_funcdef.lua", {}));
}

TEST(exception, for_loop_zero_step_int) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_for_loop_zero_step_int.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("'for' step is zero") != std::string::npos);
    }
}

TEST(exception, for_loop_zero_step_float) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_for_loop_zero_step_float.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("'for' step is zero") != std::string::npos);
    }
}

TEST(exception, spec_call_arg_count_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_spec_call_arg_count_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("wrong number of arguments") != std::string::npos);
    }
}

// 严格 arity 不能只对 C 后端生效：只编解释器字节码（两个 JIT 都关掉）时也要拒绝少传参数，
// 否则同一个脚本换 --jit_type 就有不同的通过/失败结果。
TEST(exception, interp_only_still_checks_call_arity) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    CompileConfig cfg;
    cfg.disable_jit[JIT_TCC] = true;
    cfg.disable_jit[JIT_GCC] = true;
    try {
        CompileFile(s, "./exception/test_spec_call_arg_count_error.lua", cfg);
        ASSERT_TRUE(false) << "interp-only compile should reject the arity mismatch";
    } catch (const std::exception &e) {
        ASSERT_TRUE(std::string(e.what()).find("wrong number of arguments") != std::string::npos) << e.what();
    }
}

TEST(exception, set_table_arg_count_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_set_table_arg_count_error1.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FAKELUA_SET_TABLE expects exactly 3 arguments") != std::string::npos);
    }

    // Fresh state: a failed compile leaves the JIT state in an unknown condition.
    FakeluaStateGuard sg2;
    auto s2 = sg2.GetState();
    try {
        CompileFile(s2, "./exception/test_set_table_arg_count_error2.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FAKELUA_SET_TABLE expects exactly 3 arguments") != std::string::npos);
    }
}

TEST(exception, dup_const_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_dup_const_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("duplicate global const variable") != std::string::npos);
    }
}

TEST(exception, dup_param_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_dup_param_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("the param name is duplicated") != std::string::npos);
    }
}

TEST(exception, shadow_const_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_shadow_const_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("conflicts with global constant") != std::string::npos);
    }
}

TEST(exception, duplicate_const_define_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_duplicate_const_define_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("duplicate global const variable") != std::string::npos);
    }
}

TEST(exception, duplicate_func_param_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_duplicate_func_param_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("the param name is duplicated") != std::string::npos);
    }
}

TEST(exception, shadow_global_const_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_shadow_global_const_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("conflicts with global constant") != std::string::npos);
    }
}

TEST(exception, math_spec_too_few_args) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_math_spec_too_few_args.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("wrong number of arguments") != std::string::npos);
    }
}

// ============================================================================
// Migrated from test_infer.cpp – runtime exceptions thrown from JIT-compiled
// code.  These throws happen during Call(), not during compilation, so the
// compile step must succeed first.  Both backends are exercised: TCC generates
// no DWARF unwind tables, so the error travels back through the jump boundary
// in jit_error_boundary.h rather than by unwinding out of the JIT frame.
// ============================================================================

TEST(exception, spec_assign_nonnumeric_float_throws) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_spec_assign_nonnumeric_float_throws.lua", {});

    for (auto jit_type: CCodeJitTypes()) {
        try {
            double dret = 0.0;
            Call(s, jit_type, "test", dret, 2.5);
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("attempt to assign non-numeric value to typed float variable") != std::string::npos);
        }
    }
}

TEST(exception, spec_assign_nonnumeric_int_throws) {
    const auto code = GetRecordedCCode("./exception/test_spec_assign_nonnumeric_int_throws.lua");
    // n is a math param: the int specialization must be generated.
    ASSERT_NE(code.find("test_0(int64_t n)"), std::string::npos);
    // x is a native int64_t accumulator initialized via native arithmetic.
    ASSERT_NE(code.find("int64_t x = FL_INT_ADD((n), (1))"), std::string::npos);
    // CVar guard – int branch.
    ASSERT_NE(code.find(".type_ == VAR_INT)"), std::string::npos);
    // CVar guard – float branch casts to int64_t.
    ASSERT_NE(code.find("(int64_t)"), std::string::npos);
    // Error branch for non-numeric CVar.
    ASSERT_NE(code.find("attempt to assign non-numeric value to typed int variable"), std::string::npos);
    // Return uses native addition. 特化函数的 return 先落到临时量，TCC 才能在返回前弹出 Lua 帧。
    ASSERT_NE(code.find("__fl_frame_ret = FL_INT_ADD((n), (x))"), std::string::npos);

    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);
    CompileFile(s, "./exception/test_spec_assign_nonnumeric_int_throws.lua", {});

    for (auto jit_type: CCodeJitTypes()) {
        try {
            int ret = 0;
            Call(s, jit_type, "test", ret, 5);
            ASSERT_TRUE(false);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            ASSERT_TRUE(std::string(e.what()).find("attempt to assign non-numeric value to typed int variable") != std::string::npos);
        }
    }
}

TEST(exception, no_arg_call) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    try {
        CompileFile(s, "./exception/test_no_arg_call.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("wrong number of arguments") != std::string::npos);
    }
}

TEST(exception, const_table_modify_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    CompileFile(s, "./exception/test_const_table_modify_error.lua", {});

    const char *throw_fns[] = {
            "test_modify_const",           "test_sort_const",   "test_sort_single_const",      "test_move_into_const",     "test_move_empty_into_const",          "test_insert_const",
            "test_insert_float_pos_const", "test_remove_const", "test_remove_oob_valid_const", "test_modify_packed_const", "test_copy_const_tbl_to_table_modify",
    };

    for (auto jit_type: AllJitTypes()) {
        for (const char *fn: throw_fns) {
            double res = 0;
            try {
                Call(s, jit_type, fn, res);
                ADD_FAILURE() << JitTypeName(jit_type) << " " << fn << " should have thrown";
            } catch (const std::exception &e) {
                const std::string msg = e.what();
                EXPECT_NE(msg.find("attempt to modify a const table"), std::string::npos) << JitTypeName(jit_type) << " " << fn << ": " << msg;
            }
        }

        int64_t num_res = 0;
        Call(s, jit_type, "test_copy_const_num_to_table", num_res);
        EXPECT_EQ(num_res, 1) << JitTypeName(jit_type);
    }
}

TEST(exception, const_table_nested_modify_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    CompileFile(s, "./exception/test_const_table_nested_modify_error.lua", {});

    for (auto jit_type: AllJitTypes()) {
        double res = 0;
        EXPECT_THROW(Call(s, jit_type, "test_modify_nested", res), std::exception);
    }
}

// ============================================================================
// 错误必须能穿过 JIT 帧回到调用方，而不是让进程 abort。TCC 的代码页没有 DWARF
// 展开表，这两条路径以前都会 std::terminate()：
//   1. 文件级语句被编译器搬进 __fakelua_init，在 CompileString 期间就执行
//   2. 已编译函数在 Call 期间出错
// ============================================================================

TEST(exception, init_runtime_error_is_catchable) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    // b 尚未定义，加法在 __fakelua_init 里就会失败
    EXPECT_THROW(CompileString(s, "local a = 1\nlocal b = a + b\n", {}), FakeluaException);
}

TEST(exception, call_runtime_error_is_catchable) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, 0);

    CompileString(s, "function call_non_function()\nlocal s = \"hello\"\nreturn string.sub(s, 1, 3)(s)\nend", {});

    for (auto jit_type: AllJitTypes()) {
        double res = 0;
        EXPECT_THROW(Call(s, jit_type, "call_non_function", res), FakeluaException);
    }
}

TEST(exception, stacktrace_in_thrown_message) {
    try {
        ThrowFakeluaException("stacktrace probe");
        FAIL() << "ThrowFakeluaException should throw";
    } catch (const FakeluaException &e) {
        const std::string msg = e.what();
        EXPECT_NE(msg.find("stacktrace probe"), std::string::npos) << msg;
        const auto pos = msg.find("stacktrace:\n");
        ASSERT_NE(pos, std::string::npos) << msg;
        EXPECT_GT(msg.size(), pos + std::strlen("stacktrace:\n")) << msg;
        EXPECT_NE(msg.find("0#"), std::string::npos) << msg;
    }
}

TEST(exception, stacktrace_in_build_error_message) {
    const std::string msg = BuildFakeluaErrorMessage("build probe");
    EXPECT_NE(msg.find("build probe"), std::string::npos) << msg;
    const auto pos = msg.find("stacktrace:\n");
    ASSERT_NE(pos, std::string::npos) << msg;
    EXPECT_GT(msg.size(), pos + std::strlen("stacktrace:\n")) << msg;
    EXPECT_NE(msg.find("0#"), std::string::npos) << msg;
}

// 运行期错误必须带 Lua 源码位置（file:line），三个后端一致。
// 行号对应 test/lua/exception/test_error_location.lua。
TEST(exception, runtime_error_carries_lua_location) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, static_cast<int>(LogLevel::Error));
    CompileFile(s, "./exception/test_error_location.lua", {});

    struct Case {
        const char *fn;
        const char *stmt_line;
        const char *def_line;
    };

    // GCC 不在每次调用时记语句行，报错行是函数定义行。TCC 和解释器仍是语句行。
    const Case cases[] = {
            {"arith_on_nil", "test_error_location.lua:6", "test_error_location.lua:4"},
            {"index_non_table", "test_error_location.lua:11", "test_error_location.lua:9"},
            {"call_non_function", "test_error_location.lua:16", "test_error_location.lua:14"},
            {"native_bad_argument", "test_error_location.lua:22", "test_error_location.lua:21"},
            {"error_after_callee_returns", "test_error_location.lua:46", "test_error_location.lua:45"},
    };
    for (const auto jit_type: AllJitTypes()) {
        for (const auto &c: cases) {
            const std::string name = std::string("ErrorLocation.") + c.fn;
            const char *location = jit_type == JIT_GCC ? c.def_line : c.stmt_line;
            int64_t ret = 0;
            try {
                Call(s, jit_type, name, ret);
                ADD_FAILURE() << JitTypeName(jit_type) << " " << c.fn << " should have thrown";
            } catch (const std::exception &e) {
                const std::string msg = e.what();
                EXPECT_NE(msg.find(location), std::string::npos) << JitTypeName(jit_type) << " " << c.fn << ": " << msg;
            }
        }
    }
}

// 嵌套调用的报错要带上 Lua 调用栈：最内层在前，主叫方的函数名和调用行也在。
TEST(exception, runtime_error_includes_lua_call_stack) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, static_cast<int>(LogLevel::Error));
    CompileFile(s, "./exception/test_error_location.lua", {});
    for (auto jit_type: AllJitTypes()) {
        int64_t ret = 0;
        try {
            Call(s, jit_type, "ErrorLocation.outer_call", ret);
            ADD_FAILURE() << JitTypeName(jit_type) << " outer_call should have thrown";
        } catch (const FakeluaException &e) {
            const std::string msg = e.what();
            EXPECT_NE(msg.find("stack traceback:"), std::string::npos) << JitTypeName(jit_type) << ": " << msg;
            const auto inner = msg.find("in function 'ErrorLocation.innermost'");
            const auto outer = msg.find("in function 'ErrorLocation.outer_call'");
            EXPECT_NE(inner, std::string::npos) << JitTypeName(jit_type) << ": " << msg;
            EXPECT_NE(outer, std::string::npos) << JitTypeName(jit_type) << ": " << msg;
            EXPECT_LT(inner, outer) << JitTypeName(jit_type) << ": " << msg;
            const char *inner_line = jit_type == JIT_GCC ? "test_error_location.lua:26" : "test_error_location.lua:27";
            const char *outer_line = jit_type == JIT_GCC ? "test_error_location.lua:30" : "test_error_location.lua:31";
            EXPECT_NE(msg.find(inner_line), std::string::npos) << msg;
            EXPECT_NE(msg.find(outer_line), std::string::npos) << msg;
            EXPECT_NE(e.LuaStack().find("ErrorLocation.innermost"), std::string::npos) << e.LuaStack();
        }
    }
}

// 整数特化函数同样会抛 FakeluaException（这里是整除 0）。抛出时特化帧和调用方都要在栈上。
TEST(exception, spec_function_throw_includes_lua_call_stack) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, static_cast<int>(LogLevel::Error));
    CompileFile(s, "./exception/test_spec_throw.lua", {});
    for (auto jit_type: AllJitTypes()) {
        // GCC 特化函数热路径不写行号，抛错时用函数定义行。TCC 和解释器记下语句行。
        const char *div_line = jit_type == JIT_GCC ? "test_spec_throw.lua:12" : "test_spec_throw.lua:13";
        const char *spec_caller_line = jit_type == JIT_GCC ? "test_spec_throw.lua:8" : "test_spec_throw.lua:9";
        const auto check = [&](const char *fn, const char *caller_line, auto &&invoke) {
            try {
                invoke();
                ADD_FAILURE() << JitTypeName(jit_type) << " " << fn << " should have thrown";
            } catch (const FakeluaException &e) {
                const std::string msg = e.what();
                EXPECT_NE(msg.find("floor division by zero"), std::string::npos) << JitTypeName(jit_type) << " " << fn << ": " << msg;
                const auto inner = msg.find("in function 'SpecThrow.divzero'");
                const auto outer = msg.find(std::string("in function '") + fn + "'");
                EXPECT_NE(inner, std::string::npos) << JitTypeName(jit_type) << " " << fn << ": " << msg;
                EXPECT_NE(outer, std::string::npos) << JitTypeName(jit_type) << " " << fn << ": " << msg;
                EXPECT_LT(inner, outer) << JitTypeName(jit_type) << " " << fn << ": " << msg;
                EXPECT_NE(msg.find(div_line), std::string::npos) << msg;
                EXPECT_NE(msg.find(caller_line), std::string::npos) << msg;
            }
        };
        check("SpecThrow.plain_caller", jit_type == JIT_GCC ? "test_spec_throw.lua:4" : "test_spec_throw.lua:5", [&] {
            int64_t ret = 0;
            Call(s, jit_type, "SpecThrow.plain_caller", ret);
        });
        check("SpecThrow.caller", spec_caller_line, [&] {
            int64_t ret = 0;
            Call(s, jit_type, "SpecThrow.caller", ret, static_cast<int64_t>(3));
        });
    }
}

// 嵌套 Lua 帧里的错误取最内层那条语句，不是主叫方的行。
TEST(exception, runtime_error_location_is_innermost) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, static_cast<int>(LogLevel::Error));
    CompileFile(s, "./exception/test_error_location.lua", {});
    for (auto jit_type: AllJitTypes()) {
        int64_t ret = 0;
        Call(s, jit_type, "ErrorLocation.nested_frames", ret);
        EXPECT_EQ(ret, 1) << JitTypeName(jit_type);
    }
}

// 同时加载多个 .so 时，跨编译单元闭包调用抛错与返回后抛错都必须准确定位到对应文件与行号。
TEST(exception, runtime_error_location_multi_so_closure) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, static_cast<int>(LogLevel::Error));
    CompileFile(s, "./exception/test_error_location.lua", {});
    CompileFile(s, "./exception/test_error_location_peer.lua", {});

    for (const auto jit_type: AllJitTypes()) {
        int64_t ret = 0;
        try {
            Call(s, jit_type, "ErrorLocationPeer.call_peer_throwing_closure", ret);
            ADD_FAILURE() << JitTypeName(jit_type) << " call_peer_throwing_closure should have thrown";
        } catch (const std::exception &e) {
            const std::string msg = e.what();
            const char *closure_line = jit_type == JIT_GCC ? "test_error_location.lua:51" : "test_error_location.lua:52";
            EXPECT_NE(msg.find(closure_line), std::string::npos) << JitTypeName(jit_type) << ": " << msg;
        }

        try {
            Call(s, jit_type, "ErrorLocationPeer.error_after_peer_closure_returns", ret);
            ADD_FAILURE() << JitTypeName(jit_type) << " error_after_peer_closure_returns should have thrown";
        } catch (const std::exception &e) {
            const std::string msg = e.what();
            const char *peer_line = jit_type == JIT_GCC ? "test_error_location_peer.lua:11" : "test_error_location_peer.lua:13";
            EXPECT_NE(msg.find(peer_line), std::string::npos) << JitTypeName(jit_type) << ": " << msg;
        }
    }
}

// 尾调用替换当前解释器帧后，运行期错误仍必须定位到最内层那条语句。
TEST(exception, runtime_error_location_tail_call) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, static_cast<int>(LogLevel::Error));
    CompileFile(s, "./exception/test_error_location_tail.lua", {});

    for (const auto jit_type: AllJitTypes()) {
        int64_t ret = 0;
        try {
            Call(s, jit_type, "TailErr.boom", ret, 5);
            ADD_FAILURE() << JitTypeName(jit_type) << " TailErr.boom should have thrown";
        } catch (const std::exception &e) {
            const std::string msg = e.what();
            const char *boom_line = jit_type == JIT_GCC ? "test_error_location_tail.lua:4" : "test_error_location_tail.lua:6";
            EXPECT_NE(msg.find(boom_line), std::string::npos) << JitTypeName(jit_type) << ": " << msg;
            EXPECT_NE(msg.find("TailErr.boom"), std::string::npos) << JitTypeName(jit_type) << ": " << msg;
        }
        try {
            Call(s, jit_type, "TailErr.call_missing", ret, 5);
            ADD_FAILURE() << JitTypeName(jit_type) << " TailErr.call_missing should have thrown";
        } catch (const std::exception &e) {
            const std::string msg = e.what();
            const char *missing_line = jit_type == JIT_GCC ? "test_error_location_tail.lua:11" : "test_error_location_tail.lua:13";
            EXPECT_NE(msg.find(missing_line), std::string::npos) << JitTypeName(jit_type) << ": " << msg;
            EXPECT_NE(msg.find("does_not_exist"), std::string::npos) << JitTypeName(jit_type) << ": " << msg;
        }
    }
}

// INTERP 深层尾调用后报错，不能变成 C 栈溢出，位置仍是 boom 里那一行。
TEST(exception, runtime_error_location_tail_call_deep_interp) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(s, static_cast<int>(LogLevel::Error));
    CompileConfig cfg;
    cfg.debug_mode = false;
    cfg.disable_jit[JIT_TCC] = true;
    cfg.disable_jit[JIT_GCC] = true;
    CompileFile(s, "./exception/test_error_location_tail.lua", cfg);
    int64_t ret = 0;
    try {
        Call(s, JIT_INTERP, "TailErr.boom", ret, 20000);
        ADD_FAILURE() << "deep INTERP TailErr.boom should have thrown";
    } catch (const std::exception &e) {
        const std::string msg = e.what();
        EXPECT_NE(msg.find("test_error_location_tail.lua:6"), std::string::npos) << msg;
    }
}

TEST(exception, gcc_cache_preserves_each_string_input_debug_filename) {
    StateConfig state_cfg;
    state_cfg.log_level = LogLevel::Off;
    FakeluaStateGuard first(state_cfg), second(state_cfg);
    CompileConfig cfg;
    DisableAllJit(cfg);
    cfg.disable_jit[JIT_GCC] = false;
    cfg.debug_mode = false;
    const std::string source = "function cached_failure()\n local value=nil\n return value.missing\nend\n";
    Compiler first_compiler(first.GetState()), second_compiler(second.GetState());
    const auto a = first_compiler.CompileString(source, cfg);
    const auto b = second_compiler.CompileString(source, cfg);
    ASSERT_NE(a.file_name, b.file_name);
    for (auto [state, own, other]: {std::tuple{first.GetState(), a.file_name, b.file_name}, std::tuple{second.GetState(), b.file_name, a.file_name}}) {
        try {
            int64_t result = 0;
            Call(state, JIT_GCC, "cached_failure", result);
            FAIL() << "expected runtime error";
        } catch (const std::exception &e) {
            const std::string message = e.what();
            // GCC 报函数定义行，这条语句在第 1 行。
            EXPECT_NE(message.find(own + ":1"), std::string::npos) << message;
            EXPECT_EQ(message.find(other), std::string::npos) << message;
        }
    }
}

TEST(exception, gcc_cache_failed_init_same_state_retry_all_backends) {
    const std::string source = R"(
        local text = retry_init_seed()
        function retry_init_read() return text end
    )";
    for (auto type: {JIT_TCC, JIT_GCC, JIT_INTERP}) {
        for (bool debug: {false, true}) {
            SCOPED_TRACE(std::format("backend={} debug={}", static_cast<int>(type), debug));
            StateConfig config;
            config.log_level = LogLevel::Off;
            FakeluaStateGuard state(config);
            auto *s = state.GetState();
            bool fail = true;
            int calls = 0;
            RegisterNativeFunction(s, "retry_init_seed", false, std::function<std::string(State *)>([&](State *) {
                                       ++calls;
                                       if (fail) throw std::runtime_error("expected initialization failure");
                                       return std::string("ok after retry");
                                   }));
            CompileConfig cfg;
            DisableAllJit(cfg);
            cfg.disable_jit[type] = false;
            cfg.debug_mode = debug;
            EXPECT_THROW(CompileString(s, source, cfg), std::exception);
            fail = false;
            CompileString(s, source, cfg);
            std::string value;
            Call(s, type, "retry_init_read", value);
            EXPECT_EQ(value, "ok after retry");
            EXPECT_EQ(calls, 2);
        }
    }
}

static void RegisterHostDiagnosticFunctions(State *s) {
    RegisterNativeFunction(s, "host_standard_error", 0, false, [](State *, CVar *, int) -> CVar { throw std::runtime_error("host failure"); });
    RegisterNativeFunction(s, "host_unknown_error", 0, false, [](State *, CVar *, int) -> CVar { throw 7; });
    RegisterNativeFunction(s, "host_compile_bad_source", 0, false, [](State *state, CVar *, int) -> CVar {
        CompileFile(state, "./syntax/test_unexpected_parenthesis.lua", {});
        return {};
    });
}

// Diagnostic improvements must not change native exception propagation policy.
TEST(exception, host_exceptions_preserve_backend_policy) {
    for (bool debug: {false, true}) {
        SCOPED_TRACE(debug);
        StateConfig config;
        config.log_level = LogLevel::Off;
        FakeluaStateGuard state(config);
        auto *s = state.GetState();
        RegisterHostDiagnosticFunctions(s);
        CompileConfig cfg;
        cfg.debug_mode = debug;
        CompileFile(s, "./exception/test_host_error_location.lua", cfg);
        for (auto backend: AllJitTypes()) {
            SCOPED_TRACE(JitTypeName(backend));
            for (const auto &[name, line, reason]:
                 {std::tuple{"standard", 4, "host failure"}, std::tuple{"unknown", 9, "unknown exception thrown from native code"}, std::tuple{"nested", 4, "host failure"}}) {
                int64_t result = 0;
                try {
                    Call(s, backend, std::string("HostErrorLocation.") + name, result);
                    ADD_FAILURE() << "expected host exception";
                } catch (const FakeluaException &e) {
                    EXPECT_EQ(backend, JIT_TCC) << "GCC/INTERP must preserve the original host exception";
                    EXPECT_NE(e.Msg().find(reason), std::string::npos);
                    EXPECT_EQ(e.Loc(), std::format("./exception/test_host_error_location.lua:{}", line));
                    const std::string text = e.what();
                    const auto prefix = text.find("fakelua error:");
                    EXPECT_EQ(text.find("fakelua error:", prefix + 1), std::string::npos) << text;
                } catch (const std::runtime_error &e) {
                    EXPECT_NE(backend, JIT_TCC);
                    EXPECT_EQ(std::string(e.what()), reason);
                } catch (int value) {
                    EXPECT_NE(backend, JIT_TCC);
                    EXPECT_EQ(std::string(name), "unknown");
                    EXPECT_EQ(value, 7);
                } catch (...) {
                    ADD_FAILURE() << "unexpected host exception type";
                }
                Call(s, backend, "HostErrorLocation.success", result);
                EXPECT_EQ(result, 42);
            }
            std::string message;
            Call(s, backend, "HostErrorLocation.protected_standard", message);
            if (backend == JIT_TCC) {
                EXPECT_NE(message.find("test_host_error_location.lua:4"), std::string::npos) << message;
            }
            EXPECT_NE(message.find("host failure"), std::string::npos) << message;
        }
    }
}

TEST(exception, nested_parse_failure_keeps_its_own_source_location) {
    StateConfig config;
    config.log_level = LogLevel::Off;
    FakeluaStateGuard state(config);
    auto *s = state.GetState();
    RegisterHostDiagnosticFunctions(s);
    CompileFile(s, "./exception/test_host_error_location.lua", {});
    for (auto backend: AllJitTypes()) {
        SCOPED_TRACE(JitTypeName(backend));
        int64_t result = 0;
        try {
            Call(s, backend, "HostErrorLocation.compile_failure", result);
            ADD_FAILURE() << "expected parse failure";
        } catch (const FakeluaException &e) {
            EXPECT_EQ(e.Loc(), "./syntax/test_unexpected_parenthesis.lua:2.15");
            EXPECT_NE(e.Msg().find("syntax error"), std::string::npos);
            const std::string text = e.what();
            const auto at = text.find("at ./syntax/test_unexpected_parenthesis.lua:2.15");
            ASSERT_NE(at, std::string::npos) << text;
            const auto caller = text.find("test_host_error_location.lua:");
            if (caller != std::string::npos) {
                EXPECT_GT(caller, at) << text;
            }
        }
    }
}

TEST(exception, runtime_location_after_multiline_string) {
    for (bool debug: {false, true}) {
        SCOPED_TRACE(debug);
        StateConfig config;
        config.log_level = LogLevel::Off;
        FakeluaStateGuard state(config);
        auto *s = state.GetState();
        CompileConfig cfg;
        cfg.debug_mode = debug;
        CompileFile(s, "./exception/test_multiline_error_location.lua", cfg);
        for (auto backend: AllJitTypes()) {
            SCOPED_TRACE(JitTypeName(backend));
            int64_t result = 0;
            try {
                Call(s, backend, "MultilineErrorLocation.fail", result);
                ADD_FAILURE() << "expected indexing failure";
            } catch (const FakeluaException &e) {
                const char *fail_line = backend == JIT_GCC ? "./exception/test_multiline_error_location.lua:6" : "./exception/test_multiline_error_location.lua:8";
                EXPECT_EQ(e.Loc(), fail_line);
            }
            std::string text;
            Call(s, backend, "MultilineErrorLocation.text", text);
            EXPECT_EQ(text, "first\nsecond\nthird");
        }
    }
}

TEST(exception, native_calls_without_lua_frame_preserve_host_exceptions) {
    StateConfig config;
    config.log_level = LogLevel::Off;
    FakeluaStateGuard state(config);
    auto *s = state.GetState();
    RegisterHostDiagnosticFunctions(s);
    CompileFile(s, "./exception/test_host_error_location.lua", {});
    for (auto backend: AllJitTypes()) {
        SCOPED_TRACE(JitTypeName(backend));
        int64_t result = 0;
        EXPECT_THROW(Call(s, backend, "HostErrorLocation.standard", result), std::exception);
        try {
            CallByNameArgs(s, backend, "host_standard_error", 0, nullptr);
            ADD_FAILURE() << "expected host exception";
        } catch (const std::runtime_error &e) {
            EXPECT_EQ(std::string(e.what()), "host failure");
        }
        try {
            CallByNameArgs(s, backend, "host_unknown_error", 0, nullptr);
            ADD_FAILURE() << "expected host exception";
        } catch (int value) {
            EXPECT_EQ(value, 7);
        } catch (...) {
            ADD_FAILURE() << "host exception type changed without a Lua frame";
        }
    }
}

namespace {
size_t CountSubstr(std::string_view haystack, std::string_view needle) {
    if (needle.empty()) return 0;
    size_t count = 0;
    size_t pos = 0;
    while ((pos = haystack.find(needle, pos)) != std::string_view::npos) {
        ++count;
        pos += needle.size();
    }
    return count;
}
}// namespace

// -O3 下 local 函数和运行时表访问辅助函数都会内联，报错仍要保留内联进去的叶子函数与外层主叫。
TEST(exception, gcc_o3_inlined_helper_throw_keeps_leaf_and_caller) {
    StateConfig sc;
    sc.log_level = LogLevel::Off;
    FakeluaStateGuard sg(sc);
    State *s = sg.GetState();
    CompileConfig cfg;
    cfg.debug_mode = false;
    cfg.disable_jit[JIT_TCC] = true;
    cfg.disable_jit[JIT_INTERP] = true;
    CompileString(s,
                  "local function leaf_index(v)\n"
                  "    return v.missing\n"
                  "end\n"
                  "function o3_caller()\n"
                  "    return leaf_index(nil) + 1\n"
                  "end\n",
                  cfg);
    int64_t ret = 0;
    try {
        Call(s, JIT_GCC, "o3_caller", ret);
        FAIL() << "expected runtime error";
    } catch (const FakeluaException &e) {
        const std::string stack = e.LuaStack();
        const auto leaf = stack.find("in function 'leaf_index'");
        const auto caller = stack.find("in function 'o3_caller'");
        EXPECT_NE(leaf, std::string::npos) << e.what();
        EXPECT_NE(caller, std::string::npos) << e.what();
        EXPECT_LT(leaf, caller) << e.what();
        EXPECT_TRUE(e.Loc().ends_with(":1")) << e.Loc() << "\n" << e.what();
    }
}

// 普通递归函数和嵌套匿名闭包的 dispatcher 为 false，不能被当成入口分发器折叠成一帧。
TEST(exception, ordinary_recursion_and_nested_anonymous_closures_keep_all_frames) {
    StateConfig sc;
    sc.log_level = LogLevel::Off;
    FakeluaStateGuard sg(sc);
    State *s = sg.GetState();
    CompileString(s, R"(
        function rec_plain(tag, n)
            if n <= 0 then
                error(tag)
            end
            return (rec_plain(tag, n - 1))
        end

        function nested_anon()
            local outer = function()
                local inner = function()
                    error("anon_boom")
                end
                return (inner())
            end
            return (outer())
        end
    )",
                  {});
    for (auto backend: AllJitTypes()) {
        SCOPED_TRACE(JitTypeName(backend));
        int64_t ret = 0;
        try {
            Call(s, backend, "rec_plain", ret, std::string("rec_boom"), int64_t{3});
            ADD_FAILURE() << "expected recursion error";
        } catch (const FakeluaException &e) {
            EXPECT_EQ(CountSubstr(e.LuaStack(), "in function 'rec_plain'"), 4u) << e.LuaStack();
        }
        try {
            Call(s, backend, "nested_anon", ret);
            ADD_FAILURE() << "expected nested closure error";
        } catch (const FakeluaException &e) {
            EXPECT_GE(CountSubstr(e.LuaStack(), "in function '(anonymous)'"), 2u) << e.LuaStack();
            EXPECT_NE(e.LuaStack().find("in function 'nested_anon'"), std::string::npos) << e.LuaStack();
        }
    }
}

// 超过 256 层的深递归正常返回或经 pcall 恢复后不得污染浅层帧；未捕获超深报错要带省略帧提示。
TEST(exception, deep_recursion_overflow_preserves_shallow_frames_and_marks_omitted) {
    StateConfig sc;
    sc.log_level = LogLevel::Off;
    FakeluaStateGuard sg(sc);
    State *s = sg.GetState();
    CompileString(s, R"(
        function deep_ok(tag, n)
            if n <= 0 then
                return 1
            end
            return 1 + deep_ok(tag, n - 1)
        end

        function deep_fail(tag, n)
            if n <= 0 then
                error(tag)
            end
            return (deep_fail(tag, n - 1))
        end

        function shallow_after_deep()
            local v = deep_ok("ok", 270)
            local ok, _ = pcall(function()
                return deep_fail("caught", 270)
            end)
            assert(v == 271 and not ok)
            error("shallow_boom")
        end
    )",
                  {});
    for (auto backend: AllJitTypes()) {
        SCOPED_TRACE(JitTypeName(backend));
        int64_t ret = 0;
        try {
            Call(s, backend, "shallow_after_deep", ret);
            ADD_FAILURE() << "expected shallow_boom";
        } catch (const FakeluaException &e) {
            EXPECT_NE(e.LuaStack().find("in function 'shallow_after_deep'"), std::string::npos) << e.LuaStack();
            EXPECT_EQ(e.LuaStack().find("deep_ok"), std::string::npos) << e.LuaStack();
            EXPECT_EQ(e.LuaStack().find("deep_fail"), std::string::npos) << e.LuaStack();
        }
        try {
            Call(s, backend, "deep_fail", ret, std::string("overflow_boom"), int64_t{270});
            ADD_FAILURE() << "expected overflow_boom";
        } catch (const FakeluaException &e) {
            EXPECT_NE(e.LuaStack().find("... ("), std::string::npos) << e.LuaStack();
            EXPECT_NE(e.LuaStack().find("more frames)"), std::string::npos) << e.LuaStack();
        }
    }
}

// GCC 函数回调解释器闭包并在内层抛错时，解释器帧在前、外层 GCC 帧也保留在调用栈里。
TEST(exception, cross_engine_closure_throw_merges_gcc_and_interp_frames) {
    StateConfig sc;
    sc.log_level = LogLevel::Off;
    FakeluaStateGuard sg(sc);
    State *s = sg.GetState();
    CompileString(s,
                  "local function interp_leaf()\n"
                  "    error(\"cross_boom\")\n"
                  "end\n"
                  "function make_interp_closure()\n"
                  "    return function()\n"
                  "        return (interp_leaf())\n"
                  "    end\n"
                  "end\n"
                  "function gcc_invoker(fn)\n"
                  "    return (fn())\n"
                  "end\n"
                  "function run_cross()\n"
                  "    return 0\n"
                  "end\n",
                  {});
    CVar cl{};
    cl = CallByNameArgs(s, JIT_INTERP, "make_interp_closure", 0, nullptr);
    try {
        (void) CallByNameArgs(s, JIT_GCC, "gcc_invoker", 1, &cl);
        FAIL() << "expected cross_boom";
    } catch (const FakeluaException &e) {
        const std::string stack = e.LuaStack();
        const auto leaf = stack.find("in function 'interp_leaf'");
        const auto invoker = stack.find("in function 'gcc_invoker'");
        EXPECT_NE(leaf, std::string::npos) << e.what();
        EXPECT_NE(invoker, std::string::npos) << e.what();
        EXPECT_LT(leaf, invoker) << e.what();
        EXPECT_TRUE(e.Loc().ends_with(":2")) << e.Loc() << "\n" << e.what();
    }
}
