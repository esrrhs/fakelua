#include "compile/compiler.h"
#include "fakelua.h"
#include "util/debug.h"
#include "util/string_util.h"
#include "gtest/gtest.h"

using namespace fakelua;

#define TEST_JIT_TYPE JIT_GCC

// 禁用 JIT_TCC 说明：
// 1. TCC 编译出来的动态内存帧（JIT 编译生成的机器码）中缺少 DWARF 异常展开表（.eh_frame），
//    一旦在此类动态帧中抛出 C++ 异常，运行时无法查找到对应的 Catch 块，会直接触发 std::terminate() 导致进程崩溃。
// 2. 考虑到 TCC 主要是作为我们开发测试期间使用的轻量级 JIT 编译器，没有必要在生产级别支持复杂的异常处理，
//    因此在这里全局禁用 JIT_TCC，所有的 JIT 异常流程测试均强制使用 GCC JIT（即 TEST_JIT_TYPE 设置为 JIT_GCC）。
// 这样可以规避为了兼容 TCC 动态异常处理而引入复杂的 setjmp/longjmp 结构，保持底层内存及 JIT 设计的纯粹性与高可读性。
static inline void CompileFileTccDisabled(State *s, const std::string &file, const CompileConfig &cfg = {}) {
    CompileConfig my_cfg = cfg;
    my_cfg.disable_jit[JIT_TCC] = true;
    (CompileFile)(s, file, my_cfg);
}
#define CompileFile CompileFileTccDisabled


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
        CompileFile(s, "./exception/test_const_binop_plus_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_minus_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_star_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_slash_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_double_slash_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_pow_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_mod_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_bitand_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_xor_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_bitor_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_right_shift_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_left_shift_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_less_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_less_equal_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_more_error.lua", {});
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
        CompileFile(s, "./exception/test_const_binop_more_equal_error.lua", {});
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
        CompileFile(s, "./exception/test_const_unop_len_error.lua", {});
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
        CompileFile(s, "./exception/test_const_unop_bitnot_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("attempt to perform bitwise") != std::string::npos);
    }
}


TEST(exception, stmt_support_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_stmt_support_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("goto is not supported") != std::string::npos);
    }
}

TEST(exception, label_support_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_label_support_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("label is not supported") != std::string::npos);
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
        ASSERT_TRUE(std::string(e.what()).find("compile failed") != std::string::npos);
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
        ASSERT_TRUE(std::string(e.what()).find("compile failed") != std::string::npos);
    }
}

TEST(exception, test_for_in_namelist_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_for_in_namelist_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("for in namelist size must be 1 or 2, but got") != std::string::npos);
    }
}

TEST(exception, test_for_in_exp_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_for_in_exp_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must have exactly one argument") != std::string::npos);
    }
}

TEST(exception, test_for_in_pairs_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_for_in_pairs_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("for in expression must be a function call") != std::string::npos);
    }
}

TEST(exception, test_for_in_explist_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_for_in_explist_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("for in explist size must be 1, but got") != std::string::npos);
    }
}

TEST(exception, test_for_in_prefix_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_for_in_prefix_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("for in expression must be a pairs() or ipairs() call") != std::string::npos);
    }
}

TEST(exception, test_for_in_func_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_for_in_func_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("only pairs() or ipairs() are supported") != std::string::npos);
    }
}

TEST(exception, test_for_in_func_args_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_for_in_func_args_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("argument must be an expression list") != std::string::npos);
    }
}

TEST(exception, test_for_in_prefix_func_error) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_for_in_prefix_func_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("only pairs() or ipairs() are supported") != std::string::npos);
    }
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
