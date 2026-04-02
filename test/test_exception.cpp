#include "compile/compiler.h"
#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

int noop;

TEST(exception, function_param_duplicate) {
    auto s = FakeluaNewState();
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

TEST(exception, function_param_local_duplicate) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_function_param_local_duplicate.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("duplicate") != std::string::npos);
    }
}

TEST(exception, local_define_duplicate) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_local_define_duplicate.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("duplicate") != std::string::npos);
    }
}

TEST(exception, const_define_duplicate) {
    auto s = FakeluaNewState();
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
    auto s = FakeluaNewState();
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
    auto s = FakeluaNewState();
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

TEST(exception, const_define_no_value) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_define_no_value.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("useless") != std::string::npos);
    }
}

TEST(exception, function_call_exception) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    CompileFile(s, "./exception/test_function_call_exception.lua", {});

    try {
        Call(s, "test", noop, 1, 2, 3);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("not match") != std::string::npos);
    }

    try {
        Call(s, "test1", noop, 1, 2, 3);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("not found") != std::string::npos);
    }
}

TEST(exception, variadic_function_call_exception) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    CompileFile(s, "./exception/test_variadic_function_call_exception.lua", {});

    try {
        Call(s, "test", noop, 1);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("not match") != std::string::npos);
    }
}

TEST(exception, compile_fail) {
    auto s = FakeluaNewState();
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
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_no_file.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("open file failed") != std::string::npos);
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
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        bool ret = 0;
        Call(s, "test", ret, 1);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeBool failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_char) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        char ret = 0;
        Call(s, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeChar failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_uchar) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        unsigned char ret = 0;
        Call(s, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUchar failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_short) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        short ret = 0;
        Call(s, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeShort failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_ushort) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        unsigned short ret = 0;
        Call(s, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUshort failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_int) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        int ret = 0;
        Call(s, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeInt failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_uint) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        unsigned int ret = 0;
        Call(s, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUint failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_long) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        long ret = 0;
        Call(s, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeLong failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_ulong) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        unsigned long ret = 0;
        Call(s, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUlong failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_long_long) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        long long ret = 0;
        Call(s, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeLonglong failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_ulong_long) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        unsigned long long ret = 0;
        Call(s, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeUlonglong failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_float) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        float ret = 0;
        Call(s, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeFloat failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_double) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        double ret = 0;
        Call(s, "test", ret, "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeDouble failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_cstr) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        const char *ret = 0;
        Call(s, "test", ret, 123);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeCstr failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_str) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        char *ret = 0;
        Call(s, "test", ret, 123);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeStr failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_string) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        std::string ret;
        Call(s, "test", ret, 123);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeString failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_stringview) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        std::string_view ret;
        Call(s, "test", ret, 123);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("FakeluaToNativeStringview failed") != std::string::npos);
    }
}

TEST(exception, return_index_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);
    CompileFile(s, "./exception/test_return_type_error.lua", {});

    try {
        int a = 0;
        Call(s, "test", a, 123);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("type is Nil") != std::string::npos);
    }
}

TEST(exception, const_define_variadic) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_define_variadic.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("... can not be const") != std::string::npos);
    }
}

TEST(exception, test_binop_plus_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    CompileFile(s, "./exception/test_binop_plus_error.lua", {});
    try {
        Call(s, "test", noop, 1, "a");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_plus_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_plus_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, ToInteger) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    ASSERT_ANY_THROW(ToInteger("9223372036854775808"));
    ASSERT_ANY_THROW(ToInteger("-9223372036854775809"));
    ASSERT_ANY_THROW(ToInteger("abc"));
}

TEST(exception, ToFloat) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    ASSERT_ANY_THROW(ToFloat("f"));
    ASSERT_ANY_THROW(ToFloat("1.7976931348623157e+309"));
}

TEST(exception, test_const_binop_minus_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_minus_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_star_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_star_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_slash_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_slash_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_double_slash_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_double_slash_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_pow_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_pow_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_mod_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_mod_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_bitand_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_bitand_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be integer") != std::string::npos);
    }
}

TEST(exception, test_const_binop_xor_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_xor_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be integer") != std::string::npos);
    }
}

TEST(exception, test_const_binop_bitor_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_bitor_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be integer") != std::string::npos);
    }
}

TEST(exception, test_const_binop_right_shift_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_right_shift_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be integer") != std::string::npos);
    }
}

