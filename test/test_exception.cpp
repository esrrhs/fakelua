#include "compile/compiler.h"
#include "fakelua.h"
#include "util/debug.h"
#include "util/string_util.h"
#include "gtest/gtest.h"

using namespace fakelua;

#define TEST_JIT_TYPE JIT_TCC

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
    cfg.disable_jit[JIT_TCC] = true;
    cfg.disable_jit[JIT_GCC] = true;
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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

    CompileFile(s, "./exception/test_function_call_exception.lua", {});

    try {
        CVar ret;
        Call(s, TEST_JIT_TYPE, "test", ret, 1, 2, 3);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("not match") != std::string::npos);
    }

    try {
        CVar ret;
        Call(s, TEST_JIT_TYPE, "test1", ret, 1, 2, 3);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("not found") != std::string::npos);
    }
}

TEST(exception, compile_fail) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        bool ret = 0;
        Call(s, TEST_JIT_TYPE, "test", ret, 1);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeBool failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_char) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        char ret = 0;
        Call(s, TEST_JIT_TYPE, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeChar failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_uchar) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        unsigned char ret = 0;
        Call(s, TEST_JIT_TYPE, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUchar failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_short) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        short ret = 0;
        Call(s, TEST_JIT_TYPE, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeShort failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_ushort) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        unsigned short ret = 0;
        Call(s, TEST_JIT_TYPE, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUshort failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_int) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        int ret = 0;
        Call(s, TEST_JIT_TYPE, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeInt failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_uint) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        unsigned int ret = 0;
        Call(s, TEST_JIT_TYPE, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUint failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_long) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        long ret = 0;
        Call(s, TEST_JIT_TYPE, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeLong failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_ulong) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        unsigned long ret = 0;
        Call(s, TEST_JIT_TYPE, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUlong failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_long_long) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        long long ret = 0;
        Call(s, TEST_JIT_TYPE, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeLonglong failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_ulong_long) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        unsigned long long ret = 0;
        Call(s, TEST_JIT_TYPE, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUlonglong failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_float) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        float ret = 0;
        Call(s, TEST_JIT_TYPE, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeFloat failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_double) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        double ret = 0;
        Call(s, TEST_JIT_TYPE, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeDouble failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_string) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        std::string ret;
        Call(s, TEST_JIT_TYPE, "test", ret, 123);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeString failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_stringview) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        std::string_view ret;
        Call(s, TEST_JIT_TYPE, "test", ret, 123);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeStringview failed") != std::string::npos);
    }
}

TEST(exception, const_define_variadic) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);
    EXPECT_THROW(CompileFile(s, "./exception/test_goto_skip_single_local.lua", {}), std::exception);
}

TEST(exception, goto_skip_multiple_locals) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    EXPECT_THROW(CompileFile(s, "./exception/test_goto_skip_local.lua", {}), std::exception);
}

TEST(exception, goto_nonexistent_label) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    EXPECT_THROW(CompileFile(s, "./exception/test_goto_nonexistent_label.lua", {}), std::exception);
}

TEST(exception, goto_cross_function) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    EXPECT_THROW(CompileFile(s, "./exception/test_goto_cross_function.lua", {}), std::exception);
}

TEST(exception, goto_sibling_nested) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    EXPECT_THROW(CompileFile(s, "./exception/test_goto_sibling_nested.lua", {}), std::exception);
}

TEST(exception, goto_elseif_nonexistent) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    EXPECT_THROW(CompileFile(s, "./exception/test_goto_elseif_nonexistent.lua", {}), std::exception);
}

// 文件级只承载声明：local 定义、函数定义，以及可选的首行 package 声明。
// 其余可执行语句在 SemanticAnalysis::CheckFileLevelStmts 里就应该被拒绝，
// 不能被悄悄搬进 __fakelua_init。
TEST(exception, file_level_stmt_rejected) {
    for (const char *file: {"./exception/test_top_level_if.lua", "./exception/test_top_level_while.lua",
                            "./exception/test_top_level_repeat.lua", "./exception/test_top_level_for.lua",
                            "./exception/test_top_level_for_in.lua", "./exception/test_top_level_do_end.lua",
                            "./exception/test_top_level_assign.lua", "./exception/test_top_level_table_assign.lua",
                            "./exception/test_top_level_function_call.lua", "./exception/test_top_level_return.lua",
                            "./exception/test_top_level_goto.lua", "./exception/test_top_level_package_not_first.lua"}) {
        SCOPED_TRACE(file);
        FakeluaStateGuard sg;
        auto s = sg.GetState();
        ASSERT_NE(s, nullptr);
        SetDebugLogLevel(0);
        EXPECT_THROW(CompileFile(s, file, {}), std::exception);
    }
}

