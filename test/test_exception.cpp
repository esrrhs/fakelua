#include "compile/compiler.h"
#include "fakelua.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(exception, function_param_duplicate) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_function_param_duplicate.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("duplicate") != std::string::npos);
    }
}

TEST(exception, function_param_local_duplicate) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_function_param_local_duplicate.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("duplicate") != std::string::npos);
    }
}

TEST(exception, local_define_duplicate) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_local_define_duplicate.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("duplicate") != std::string::npos);
    }
}

TEST(exception, const_define_duplicate) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_define_duplicate.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("duplicate") != std::string::npos);
    }
}

TEST(exception, const_define_func_param_duplicate) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_define_func_param_duplicate.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("duplicate") != std::string::npos);
    }
}

TEST(exception, const_define_no_match) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_define_no_match.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("match") != std::string::npos);
    }
}

TEST(exception, const_define_no_value) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_define_no_value.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("useless") != std::string::npos);
    }
}

TEST(exception, function_call_exception) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    L->compile_file("./exception/test_function_call_exception.lua", {});

    try {
        L->call("test", std::tie(), 1, 2, 3);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("not match") != std::string::npos);
    }

    try {
        L->call("test1", std::tie(), 1, 2, 3);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("not found") != std::string::npos);
    }
}

TEST(exception, variadic_function_call_exception) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    L->compile_file("./exception/test_variadic_function_call_exception.lua", {});

    try {
        L->call("test", std::tie(), 1);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("not match") != std::string::npos);
    }
}

TEST(exception, compile_fail) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_compile_fail.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("syntax error") != std::string::npos);
    }
}

TEST(exception, compile_no_file) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_no_file.lua", {});
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

TEST(exception, debug_assert_fail) {
    try {
        debug_assert_fail("false");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("assert fail") != std::string::npos);
    }
}

TEST(exception, replace_escape_chars) {
    try {
        std::string s = "\\e";
        replace_escape_chars(s);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("invalid escape sequence") != std::string::npos);
    }

    try {
        std::string s = "\\256";
        replace_escape_chars(s);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("decimal escape too large") != std::string::npos);
    }
}

TEST(exception, return_type_error_bool) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        bool ret = 0;
        L->call("test", std::tie(ret), 1);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_bool failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_char) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        char ret = 0;
        L->call("test", std::tie(ret), "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_char failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_uchar) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        unsigned char ret = 0;
        L->call("test", std::tie(ret), "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_uchar failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_short) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        short ret = 0;
        L->call("test", std::tie(ret), "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_short failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_ushort) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        unsigned short ret = 0;
        L->call("test", std::tie(ret), "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_ushort failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_int) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        int ret = 0;
        L->call("test", std::tie(ret), "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_int failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_uint) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        unsigned int ret = 0;
        L->call("test", std::tie(ret), "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_uint failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_long) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        long ret = 0;
        L->call("test", std::tie(ret), "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_long failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_ulong) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        unsigned long ret = 0;
        L->call("test", std::tie(ret), "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_ulong failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_long_long) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        long long ret = 0;
        L->call("test", std::tie(ret), "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_longlong failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_ulong_long) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        unsigned long long ret = 0;
        L->call("test", std::tie(ret), "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_ulonglong failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_float) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        float ret = 0;
        L->call("test", std::tie(ret), "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_float failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_double) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        double ret = 0;
        L->call("test", std::tie(ret), "1");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_double failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_cstr) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        const char *ret = 0;
        L->call("test", std::tie(ret), 123);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_cstr failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_str) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        char *ret = 0;
        L->call("test", std::tie(ret), 123);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_str failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_string) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        std::string ret;
        L->call("test", std::tie(ret), 123);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_string failed") != std::string::npos);
    }
}

TEST(exception, return_type_error_stringview) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        std::string_view ret;
        L->call("test", std::tie(ret), 123);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("fakelua_to_native_stringview failed") != std::string::npos);
    }
}

TEST(exception, return_index_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);
    L->compile_file("./exception/test_return_type_error.lua", {});

    try {
        int a = 0;
        int b = 0;
        L->call("test", std::tie(a, b), 123);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("type is VAR_NIL") != std::string::npos);
    }
}

TEST(exception, const_define_variadic) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_define_variadic.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("... can not be const") != std::string::npos);
    }
}

TEST(exception, test_binop_plus_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    L->compile_file("./exception/test_binop_plus_error.lua", {});
    try {
        L->call("test", std::tie(), 1, "a");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_plus_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_plus_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, to_integer) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    ASSERT_ANY_THROW(to_integer("9223372036854775808"));
    ASSERT_ANY_THROW(to_integer("-9223372036854775809"));
    ASSERT_ANY_THROW(to_integer("abc"));
}

TEST(exception, to_float) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    ASSERT_ANY_THROW(to_float("f"));
    ASSERT_ANY_THROW(to_float("1.7976931348623157e+309"));
}

TEST(exception, test_const_binop_minus_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_minus_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_star_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_star_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_slash_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_slash_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_double_slash_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_double_slash_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_pow_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_pow_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_mod_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_mod_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_bitand_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_bitand_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be integer") != std::string::npos);
    }
}

TEST(exception, test_const_binop_xor_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_xor_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be integer") != std::string::npos);
    }
}

TEST(exception, test_const_binop_bitor_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_bitor_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be integer") != std::string::npos);
    }
}

TEST(exception, test_const_binop_right_shift_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_right_shift_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be integer") != std::string::npos);
    }
}

TEST(exception, test_const_binop_left_shift_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_left_shift_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be integer") != std::string::npos);
    }
}

TEST(exception, test_const_binop_less_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_less_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_less_equal_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_less_equal_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_more_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_more_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_binop_more_equal_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_binop_more_equal_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_unop_minus_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_unop_minus_error.lua", {});
        int ret = 0;
        L->call("test", std::tie(ret), "abc");
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be number") != std::string::npos);
    }
}

TEST(exception, test_const_unop_len_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_unop_len_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be string or table") != std::string::npos);
    }
}

TEST(exception, test_const_unop_bitnot_error) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_const_unop_bitnot_error.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("must be integer") != std::string::npos);
    }
}

TEST(exception, test_label_exception) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        L->compile_file("./exception/test_label_exception.lua", {});
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("not support label") != std::string::npos);
    }
}

var *CALL_VAR_FUNC_TEST_EXCEPTION_FUNC(...) {
    return nullptr;
}

TEST(exception, test_call_var_func) {
    auto L = fakelua_newstate();
    ASSERT_NE(L.get(), nullptr);
    L->set_debug_log_level(0);

    try {
        std::vector<var *> args;
        args.resize(33);
        call_var_func(CALL_VAR_FUNC_TEST_EXCEPTION_FUNC, args);
        ASSERT_TRUE(false);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        ASSERT_TRUE(std::string(e.what()).find("too many arguments") != std::string::npos);
    }
}
