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