// 文件级语句被拒绝时，报错信息应指明是文件级语句问题并带上出错位置。
TEST(exception, file_level_stmt_error_message) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

    ASSERT_NO_THROW(CompileFile(s, "./exception/test_const_func_call_error.lua", {}));
}

TEST(exception, no_define_lvalue_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_no_define_lvalue_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("TCC compile") != std::string::npos);
    }
}

TEST(exception, global_duplicate_lvalue_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_break_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("TCC compile") != std::string::npos);
    }
}

TEST(exception, test_continue_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);
    EXPECT_THROW(CompileFile(s, "./exception/test_continue_skip_local.lua", {}), std::exception);
}

TEST(exception, function_too_many_params) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_function_too_many_params.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("function input parameters exceed limit 32") != std::string::npos);
    }
}

TEST(exception, math_param_non_numeric_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_math_param_non_numeric_error.lua", {});

    try {
        CVar ret;
        Call(s, JIT_GCC, "test", ret, "hello", 1);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("bad argument #1 (a): attempt to perform arithmetic on non-numeric value") != std::string::npos);
    }
}

TEST(exception, const_no_init) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    EXPECT_THROW(CompileFile(s, "./exception/test_const_no_init.lua", {}), std::exception);
}

TEST(exception, const_reassign) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    EXPECT_THROW(CompileFile(s, "./exception/test_const_reassign.lua", {}), std::exception);
}

TEST(exception, top_level_bare_local) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    EXPECT_THROW(CompileFile(s, "./exception/test_top_level_bare_local.lua", {}), std::exception);
}

TEST(exception, test_spec_duplicate_keys) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

    EXPECT_NO_THROW(CompileFile(s, "./exception/test_table_var_func_call.lua", {}));
}

TEST(exception, vararg_nested_function) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_vararg_with_nested_function.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("not support stmt type") != std::string::npos);
    }
}

TEST(exception, vararg_nested_localfunction) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    EXPECT_NO_THROW(CompileFile(s, "./exception/test_vararg_with_nested_localfunction.lua", {}));
}

TEST(exception, vararg_funcdef) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    EXPECT_NO_THROW(CompileFile(s, "./exception/test_vararg_with_funcdef.lua", {}));
}

TEST(exception, for_loop_zero_step_int) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_spec_call_arg_count_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("wrong number of arguments") != std::string::npos);
    }
}

TEST(exception, set_table_arg_count_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_spec_assign_nonnumeric_float_throws.lua", {});

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    ASSERT_NE(code.find("int64_t x = ((n) + (1))"), std::string::npos);
    // CVar guard – int branch.
    ASSERT_NE(code.find(".type_ == VAR_INT)"), std::string::npos);
    // CVar guard – float branch casts to int64_t.
    ASSERT_NE(code.find("(int64_t)"), std::string::npos);
    // Error branch for non-numeric CVar.
    ASSERT_NE(code.find("attempt to assign non-numeric value to typed int variable"), std::string::npos);
    // Return uses native addition.
    ASSERT_NE(code.find("return ((n) + (x))"), std::string::npos);

    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_spec_assign_nonnumeric_int_throws.lua", {});

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
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
    SetDebugLogLevel(0);

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
    SetDebugLogLevel(0);

    CompileFile(s, "./exception/test_const_table_modify_error.lua", {});

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        double res = 0;
        EXPECT_THROW(Call(s, jit_type, "test_modify_const", res), std::exception);
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
    SetDebugLogLevel(0);

    // b 尚未定义，加法在 __fakelua_init 里就会失败
    EXPECT_THROW(CompileString(s, "local a = 1\nlocal b = a + b\n", {}), FakeluaException);
}

TEST(exception, call_runtime_error_is_catchable) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    CompileString(s, "function call_non_function()\nlocal s = \"hello\"\nreturn string.sub(s, 1, 3)(s)\nend", {});

    for (auto jit_type: {JIT_TCC, JIT_GCC}) {
        double res = 0;
        EXPECT_THROW(Call(s, jit_type, "call_non_function", res), FakeluaException);
    }
}