TEST(exception, test_const_binop_left_shift_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_left_shift_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be integer") != std::string::npos);
    }
}

TEST(exception, test_const_binop_less_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_less_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_less_equal_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_less_equal_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_more_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_more_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_more_equal_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_binop_more_equal_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_unop_minus_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_unop_minus_error.lua", {});
        int ret = 0;
        Call(s, "test", ret, "abc");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_unop_len_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_unop_len_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be string or table") != std::string::npos);
    }
}

TEST(exception, test_const_unop_bitnot_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_unop_bitnot_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be integer") != std::string::npos);
    }
}

TEST(exception, test_label_exception) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_label_exception.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("not support label") != std::string::npos);
    }
}

Var *CALL_VAR_FUNC_TEST_EXCEPTION_FUNC(...) {
    return nullptr;
}

TEST(exception, test_call_var_func) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        std::vector<Var *> args;
        args.resize(33);
        CallVarFunc(CALL_VAR_FUNC_TEST_EXCEPTION_FUNC, args);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("too many arguments") != std::string::npos);
    }
}

TEST(exception, TableSet) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_table_set_error.lua", {});
        int ret = 0;
        Call(s, "test", ret, "abc");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be table") != std::string::npos);
    }
}

TEST(exception, TableGet) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_table_get_error.lua", {});
        int ret = 0;
        Call(s, "test", ret, "abc");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be table") != std::string::npos);
    }
}

TEST(exception, table_loop) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_table_loop_error.lua", {});
        int ret = 0;
        Call(s, "test", ret, "abc");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be table") != std::string::npos);
    }
}

TEST(exception, stmt_support_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_stmt_support_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("not support stmt type") != std::string::npos);
    }
}

TEST(exception, const_func_call_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_const_func_call_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("functioncall can not be const") != std::string::npos);
    }
}

TEST(exception, no_define_lvalue_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_no_define_lvalue_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("can not find var") != std::string::npos);
    }
}

TEST(exception, global_duplicate_lvalue_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_global_duplicate_lvalue_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("the const define name is duplicated") != std::string::npos);
    }
}

TEST(exception, test_break_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_break_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("break must in loop") != std::string::npos);
    }
}

TEST(exception, test_for_in_exp_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_for_in_exp_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("for in ipairs() or pairs() args size must be 1, but got") != std::string::npos);
    }
}

TEST(exception, test_for_in_namelist_error) {
    auto s = FakeluaNewState();
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

TEST(exception, test_for_in_pairs_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_for_in_pairs_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("for in exp (expect functioncall) must be ipairs() or pairs()") != std::string::npos);
    }
}

TEST(exception, test_for_in_explist_error) {
    auto s = FakeluaNewState();
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
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_for_in_prefix_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("for in exp (expect prefixexp) must be ipairs() or pairs()") != std::string::npos);
    }
}

TEST(exception, test_for_in_func_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_for_in_func_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("for in exp (expect ipairs/pairs) must be ipairs() or pairs()") != std::string::npos);
    }
}

TEST(exception, test_for_in_func_args_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_for_in_func_args_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("for in ipairs() or pairs() args size must be 1, but got") != std::string::npos);
    }
}

TEST(exception, test_for_in_prefix_func_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_for_in_prefix_func_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("for in exp (expect ipairs/pairs) must be ipairs() or pairs()") != std::string::npos);
    }
}

TEST(exception, test_col_func_not_find_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_col_func_not_find_error.lua", {});
        int ret = 0;
        Call(s, "test", ret, "abc");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("CallVar: function xxx not found") != std::string::npos);
    }
}

TEST(exception, test_col_func_param_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_col_func_param_error.lua", {});
        int ret = 0;
        Call(s, "test", ret, "abc");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("expect 2 params, but got") != std::string::npos);
    }
}

TEST(exception, test_col_func_table_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_col_func_table_error.lua", {});
        int ret = 0;
        Call(s, "test", ret, "abc");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("CallVar: colon func must be table type, but got") != std::string::npos);
    }
}

TEST(exception, test_col_func_type_error) {
    auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetDebugLogLevel(0);

    try {
        CompileFile(s, "./exception/test_col_func_type_error.lua", {});
        int ret = 0;
        Call(s, "test", ret, "abc");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("CallVar: func must be string type, but got") != std::string::npos);
    }
}
