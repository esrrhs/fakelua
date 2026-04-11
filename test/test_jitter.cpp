#include "compile/compiler.h"
#include "fakelua.h"
#include "var/var_string.h"
#include "var/var_table.h"
#include "gtest/gtest.h"

using namespace fakelua;

static void JitterRunHelper(const std::function<void(State *, JITType, bool)> &f) {
    const auto s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    f(s, JIT_TCC, true);
    f(s, JIT_TCC, false);
}

TEST(jitter, empty_file) {
    JitterRunHelper(
            [](State *s, JITType type, bool debug_mode) { CompileFile(s, "./jit/test_empty_file.lua", {.debug_mode = debug_mode}); });
}

TEST(jitter, empty_func) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_empty_func.lua", {.debug_mode = debug_mode});
        CVar ret;
        const auto v = static_cast<Var &>(ret);
        Call(s, type, "test", ret);
        ASSERT_EQ(v.Type(), VarType::Nil);
    });
}

TEST(jitter, empty_local_func) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_empty_local_func.lua", {.debug_mode = debug_mode});
        CVar ret;
        const auto v = static_cast<Var &>(ret);
        Call(s, type, "test", ret);
        ASSERT_EQ(v.Type(), VarType::Nil);
    });
}

// Multi-return is not supported yet, these tests verify the exception is thrown
TEST(jitter, multi_return) {
    EXPECT_THROW(
            {
                const auto s = FakeluaNewState();
                CompileFile(s, "./jit/test_multi_return.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, multi_return_call) {
    EXPECT_THROW(
            {
                const auto s = FakeluaNewState();
                CompileFile(s, "./jit/test_multi_return_call.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, multi_return_call_ex) {
    EXPECT_THROW(
            {
                const auto s = FakeluaNewState();
                CompileFile(s, "./jit/test_multi_return_call_ex.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, multi_return_sub) {
    EXPECT_THROW(
            {
                const auto s = FakeluaNewState();
                CompileFile(s, "./jit/test_multi_return_sub.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, multi_return_multi) {
    EXPECT_THROW(
            {
                const auto s = FakeluaNewState();
                CompileFile(s, "./jit/test_multi_return_multi.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, multi_return_multi_ex) {
    EXPECT_THROW(
            {
                const auto s = FakeluaNewState();
                CompileFile(s, "./jit/test_multi_return_multi_ex.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, multi_name) {
    EXPECT_THROW(
            {
                const auto s = FakeluaNewState();
                CompileFile(s, "./jit/test_multi_name_func.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, multi_col_name) {
    EXPECT_THROW(
            {
                const auto s = FakeluaNewState();
                CompileFile(s, "./jit/test_multi_col_name_func.lua", {.debug_mode = true});
            },
            std::exception);
}

// Global const variable definitions
TEST(jitter, const_define) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_const_define.lua", {.debug_mode = debug_mode});
        int i = 0;
        Call(s, type, "test", i);
        ASSERT_EQ(i, 1);
    });
}

TEST(jitter, multi_const_define) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_multi_const_define.lua", {.debug_mode = debug_mode});

        CVar ret0 = {};
        auto v = (Var &) ret0;
        int ret1 = 0;
        bool ret2 = false;
        bool ret3 = false;
        std::string ret4;
        double ret5;
        Call(s, type, "test0", ret0);
        Call(s, type, "test1", ret1);
        Call(s, type, "test2", ret2);
        Call(s, type, "test3", ret3);
        Call(s, type, "test4", ret4);
        Call(s, type, "test5", ret5);

        ASSERT_EQ(v.Type(), VarType::Nil);
        ASSERT_EQ(ret1, 1);
        ASSERT_EQ(ret2, false);
        ASSERT_EQ(ret3, true);
        ASSERT_EQ(ret4, "test");
        ASSERT_NEAR(ret5, 2.3, 0.001);
    });
}
//
TEST(jitter, empty_func_with_params) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_empty_func_with_params.lua", {.debug_mode = debug_mode});

        int ret1 = 0;
        bool ret2 = false;
        std::string ret3;
        double ret4 = 0;
        Call(s, type, "test1", ret1, 2.3, "test", true, 1);
        Call(s, type, "test2", ret2, 2.3, "test", true, 1);
        Call(s, type, "test3", ret3, 2.3, "test", true, 1);
        Call(s, type, "test4", ret4, 2.3, "test", true, 1);
        ASSERT_EQ(ret1, 1);
        ASSERT_EQ(ret2, true);
        ASSERT_EQ(ret3, "test");
        ASSERT_NEAR(ret4, 2.3, 0.001);
    });
}

// Variadic functions (...) are not supported yet, these tests verify the exception is thrown
TEST(jitter, variadic_func) {
    EXPECT_THROW(
            {
                const auto s = FakeluaNewState();
                CompileFile(s, "./jit/test_variadic_func.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, variadic_func_with_params) {
    EXPECT_THROW(
            {
                const auto s = FakeluaNewState();
                CompileFile(s, "./jit/test_variadic_func_with_params.lua", {.debug_mode = true});
            },
            std::exception);
}

TEST(jitter, string) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileString(s, "function test() return 1 end", {.debug_mode = debug_mode});
        int ret = 0;
        Call(s, type, "test", ret);
        ASSERT_EQ(ret, 1);
    });
}
//
TEST(jitter, local_define) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_local_define.lua", {.debug_mode = debug_mode});
        CVar ret;
        const auto v = static_cast<Var &>(ret);
        Call(s, type, "test", ret);
        ASSERT_EQ(v.Type(), VarType::Nil);
    });
}

TEST(jitter, local_define_with_values) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_local_define_with_value.lua", {.debug_mode = debug_mode});
        bool ret1 = false;
        int ret2 = 0;
        int ret3 = 0;
        std::string ret4;
        CVar ret5;
        const auto v5 = static_cast<Var &>(ret5);
        Call(s, type, "test1", ret1, true, 2);
        Call(s, type, "test2", ret2, true, 2);
        Call(s, type, "test3", ret3, true, 2);
        Call(s, type, "test4", ret4, true, 2);
        Call(s, type, "test5", ret5, true, 2);
        ASSERT_EQ(ret1, true);
        ASSERT_EQ(ret2, 2);
        ASSERT_EQ(ret3, 1);
        ASSERT_EQ(ret4, "test");
        ASSERT_EQ(v5.Type(), VarType::Nil);
    });
}
//
TEST(jitter, test_assign) {
    JitterRunHelper([](State *s, JITType type, bool debug_mode) {
        CompileFile(s, "./jit/test_assign.lua", {.debug_mode = debug_mode});
        int a = 0;
        std::string b;
        Call(s, type, "test1", a, true, 1.1);
        Call(s, type, "test2", b, true, 1.1);
        ASSERT_EQ(a, 1);
        ASSERT_EQ(b, "2");
    });
}
//
// TEST(jitter, test_assign_not_match) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     std::string b;
//     int c = 0;
//     int d = 0;
//     L->CompileFile("./jit/test_assign_not_match.lua", {});
//     L->call("test", std::tie(a, b, c, d), true, "2", 1.1, 1);
//     ASSERT_EQ(a, 1);
//     ASSERT_EQ(b, "2");
//     ASSERT_EQ(c, 3);
//     ASSERT_EQ(d, 4);
//
//     a = 0;
//     b.clear();
//     L->CompileFile("./jit/test_assign_not_match.lua", {.debug_mode = false});
//     L->call("test", std::tie(a, b, c, d), true, "2", 1.1, 1);
//     ASSERT_EQ(a, 1);
//     ASSERT_EQ(b, "2");
//     ASSERT_EQ(c, 3);
//     ASSERT_EQ(d, 4);
// }
//
// TEST(jitter, test_assign_variadic_match) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     std::string a;
//     int b = 0;
//     L->CompileFile("./jit/test_assign_variadic.lua", {});
//     L->call("test", std::tie(a, b), 1, "2");
//     ASSERT_EQ(a, "2");
//     ASSERT_EQ(b, 1);
//
//     a.clear();
//     b = 0;
//     L->CompileFile("./jit/test_assign_variadic.lua", {.debug_mode = false});
//     L->call("test", std::tie(a, b), 1, "2");
//     ASSERT_EQ(a, "2");
//     ASSERT_EQ(b, 1);
// }
//
// TEST(jitter, test_assign_variadic_no_match) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     int b = 0;
//     L->CompileFile("./jit/test_assign_variadic_no_match.lua", {});
//     L->call("test", std::tie(a, b), 2, "2");
//     ASSERT_EQ(a, 1);
//     ASSERT_EQ(b, 2);
//
//     a = 0;
//     b = 0;
//     L->CompileFile("./jit/test_assign_variadic_no_match.lua", {.debug_mode = false});
//     L->call("test", std::tie(a, b), 2, "2");
//     ASSERT_EQ(a, 1);
//     ASSERT_EQ(b, 2);
// }
//
// TEST(jitter, test_assign_variadic_empty) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     CVar b = {};
//     auto vb = (Var &) b;
//     L->CompileFile("./jit/test_assign_variadic_no_match.lua", {});
//     L->call("test", std::tie(a, b));
//     ASSERT_EQ(a, 1);
//     ASSERT_EQ(vb.Type(), VarType::Nil);
//
//     a = 0;
//     b = {};
//     L->CompileFile("./jit/test_assign_variadic_no_match.lua", {.debug_mode = false});
//     L->call("test", std::tie(a, b));
//     ASSERT_EQ(a, 1);
//     ASSERT_EQ(vb.Type(), VarType::Nil);
// }
//
// TEST(jitter, test_const_table) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//     std::vector<VarInterface *> tmp;
//     auto newfunc = [&]() {
//         auto ret = new SimpleVarImpl();
//         tmp.push_back(ret);
//         return ret;
//     };
//     L->SetVarInterfaceNewFunc(newfunc);
//
//     VarInterface *t1 = nullptr;
//     VarInterface *t2 = nullptr;
//     VarInterface *t3 = nullptr;
//     L->CompileFile("./jit/test_const_table.lua", {});
//     L->call("test", std::tie(t1, t2, t3));
//     ASSERT_NE(t1, nullptr);
//     ASSERT_EQ(t1->ViGetType(), VarInterface::Type::TABLE);
//     ASSERT_NE(t2, nullptr);
//     ASSERT_EQ(t2->ViGetType(), VarInterface::Type::TABLE);
//     ASSERT_NE(t3, nullptr);
//     ASSERT_EQ(t3->ViGetType(), VarInterface::Type::TABLE);
//
//     // need sort kv
//     dynamic_cast<SimpleVarImpl *>(t1)->ViSortTable();
//     dynamic_cast<SimpleVarImpl *>(t2)->ViSortTable();
//     dynamic_cast<SimpleVarImpl *>(t3)->ViSortTable();
//     ASSERT_EQ(t1->ViToString(0),
//               "table:\n\t[1] = 1\n\t[2] = 2\n\t[3] = 3\n\t[4] = 4\n\t[5] = 5\n\t[6] = 6\n\t[7] = 7\n\t[8] = 8\n\t[9] = 9\n\t[10] = 10");
//     ASSERT_EQ(t2->ViToString(0), "table:\n\t[\"a\"] = 1\n\t[\"b\"] = 2\n\t[\"c\"] = 3");
//     ASSERT_EQ(t3->ViToString(0), "table:\n\t[1] = 1\n\t[2] = 3\n\t[3] = 5\n\t[\"b\"] = 2\n\t[\"d\"] = 4");
//
//     t1 = nullptr;
//     t2 = nullptr;
//     t3 = nullptr;
//     L->CompileFile("./jit/test_const_table.lua", {.debug_mode = false});
//     L->call("test", std::tie(t1, t2, t3));
//     ASSERT_NE(t1, nullptr);
//     ASSERT_EQ(t1->ViGetType(), VarInterface::Type::TABLE);
//     ASSERT_NE(t2, nullptr);
//     ASSERT_EQ(t2->ViGetType(), VarInterface::Type::TABLE);
//     ASSERT_NE(t3, nullptr);
//     ASSERT_EQ(t3->ViGetType(), VarInterface::Type::TABLE);
//
//     // need sort kv
//     dynamic_cast<SimpleVarImpl *>(t1)->ViSortTable();
//     dynamic_cast<SimpleVarImpl *>(t2)->ViSortTable();
//     dynamic_cast<SimpleVarImpl *>(t3)->ViSortTable();
//     ASSERT_EQ(t1->ViToString(0),
//               "table:\n\t[1] = 1\n\t[2] = 2\n\t[3] = 3\n\t[4] = 4\n\t[5] = 5\n\t[6] = 6\n\t[7] = 7\n\t[8] = 8\n\t[9] = 9\n\t[10] = 10");
//     ASSERT_EQ(t2->ViToString(0), "table:\n\t[\"a\"] = 1\n\t[\"b\"] = 2\n\t[\"c\"] = 3");
//     ASSERT_EQ(t3->ViToString(0), "table:\n\t[1] = 1\n\t[2] = 3\n\t[3] = 5\n\t[\"b\"] = 2\n\t[\"d\"] = 4");
//
//     for (auto &i: tmp) {
//         delete i;
//     }
// }
//
// TEST(jitter, test_const_nested_table) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//     std::vector<VarInterface *> tmp;
//     auto newfunc = [&]() {
//         auto ret = new SimpleVarImpl();
//         tmp.push_back(ret);
//         return ret;
//     };
//     L->SetVarInterfaceNewFunc(newfunc);
//
//     VarInterface *t = nullptr;
//     L->CompileFile("./jit/test_const_nested_table.lua", {});
//     L->call("test", std::tie(t));
//     ASSERT_NE(t, nullptr);
//     ASSERT_EQ(t->ViGetType(), VarInterface::Type::TABLE);
//
//     // need sort kv
//     dynamic_cast<SimpleVarImpl *>(t)->ViSortTable();
//     ASSERT_EQ(t->ViToString(0), "table:\n\t[\"array\"] = table:\n\t\t[1] = 1\n\t\t[2] = 2\n\t\t[3] = 3\n\t[\"map\"] = table:\n\t\t[\"a\"] "
//                                 "= 1\n\t\t[\"b\"] = 2\n\t\t[\"c\"] = 3");
//
//     t = nullptr;
//     L->CompileFile("./jit/test_const_nested_table.lua", {.debug_mode = false});
//     L->call("test", std::tie(t));
//     ASSERT_NE(t, nullptr);
//     ASSERT_EQ(t->ViGetType(), VarInterface::Type::TABLE);
//
//     // need sort kv
//     dynamic_cast<SimpleVarImpl *>(t)->ViSortTable();
//     ASSERT_EQ(t->ViToString(0), "table:\n\t[\"array\"] = table:\n\t\t[1] = 1\n\t\t[2] = 2\n\t\t[3] = 3\n\t[\"map\"] = table:\n\t\t[\"a\"] "
//                                 "= 1\n\t\t[\"b\"] = 2\n\t\t[\"c\"] = 3");
//
//     for (auto &i: tmp) {
//         delete i;
//     }
// }
//
// TEST(jitter, test_local_table) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//     std::vector<VarInterface *> tmp;
//     auto newfunc = [&]() {
//         auto ret = new SimpleVarImpl();
//         tmp.push_back(ret);
//         return ret;
//     };
//     L->SetVarInterfaceNewFunc(newfunc);
//
//     VarInterface *t1 = nullptr;
//     VarInterface *t2 = nullptr;
//     VarInterface *t3 = nullptr;
//     L->CompileFile("./jit/test_local_table.lua", {});
//     L->call("test", std::tie(t1, t2, t3));
//     ASSERT_NE(t1, nullptr);
//     ASSERT_EQ(t1->ViGetType(), VarInterface::Type::TABLE);
//     ASSERT_NE(t2, nullptr);
//     ASSERT_EQ(t2->ViGetType(), VarInterface::Type::TABLE);
//     ASSERT_NE(t3, nullptr);
//     ASSERT_EQ(t3->ViGetType(), VarInterface::Type::TABLE);
//
//     // need sort kv
//     dynamic_cast<SimpleVarImpl *>(t1)->ViSortTable();
//     dynamic_cast<SimpleVarImpl *>(t2)->ViSortTable();
//     dynamic_cast<SimpleVarImpl *>(t3)->ViSortTable();
//     ASSERT_EQ(t1->ViToString(0),
//               "table:\n\t[1] = 1\n\t[2] = 2\n\t[3] = 3\n\t[4] = 4\n\t[5] = 5\n\t[6] = 6\n\t[7] = 7\n\t[8] = 8\n\t[9] = 9\n\t[10] = 10");
//     ASSERT_EQ(t2->ViToString(0), "table:\n\t[\"a\"] = 1\n\t[\"b\"] = 2\n\t[\"c\"] = 3");
//     ASSERT_EQ(t3->ViToString(0), "table:\n\t[1] = 1\n\t[2] = 3\n\t[3] = 5\n\t[\"b\"] = 2\n\t[\"d\"] = 4");
//
//     t1 = nullptr;
//     t2 = nullptr;
//     t3 = nullptr;
//     L->CompileFile("./jit/test_local_table.lua", {.debug_mode = false});
//     L->call("test", std::tie(t1, t2, t3));
//     ASSERT_NE(t1, nullptr);
//     ASSERT_EQ(t1->ViGetType(), VarInterface::Type::TABLE);
//     ASSERT_NE(t2, nullptr);
//     ASSERT_EQ(t2->ViGetType(), VarInterface::Type::TABLE);
//     ASSERT_NE(t3, nullptr);
//     ASSERT_EQ(t3->ViGetType(), VarInterface::Type::TABLE);
//
//     // need sort kv
//     dynamic_cast<SimpleVarImpl *>(t1)->ViSortTable();
//     dynamic_cast<SimpleVarImpl *>(t2)->ViSortTable();
//     dynamic_cast<SimpleVarImpl *>(t3)->ViSortTable();
//     ASSERT_EQ(t1->ViToString(0),
//               "table:\n\t[1] = 1\n\t[2] = 2\n\t[3] = 3\n\t[4] = 4\n\t[5] = 5\n\t[6] = 6\n\t[7] = 7\n\t[8] = 8\n\t[9] = 9\n\t[10] = 10");
//     ASSERT_EQ(t2->ViToString(0), "table:\n\t[\"a\"] = 1\n\t[\"b\"] = 2\n\t[\"c\"] = 3");
//     ASSERT_EQ(t3->ViToString(0), "table:\n\t[1] = 1\n\t[2] = 3\n\t[3] = 5\n\t[\"b\"] = 2\n\t[\"d\"] = 4");
//
//     for (auto &i: tmp) {
//         delete i;
//     }
// }
//
// TEST(jitter, test_local_nested_table) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//     std::vector<VarInterface *> tmp;
//     auto newfunc = [&]() {
//         auto ret = new SimpleVarImpl();
//         tmp.push_back(ret);
//         return ret;
//     };
//     L->SetVarInterfaceNewFunc(newfunc);
//
//     VarInterface *t = nullptr;
//     L->CompileFile("./jit/test_local_nested_table.lua", {});
//     L->call("test", std::tie(t));
//     ASSERT_NE(t, nullptr);
//     ASSERT_EQ(t->ViGetType(), VarInterface::Type::TABLE);
//
//     // need sort kv
//     dynamic_cast<SimpleVarImpl *>(t)->ViSortTable();
//     ASSERT_EQ(t->ViToString(0), "table:\n\t[\"array\"] = table:\n\t\t[1] = 1\n\t\t[2] = 2\n\t\t[3] = 3\n\t[\"map\"] = table:\n\t\t[\"a\"] "
//                                 "= 1\n\t\t[\"b\"] = 2\n\t\t[\"c\"] = 3");
//
//     t = nullptr;
//     L->CompileFile("./jit/test_local_nested_table.lua", {.debug_mode = false});
//     L->call("test", std::tie(t));
//     ASSERT_NE(t, nullptr);
//     ASSERT_EQ(t->ViGetType(), VarInterface::Type::TABLE);
//
//     // need sort kv
//     dynamic_cast<SimpleVarImpl *>(t)->ViSortTable();
//     ASSERT_EQ(t->ViToString(0), "table:\n\t[\"array\"] = table:\n\t\t[1] = 1\n\t\t[2] = 2\n\t\t[3] = 3\n\t[\"map\"] = table:\n\t\t[\"a\"] "
//                                 "= 1\n\t\t[\"b\"] = 2\n\t\t[\"c\"] = 3");
//
//     for (auto &i: tmp) {
//         delete i;
//     }
// }
//
// TEST(jitter, test_local_table_with_variadic) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//     std::vector<VarInterface *> tmp;
//     auto newfunc = [&]() {
//         auto ret = new SimpleVarImpl();
//         tmp.push_back(ret);
//         return ret;
//     };
//     L->SetVarInterfaceNewFunc(newfunc);
//
//     VarInterface *t = nullptr;
//     L->CompileFile("./jit/test_local_table_with_variadic.lua", {});
//     L->call("test", std::tie(t), "a", "b", "c");
//     ASSERT_NE(t, nullptr);
//     ASSERT_EQ(t->ViGetType(), VarInterface::Type::TABLE);
//
//     // need sort kv
//     dynamic_cast<SimpleVarImpl *>(t)->ViSortTable();
//     ASSERT_EQ(t->ViToString(0), "table:\n\t[1] = \"a\"\n\t[2] = \"b\"\n\t[3] = \"c\"");
//
//     t = nullptr;
//     L->CompileFile("./jit/test_local_table_with_variadic.lua", {.debug_mode = false});
//     L->call("test", std::tie(t), "a", "b", "c");
//     ASSERT_NE(t, nullptr);
//     ASSERT_EQ(t->ViGetType(), VarInterface::Type::TABLE);
//
//     // need sort kv
//     dynamic_cast<SimpleVarImpl *>(t)->ViSortTable();
//     ASSERT_EQ(t->ViToString(0), "table:\n\t[1] = \"a\"\n\t[2] = \"b\"\n\t[3] = \"c\"");
//
//     for (auto &i: tmp) {
//         delete i;
//     }
// }
//
// TEST(jitter, test_local_table_with_variadic_no_end) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//     std::vector<VarInterface *> tmp;
//     auto newfunc = [&]() {
//         auto ret = new SimpleVarImpl();
//         tmp.push_back(ret);
//         return ret;
//     };
//     L->SetVarInterfaceNewFunc(newfunc);
//
//     VarInterface *t = nullptr;
//     L->CompileFile("./jit/test_local_table_with_variadic_no_end.lua", {});
//     L->call("test", std::tie(t), "a", "b", "c", "d", "e");
//     ASSERT_NE(t, nullptr);
//     ASSERT_EQ(t->ViGetType(), VarInterface::Type::TABLE);
//
//     // need sort kv
//     dynamic_cast<SimpleVarImpl *>(t)->ViSortTable();
//     ASSERT_EQ(t->ViToString(0), "table:\n\t[1] = \"c\"\n\t[2] = \"a\"\n\t[3] = \"b\"");
//
//     t = nullptr;
//     L->CompileFile("./jit/test_local_table_with_variadic_no_end.lua", {.debug_mode = false});
//     L->call("test", std::tie(t), "a", "b", "c", "d", "e");
//     ASSERT_NE(t, nullptr);
//     ASSERT_EQ(t->ViGetType(), VarInterface::Type::TABLE);
//
//     // need sort kv
//     dynamic_cast<SimpleVarImpl *>(t)->ViSortTable();
//     ASSERT_EQ(t->ViToString(0), "table:\n\t[1] = \"c\"\n\t[2] = \"a\"\n\t[3] = \"b\"");
//
//     for (auto &i: tmp) {
//         delete i;
//     }
// }
//
// TEST(jitter, test_local_table_with_variadic_no_end_replace) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//     std::vector<VarInterface *> tmp;
//     auto newfunc = [&]() {
//         auto ret = new SimpleVarImpl();
//         tmp.push_back(ret);
//         return ret;
//     };
//     L->SetVarInterfaceNewFunc(newfunc);
//
//     VarInterface *t = nullptr;
//     L->CompileFile("./jit/test_local_table_with_variadic_no_end_replace.lua", {});
//     L->call("test", std::tie(t), "a", "b", "c", "d", "e");
//     ASSERT_NE(t, nullptr);
//     ASSERT_EQ(t->ViGetType(), VarInterface::Type::TABLE);
//
//     // need sort kv
//     dynamic_cast<SimpleVarImpl *>(t)->ViSortTable();
//     ASSERT_EQ(t->ViToString(0), "table:\n\t[1] = \"c\"\n\t[2] = \"b\"");
//
//     t = nullptr;
//     L->CompileFile("./jit/test_local_table_with_variadic_no_end_replace.lua", {.debug_mode = false});
//     L->call("test", std::tie(t), "a", "b", "c", "d", "e");
//     ASSERT_NE(t, nullptr);
//     ASSERT_EQ(t->ViGetType(), VarInterface::Type::TABLE);
//
//     // need sort kv
//     dynamic_cast<SimpleVarImpl *>(t)->ViSortTable();
//     ASSERT_EQ(t->ViToString(0), "table:\n\t[1] = \"c\"\n\t[2] = \"b\"");
//
//     for (auto &i: tmp) {
//         delete i;
//     }
// }
//
// TEST(jitter, compile_empty_string) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     L->CompileString("", {});
// }
//
// TEST(jitter, test_assign_simple_var) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     int b = 0;
//     L->CompileFile("./jit/test_assign_simple_var.lua", {});
//     L->call("test", std::tie(a, b), "test", 1);
//     ASSERT_EQ(a, 1);
//     ASSERT_EQ(b, 1);
//
//     a = 0;
//     b = 0;
//     L->CompileFile("./jit/test_assign_simple_var.lua", {.debug_mode = false});
//     L->call("test", std::tie(a, b), "test", 1);
//     ASSERT_EQ(a, 1);
//     ASSERT_EQ(b, 1);
// }
//
// TEST(jitter, test_const_define_simple_var) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     int b = 0;
//     int c = 0;
//     L->CompileFile("./jit/test_const_define_simple_var.lua", {});
//     L->call("test", std::tie(a, b, c));
//     ASSERT_EQ(a, 1);
//     ASSERT_EQ(b, 1);
//     ASSERT_EQ(c, 1);
//
//     a = 0;
//     b = 0;
//     L->CompileFile("./jit/test_const_define_simple_var.lua", {.debug_mode = false});
//     L->call("test", std::tie(a, b, c));
//     ASSERT_EQ(a, 1);
//     ASSERT_EQ(b, 1);
//     ASSERT_EQ(c, 1);
// }
//
// TEST(jitter, test_binop_plus) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     double ret2 = 0;
//     L->CompileFile("./jit/test_binop_plus.lua", {});
//     L->call("test", std::tie(ret1, ret2), 1, 2, 1.1, 2.2);
//     ASSERT_EQ(ret1, 3);
//     ASSERT_NEAR(ret2, 3.3, 0.001);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_binop_plus.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), "1", 2, "1.1", 2.2);
//     ASSERT_EQ(ret1, 3);
//     ASSERT_NEAR(ret2, 3.3, 0.001);
// }
//
// TEST(jitter, test_const_binop_plus) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     double ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_plus.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 3);
//     ASSERT_NEAR(ret2, 3.2, 0.001);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_plus.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 3);
//     ASSERT_NEAR(ret2, 3.2, 0.001);
// }
//
// TEST(jitter, test_binop_minus) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     double ret2 = 0;
//     L->CompileFile("./jit/test_binop_minus.lua", {});
//     L->call("test", std::tie(ret1, ret2), 1, 2, 2, 1.2);
//     ASSERT_EQ(ret1, -1);
//     ASSERT_NEAR(ret2, 0.8, 0.001);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_binop_minus.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), "1", "2", 2, 1.2);
//     ASSERT_EQ(ret1, -1);
//     ASSERT_NEAR(ret2, 0.8, 0.001);
// }
//
// TEST(jitter, test_const_binop_minus) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     double ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_minus.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, -1);
//     ASSERT_NEAR(ret2, 1.1, 0.001);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_minus.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, -1);
//     ASSERT_NEAR(ret2, 1.1, 0.001);
// }
//
// TEST(jitter, test_binop_star) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     double ret2 = 0;
//     L->CompileFile("./jit/test_binop_star.lua", {});
//     L->call("test", std::tie(ret1, ret2), 1, 2, 2, 1.2);
//     ASSERT_EQ(ret1, 4);
//     ASSERT_NEAR(ret2, 1.4, 0.001);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_binop_star.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), "1", "2", 2, 1.2);
//     ASSERT_EQ(ret1, 4);
//     ASSERT_NEAR(ret2, 1.4, 0.001);
// }
//
// TEST(jitter, test_const_binop_star) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     double ret1 = 0;
//     double ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_star.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_NEAR(ret1, 3.2, 0.001);
//     ASSERT_NEAR(ret2, 0.1, 0.001);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_star.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_NEAR(ret1, 3.2, 0.001);
//     ASSERT_NEAR(ret2, 0.1, 0.001);
// }
//
// TEST(jitter, test_empty_return) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     CVar ret = {};
//     auto v = (Var &) ret;
//     L->CompileFile("./jit/test_empty_return.lua", {});
//     L->call("test", std::tie(ret));
//     ASSERT_EQ(v.Type(), VarType::Nil);
//
//     ret = {};
//     L->CompileFile("./jit/test_empty_return.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret));
//     ASSERT_EQ(v.Type(), VarType::Nil);
// }
//
// TEST(jitter, test_empty_func_no_return) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     CVar ret = {};
//     auto v = (Var &) ret;
//     L->CompileFile("./jit/test_empty_func_no_return.lua", {});
//     L->call("test", std::tie(ret));
//     ASSERT_EQ(v.Type(), VarType::Nil);
//
//     ret = {};
//     L->CompileFile("./jit/test_empty_func_no_return.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret));
//     ASSERT_EQ(v.Type(), VarType::Nil);
// }
//
// TEST(jitter, test_binop_slash) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     double ret1 = 0;
//     double ret2 = 0;
//     L->CompileFile("./jit/test_binop_slash.lua", {});
//     L->call("test", std::tie(ret1, ret2), 1, 2, 2.4, 1.2);
//     ASSERT_NEAR(ret1, 2.5, 0.001);
//     ASSERT_NEAR(ret2, 1, 0.001);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_binop_slash.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), "1", "2", 2.4, 1.2);
//     ASSERT_NEAR(ret1, 2.5, 0.001);
//     ASSERT_NEAR(ret2, 1, 0.001);
// }
//
// TEST(jitter, test_const_binop_slash) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     double ret1 = 0;
//     double ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_slash.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_NEAR(ret1, 1.7, 0.001);
//     ASSERT_NEAR(ret2, 0.1, 0.001);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_slash.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_NEAR(ret1, 1.7, 0.001);
//     ASSERT_NEAR(ret2, 0.1, 0.001);
// }
//
// TEST(jitter, test_binop_double_slash) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_binop_double_slash.lua", {});
//     L->call("test", std::tie(ret1, ret2), 3, 2, 2.4, 1.2);
//     ASSERT_EQ(ret1, 3);
//     ASSERT_EQ(ret2, 1);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_binop_double_slash.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 3, 2, 2.4, 1.2);
//     ASSERT_EQ(ret1, 3);
//     ASSERT_EQ(ret2, 1);
// }
//
// TEST(jitter, test_const_binop_double_slash) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     double ret1 = 0;
//     double ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_double_slash.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_NEAR(ret1, 2.2, 0.001);
//     ASSERT_NEAR(ret2, -0.1, 0.001);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_double_slash.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_NEAR(ret1, 2.2, 0.001);
//     ASSERT_NEAR(ret2, -0.1, 0.001);
// }
//
// TEST(jitter, test_binop_pow) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     double ret2 = 0;
//     L->CompileFile("./jit/test_binop_pow.lua", {});
//     L->call("test", std::tie(ret1, ret2), 3, 2, 2.4, 1.2);
//     ASSERT_EQ(ret1, 11);
//     ASSERT_NEAR(ret2, 1.859258955601, 0.001);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_binop_pow.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 3, 2, 2.4, 1.2);
//     ASSERT_EQ(ret1, 11);
//     ASSERT_NEAR(ret2, 1.859258955601, 0.001);
// }
//
// TEST(jitter, test_const_binop_pow) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     double ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_pow.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 8);
//     ASSERT_NEAR(ret2, 1.2332863005547, 0.001);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_pow.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 8);
//     ASSERT_NEAR(ret2, 1.2332863005547, 0.001);
// }
//
// TEST(jitter, test_binop_mod) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_binop_mod.lua", {});
//     L->call("test", std::tie(ret1, ret2), 3, 2, 2.4, 1.2);
//     ASSERT_EQ(ret1, 3);
//     ASSERT_EQ(ret2, -1);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_binop_mod.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 3, 2, 2.4, 1.2);
//     ASSERT_EQ(ret1, 3);
//     ASSERT_EQ(ret2, -1);
// }
//
// TEST(jitter, test_const_binop_mod) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_mod.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 2);
//     ASSERT_EQ(ret2, 0);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_mod.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 2);
//     ASSERT_EQ(ret2, 0);
// }
//
// TEST(jitter, test_binop_bitand) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_binop_bitand.lua", {});
//     L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
//     ASSERT_EQ(ret1, 0);
//     ASSERT_EQ(ret2, 0);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_binop_bitand.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
//     ASSERT_EQ(ret1, 0);
//     ASSERT_EQ(ret2, 0);
// }
//
// TEST(jitter, test_const_binop_bitand) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_bitand.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 2);
//     ASSERT_EQ(ret2, -124);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_bitand.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 2);
//     ASSERT_EQ(ret2, -124);
// }
//
// TEST(jitter, test_binop_xor) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_binop_xor.lua", {});
//     L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
//     ASSERT_EQ(ret1, 7);
//     ASSERT_EQ(ret2, 15);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_binop_xor.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
//     ASSERT_EQ(ret1, 7);
//     ASSERT_EQ(ret2, 15);
// }
//
// TEST(jitter, test_const_binop_xor) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_xor.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 1);
//     ASSERT_EQ(ret2, 18);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_xor.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 1);
//     ASSERT_EQ(ret2, 18);
// }
//
// TEST(jitter, test_binop_bitor) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_binop_bitor.lua", {});
//     L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
//     ASSERT_EQ(ret1, 7);
//     ASSERT_EQ(ret2, 15);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_binop_bitor.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
//     ASSERT_EQ(ret1, 7);
//     ASSERT_EQ(ret2, 15);
// }
//
// TEST(jitter, test_const_binop_bitor) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_bitor.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 3);
//     ASSERT_EQ(ret2, -123);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_bitor.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 3);
//     ASSERT_EQ(ret2, -123);
// }
//
// TEST(jitter, test_binop_right_shift) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_binop_right_shift.lua", {});
//     L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
//     ASSERT_EQ(ret1, 1);
//     ASSERT_EQ(ret2, 0);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_binop_right_shift.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
//     ASSERT_EQ(ret1, 1);
//     ASSERT_EQ(ret2, 0);
// }
//
// TEST(jitter, test_const_binop_right_shift) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int64_t ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_right_shift.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 1776);
//     ASSERT_EQ(ret2, 4611686018427387873);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_right_shift.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 1776);
//     ASSERT_EQ(ret2, 4611686018427387873);
// }
//
// TEST(jitter, test_binop_left_shift) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_binop_left_shift.lua", {});
//     L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
//     ASSERT_EQ(ret1, 20);
//     ASSERT_EQ(ret2, 8192);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_binop_left_shift.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 3, 2, "4", 12);
//     ASSERT_EQ(ret1, 20);
//     ASSERT_EQ(ret2, 8192);
// }
//
// TEST(jitter, test_const_binop_left_shift) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_left_shift.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 27);
//     ASSERT_EQ(ret2, -496);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_left_shift.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 27);
//     ASSERT_EQ(ret2, -496);
// }
//
// TEST(jitter, test_binop_concat) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     std::string ret;
//     L->CompileFile("./jit/test_binop_concat.lua", {});
//     L->call("test", std::tie(ret), 3, 1.2, true, "test");
//     ASSERT_EQ(ret, "31.2truetest");
//
//     ret.clear();
//     L->CompileFile("./jit/test_binop_concat.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret), 3, 1.2, true, "test");
//     ASSERT_EQ(ret, "31.2truetest");
// }
//
// TEST(jitter, test_const_binop_concat) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     std::string ret;
//     L->CompileFile("./jit/test_const_binop_concat.lua", {});
//     L->call("test", std::tie(ret));
//     ASSERT_EQ(ret, "23.2trueabcnil");
//
//     ret.clear();
//     L->CompileFile("./jit/test_const_binop_concat.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret));
//     ASSERT_EQ(ret, "23.2trueabcnil");
// }
//
// TEST(jitter, test_binop_less) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_binop_less.lua", {});
//     L->call("test", std::tie(ret1, ret2), 3, 1.2, 1, "10");
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_binop_less.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 3, 1.2, 1, "10");
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
// }
//
// TEST(jitter, test_const_binop_less) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_const_binop_less.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_const_binop_less.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
// }
//
// TEST(jitter, test_binop_less_equal) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_binop_less_equal.lua", {});
//     L->call("test", std::tie(ret1, ret2), 3, 1.2, 10, "10");
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_binop_less_equal.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 3, 1.2, 10, "10");
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
// }
//
// TEST(jitter, test_const_binop_less_equal) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_const_binop_less_equal.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_const_binop_less_equal.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
// }
//
// TEST(jitter, test_binop_more) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_binop_more.lua", {});
//     L->call("test", std::tie(ret1, ret2), 3, 1.2, 1, "10");
//     ASSERT_TRUE(ret1);
//     ASSERT_FALSE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_binop_more.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 3, 1.2, 1, "10");
//     ASSERT_TRUE(ret1);
//     ASSERT_FALSE(ret2);
// }
//
// TEST(jitter, test_const_binop_more) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_const_binop_more.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_TRUE(ret1);
//     ASSERT_FALSE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_const_binop_more.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_TRUE(ret1);
//     ASSERT_FALSE(ret2);
// }
//
// TEST(jitter, test_binop_more_equal) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_binop_more_equal.lua", {});
//     L->call("test", std::tie(ret1, ret2), 1, 1.2, 10, "10");
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_binop_more_equal.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 1, 1.2, 10, "10");
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
// }
//
// TEST(jitter, test_const_binop_more_equal) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_const_binop_more_equal.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_TRUE(ret1);
//     ASSERT_TRUE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_const_binop_more_equal.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_TRUE(ret1);
//     ASSERT_TRUE(ret2);
// }
//
// TEST(jitter, test_binop_equal) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_binop_equal.lua", {});
//     L->call("test", std::tie(ret1, ret2), 1, 1.2, "10", "10");
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_binop_equal.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 1, 1.2, "10", "10");
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
// }
//
// TEST(jitter, test_const_binop_equal) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_const_binop_equal.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_const_binop_equal.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
// }
//
// TEST(jitter, test_binop_not_equal) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_binop_not_equal.lua", {});
//     L->call("test", std::tie(ret1, ret2), 1, 1.2, "10", "10");
//     ASSERT_TRUE(ret1);
//     ASSERT_FALSE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_binop_not_equal.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 1, 1.2, "10", "10");
//     ASSERT_TRUE(ret1);
//     ASSERT_FALSE(ret2);
// }
//
// TEST(jitter, test_const_binop_not_equal) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_const_binop_not_equal.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_TRUE(ret1);
//     ASSERT_FALSE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_const_binop_not_equal.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_TRUE(ret1);
//     ASSERT_FALSE(ret2);
// }
//
// TEST(jitter, test_binop_and) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     float ret1 = 0;
//     CVar ret2 = {};
//     auto v = (Var &) ret2;
//     L->CompileFile("./jit/test_binop_and.lua", {});
//     L->call("test", std::tie(ret1, ret2), 1, 1.2, nullptr, "10");
//     ASSERT_NEAR(ret1, 1.2, 0.001);
//     ASSERT_EQ(v.Type(), VarType::Nil);
//
//     ret1 = 0;
//     ret2 = {};
//     L->CompileFile("./jit/test_binop_and.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 1, 1.2, nullptr, "10");
//     ASSERT_NEAR(ret1, 1.2, 0.001);
//     ASSERT_EQ(v.Type(), VarType::Nil);
// }
//
// TEST(jitter, test_binop_and_bool) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_binop_and_bool.lua", {});
//     L->call("test", std::tie(ret1, ret2), true, false);
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_binop_and_bool.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), true, false);
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
// }
//
// TEST(jitter, test_binop_and_or) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     float ret2 = 0;
//     L->CompileFile("./jit/test_binop_and_or.lua", {});
//     L->call("test", std::tie(ret1, ret2), 1, 2, 3, nullptr);
//     ASSERT_EQ(ret1, 3);
//     ASSERT_EQ(ret2, 4);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_binop_and_or.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 1, 2, 3, nullptr);
//     ASSERT_EQ(ret1, 3);
//     ASSERT_EQ(ret2, 4);
// }
//
// TEST(jitter, test_const_binop_and) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     CVar ret2 = {};
//     auto v = (Var &) ret2;
//     L->CompileFile("./jit/test_const_binop_and.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 3);
//     ASSERT_EQ(v.Type(), VarType::Bool);
//     ASSERT_EQ(v.GetBool(), false);
//
//     ret1 = 0;
//     ret2 = {};
//     L->CompileFile("./jit/test_const_binop_and.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 3);
//     ASSERT_EQ(v.Type(), VarType::Bool);
//     ASSERT_EQ(v.GetBool(), false);
// }
//
// TEST(jitter, test_binop_or) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_binop_or.lua", {});
//     L->call("test", std::tie(ret1, ret2), 1, 1.2, nullptr, "10");
//     ASSERT_EQ(ret1, 3);
//     ASSERT_EQ(ret2, 9);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_binop_or.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 1, 1.2, nullptr, "10");
//     ASSERT_EQ(ret1, 3);
//     ASSERT_EQ(ret2, 9);
// }
//
// TEST(jitter, test_const_binop_or) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     float ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_or.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 21);
//     ASSERT_NEAR(ret2, 2.2, 0.001);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_binop_or.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 21);
//     ASSERT_NEAR(ret2, 2.2, 0.001);
// }
//
// TEST(jitter, test_unop_minus) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     float ret = 0;
//     L->CompileFile("./jit/test_unop_minus.lua", {});
//     L->call("test", std::tie(ret), 2, "2.2");
//     ASSERT_NEAR(ret, -3.4, 0.001);
//
//     ret = 0;
//     L->CompileFile("./jit/test_unop_minus.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret), 2, "2.2");
//     ASSERT_NEAR(ret, -3.4, 0.001);
// }
//
// TEST(jitter, test_const_unop_minus) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret = 0;
//     L->CompileFile("./jit/test_const_unop_minus.lua", {});
//     L->call("test", std::tie(ret));
//     ASSERT_EQ(ret, 24);
//
//     ret = 0;
//     L->CompileFile("./jit/test_const_unop_minus.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret));
//     ASSERT_EQ(ret, 24);
// }
//
// TEST(jitter, test_unop_not) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_unop_not.lua", {});
//     L->call("test", std::tie(ret1, ret2), 2, nullptr);
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_unop_not.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 2, nullptr);
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
// }
//
// TEST(jitter, test_const_unop_not) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret1 = false;
//     bool ret2 = false;
//     L->CompileFile("./jit/test_const_unop_not.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
//
//     ret1 = false;
//     ret2 = false;
//     L->CompileFile("./jit/test_const_unop_not.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_FALSE(ret1);
//     ASSERT_TRUE(ret2);
// }
//
// TEST(jitter, test_unop_len) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_unop_len.lua", {});
//     L->call("test", std::tie(ret1, ret2), "abc", "123");
//     ASSERT_EQ(ret1, 3);
//     ASSERT_EQ(ret2, 3);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_unop_len.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), "abc", "123");
//     ASSERT_EQ(ret1, 3);
//     ASSERT_EQ(ret2, 3);
// }
//
// TEST(jitter, test_const_unop_len) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_const_unop_len.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 2);
//     ASSERT_EQ(ret2, 3);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_unop_len.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, 2);
//     ASSERT_EQ(ret2, 3);
// }
//
// TEST(jitter, test_unop_bitnot) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_unop_bitnot.lua", {});
//     L->call("test", std::tie(ret1, ret2), 123, -123);
//     ASSERT_EQ(ret1, -124);
//     ASSERT_EQ(ret2, 122);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_unop_bitnot.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2), 123, -123);
//     ASSERT_EQ(ret1, -124);
//     ASSERT_EQ(ret2, 122);
// }
//
// TEST(jitter, test_const_unop_bitnot) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret1 = 0;
//     int ret2 = 0;
//     L->CompileFile("./jit/test_const_unop_bitnot.lua", {});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, -124);
//     ASSERT_EQ(ret2, 122);
//
//     ret1 = 0;
//     ret2 = 0;
//     L->CompileFile("./jit/test_const_unop_bitnot.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret1, ret2));
//     ASSERT_EQ(ret1, -124);
//     ASSERT_EQ(ret2, 122);
// }
//
// TEST(jitter, test_local_func_call) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret = false;
//     L->CompileFile("./jit/test_local_func_call.lua", {});
//     L->call("test", std::tie(ret), 2, 1);
//     ASSERT_TRUE(ret);
//     L->call("test", std::tie(ret), 1, 2);
//     ASSERT_FALSE(ret);
//
//     ret = false;
//     L->CompileFile("./jit/test_local_func_call.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret), 2, 1);
//     ASSERT_TRUE(ret);
//     L->call("test", std::tie(ret), 1, 2);
//     ASSERT_FALSE(ret);
// }
//
// TEST(jitter, test_global_func_call) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret = false;
//     L->CompileFile("./jit/test_global_func_call.lua", {});
//     L->call("test", std::tie(ret), 2, 1);
//     ASSERT_TRUE(ret);
//
//     ret = false;
//     L->CompileFile("./jit/test_global_func_call.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret), 2, 1);
//     ASSERT_TRUE(ret);
// }
//
// TEST(jitter, test_assign_table_var) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_assign_table_var.lua", {});
//     L->call("test", std::tie(a), "test", 1);
//     ASSERT_EQ(a, 1);
//
//     a = 0;
//     L->CompileFile("./jit/test_assign_simple_var.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), "test", 1);
//     ASSERT_EQ(a, 1);
// }
//
// TEST(jitter, test_do_block) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     std::string b;
//     L->CompileFile("./jit/test_do_block.lua", {});
//     L->call("test", std::tie(a, b), true, 1.1);
//     ASSERT_EQ(a, 1);
//     ASSERT_EQ(b, "2");
//
//     a = 0;
//     b.clear();
//     L->CompileFile("./jit/test_do_block.lua", {.debug_mode = false});
//     L->call("test", std::tie(a, b), true, 1.1);
//     ASSERT_EQ(a, 1);
//     ASSERT_EQ(b, "2");
// }
//
// TEST(jitter, test_while) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     std::string b;
//     L->CompileFile("./jit/test_while.lua", {});
//     L->call("test", std::tie(a, b), 1, "a");
//     ASSERT_EQ(a, 3);
//     ASSERT_EQ(b, "a22");
//
//     a = 0;
//     b.clear();
//     L->CompileFile("./jit/test_while.lua", {.debug_mode = false});
//     L->call("test", std::tie(a, b), 1, "a");
//     ASSERT_EQ(a, 3);
//     ASSERT_EQ(b, "a22");
// }
//
// TEST(jitter, test_repeat) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     std::string b;
//     L->CompileFile("./jit/test_repeat.lua", {});
//     L->call("test", std::tie(a, b), 1, "a");
//     ASSERT_EQ(a, 3);
//     ASSERT_EQ(b, "a22");
//
//     a = 0;
//     b.clear();
//     L->CompileFile("./jit/test_repeat.lua", {.debug_mode = false});
//     L->call("test", std::tie(a, b), 1, "a");
//     ASSERT_EQ(a, 3);
//     ASSERT_EQ(b, "a22");
// }
//
// TEST(jitter, test_while_double) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_while_double.lua", {});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 18);
//
//     a = 0;
//     L->CompileFile("./jit/test_while_double.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 18);
// }
//
// TEST(jitter, test_repeat_double) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_repeat_double.lua", {});
//     L->call("test", std::tie(a), 4, 3);
//     ASSERT_EQ(a, 18);
//
//     a = 0;
//     L->CompileFile("./jit/test_repeat_double.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 4, 3);
//     ASSERT_EQ(a, 18);
// }
//
// TEST(jitter, test_if) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_if.lua", {});
//     L->call("test", std::tie(a), 5, 6);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 5, 2);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 3, 2);
//     ASSERT_EQ(a, 2);
//     L->call("test", std::tie(a), 0, 2);
//     ASSERT_EQ(a, 0);
//
//     a = 0;
//     L->CompileFile("./jit/test_if.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 5, 6);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 5, 2);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 3, 2);
//     ASSERT_EQ(a, 2);
//     L->call("test", std::tie(a), 0, 2);
//     ASSERT_EQ(a, 0);
// }
//
// TEST(jitter, test_if_simple) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_if_simple.lua", {});
//     L->call("test", std::tie(a), 5, 6);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 0, 2);
//     ASSERT_EQ(a, 0);
//
//     a = 0;
//     L->CompileFile("./jit/test_if_simple.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 5, 6);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 0, 2);
//     ASSERT_EQ(a, 0);
// }
//
// TEST(jitter, test_if_elseif) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_if_elseif.lua", {});
//     L->call("test", std::tie(a), 5, 5);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 4, 3);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 3, 2);
//     ASSERT_EQ(a, 2);
//     L->call("test", std::tie(a), 2, 1);
//     ASSERT_EQ(a, 1);
//     L->call("test", std::tie(a), 0, 0);
//     ASSERT_EQ(a, 0);
//
//     a = 0;
//     L->CompileFile("./jit/test_if_elseif.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 5, 5);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 4, 3);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 3, 2);
//     ASSERT_EQ(a, 2);
//     L->call("test", std::tie(a), 2, 1);
//     ASSERT_EQ(a, 1);
//     L->call("test", std::tie(a), 0, 0);
//     ASSERT_EQ(a, 0);
// }
//
// TEST(jitter, test_if_elseif_normal) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_if_elseif_normal.lua", {});
//     L->call("test", std::tie(a), 5, 5);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 4, 3);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 3, 2);
//     ASSERT_EQ(a, 2);
//     L->call("test", std::tie(a), 2, 1);
//     ASSERT_EQ(a, 1);
//     L->call("test", std::tie(a), 0, 0);
//     ASSERT_EQ(a, 0);
//
//     a = 0;
//     L->CompileFile("./jit/test_if_elseif_normal.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 5, 5);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 4, 3);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 3, 2);
//     ASSERT_EQ(a, 2);
//     L->call("test", std::tie(a), 2, 1);
//     ASSERT_EQ(a, 1);
//     L->call("test", std::tie(a), 0, 0);
//     ASSERT_EQ(a, 0);
// }
//
// TEST(jitter, test_if_elseif_return) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_if_elseif_return.lua", {});
//     L->call("test", std::tie(a), 5, 5);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 4, 3);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 3, 2);
//     ASSERT_EQ(a, 2);
//     L->call("test", std::tie(a), 2, 1);
//     ASSERT_EQ(a, 1);
//     L->call("test", std::tie(a), 0, 0);
//     ASSERT_EQ(a, 0);
//
//     a = 0;
//     L->CompileFile("./jit/test_if_elseif_return.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 5, 5);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 4, 3);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 3, 2);
//     ASSERT_EQ(a, 2);
//     L->call("test", std::tie(a), 2, 1);
//     ASSERT_EQ(a, 1);
//     L->call("test", std::tie(a), 0, 0);
//     ASSERT_EQ(a, 0);
// }
//
// TEST(jitter, test_if_else) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_if_else.lua", {});
//     L->call("test", std::tie(a), 5, 6);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 0, 2);
//     ASSERT_EQ(a, 0);
//
//     a = 0;
//     L->CompileFile("./jit/test_if_else.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 5, 6);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 0, 2);
//     ASSERT_EQ(a, 0);
// }
//
// TEST(jitter, test_while_return) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_while_return.lua", {});
//     L->call("test", std::tie(a), 4, 4);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 2, 4);
//     ASSERT_EQ(a, 2);
//
//     a = 0;
//     L->CompileFile("./jit/test_while_return.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 4, 4);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 2, 4);
//     ASSERT_EQ(a, 2);
// }
//
// TEST(jitter, test_repeat_return) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_repeat_return.lua", {});
//     L->call("test", std::tie(a), 4, 4);
//     ASSERT_EQ(a, 4);
//
//     a = 0;
//     L->CompileFile("./jit/test_repeat_return.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 4, 4);
//     ASSERT_EQ(a, 4);
// }
//
// TEST(jitter, test_while_break) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_while_break.lua", {});
//     L->call("test", std::tie(a), 4, 4);
//     ASSERT_EQ(a, 11);
//
//     a = 0;
//     L->CompileFile("./jit/test_while_break.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 4, 4);
//     ASSERT_EQ(a, 11);
// }
//
// TEST(jitter, test_repeat_break) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_repeat_break.lua", {});
//     L->call("test", std::tie(a), 4, 4);
//     ASSERT_EQ(a, 11);
//
//     a = 0;
//     L->CompileFile("./jit/test_repeat_break.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 4, 4);
//     ASSERT_EQ(a, 11);
// }
//
// TEST(jitter, test_while_if_return) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_while_if_return.lua", {});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 4, 4);
//     ASSERT_EQ(a, 4);
//
//     a = 0;
//     L->CompileFile("./jit/test_while_if_return.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 4, 4);
//     ASSERT_EQ(a, 4);
// }
//
// TEST(jitter, test_repeat_if_return) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_repeat_if_return.lua", {});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 4, 4);
//     ASSERT_EQ(a, 4);
//
//     a = 0;
//     L->CompileFile("./jit/test_repeat_if_return.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 4, 4);
//     ASSERT_EQ(a, 4);
// }
//
// TEST(jitter, test_for_loop) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_for_loop.lua", {});
//     L->call("test", std::tie(a), 3, 5);
//     ASSERT_EQ(a, 8);
//
//     a = 0;
//     L->CompileFile("./jit/test_for_loop.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 5);
//     ASSERT_EQ(a, 8);
// }
//
// TEST(jitter, test_for_loop_default) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_for_loop_default.lua", {});
//     L->call("test", std::tie(a), 3, 5);
//     ASSERT_EQ(a, 12);
//
//     a = 0;
//     L->CompileFile("./jit/test_for_loop_default.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 5);
//     ASSERT_EQ(a, 12);
// }
//
// TEST(jitter, test_for_loop_return) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_for_loop_return.lua", {});
//     L->call("test", std::tie(a), 3, 3);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 4, 3);
//     ASSERT_EQ(a, 0);
//
//     a = 0;
//     L->CompileFile("./jit/test_for_loop_return.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 3);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 4, 3);
//     ASSERT_EQ(a, 0);
// }
//
// TEST(jitter, test_while_just_break) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_while_just_break.lua", {});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 5, 4);
//     ASSERT_EQ(a, 5);
//
//     a = 0;
//     L->CompileFile("./jit/test_while_just_break.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 5, 4);
//     ASSERT_EQ(a, 5);
// }
//
// TEST(jitter, test_repeat_just_break) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_repeat_just_break.lua", {});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 5, 4);
//     ASSERT_EQ(a, 4);
//
//     a = 0;
//     L->CompileFile("./jit/test_repeat_just_break.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 4);
//     L->call("test", std::tie(a), 5, 4);
//     ASSERT_EQ(a, 4);
// }
//
// TEST(jitter, test_for_loop_double) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_for_loop_double.lua", {});
//     L->call("test", std::tie(a), 3, 5);
//     ASSERT_EQ(a, 21);
//
//     a = 0;
//     L->CompileFile("./jit/test_for_loop_double.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 5);
//     ASSERT_EQ(a, 21);
// }
//
// TEST(jitter, test_for_loop_break) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_for_loop_break.lua", {});
//     L->call("test", std::tie(a), 3, 5);
//     ASSERT_EQ(a, 4);
//
//     a = 0;
//     L->CompileFile("./jit/test_for_loop_break.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 5);
//     ASSERT_EQ(a, 4);
// }
//
// TEST(jitter, test_for_loop_if_return) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_for_loop_if_return.lua", {});
//     L->call("test", std::tie(a), 3, 5);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 6, 10);
//     ASSERT_EQ(a, 10);
//
//     a = 0;
//     L->CompileFile("./jit/test_for_loop_if_return.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 5);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 6, 10);
//     ASSERT_EQ(a, 10);
// }
//
// TEST(jitter, test_for_loop_just_break) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_for_loop_just_break.lua", {});
//     L->call("test", std::tie(a), 3, 5);
//     ASSERT_EQ(a, 5);
//     L->call("test", std::tie(a), 5, 3);
//     ASSERT_EQ(a, 5);
//
//     a = 0;
//     L->CompileFile("./jit/test_for_loop_just_break.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 5);
//     ASSERT_EQ(a, 5);
//     L->call("test", std::tie(a), 5, 3);
//     ASSERT_EQ(a, 5);
// }
//
// TEST(jitter, test_for_in) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_for_in.lua", {});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 32);
//
//     a = 0;
//     L->CompileFile("./jit/test_for_in.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 32);
// }
//
// TEST(jitter, test_for_in_double) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_for_in_double.lua", {});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 320);
//
//     a = 0;
//     L->CompileFile("./jit/test_for_in_double.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 320);
// }
//
// TEST(jitter, test_for_in_break) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_for_in_break.lua", {});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 30);
//
//     a = 0;
//     L->CompileFile("./jit/test_for_in_break.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 30);
// }
//
// TEST(jitter, test_for_in_if_return) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_for_in_if_return.lua", {});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 5, 4);
//     ASSERT_EQ(a, 4);
//
//     a = 0;
//     L->CompileFile("./jit/test_for_in_if_return.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 3);
//     L->call("test", std::tie(a), 5, 4);
//     ASSERT_EQ(a, 4);
// }
//
// TEST(jitter, test_for_in_just_break) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_for_in_just_break.lua", {});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 3);
//
//     a = 0;
//     L->CompileFile("./jit/test_for_in_just_break.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 3);
// }
//
// TEST(jitter, test_for_in_return) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_for_in_return.lua", {});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 3);
//
//     a = 0;
//     L->CompileFile("./jit/test_for_in_return.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 3);
// }
//
// TEST(jitter, test_for_in_return_fallback) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int a = 0;
//     L->CompileFile("./jit/test_for_in_return_fallback.lua", {});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 0);
//
//     a = 0;
//     L->CompileFile("./jit/test_for_in_return_fallback.lua", {.debug_mode = false});
//     L->call("test", std::tie(a), 3, 4);
//     ASSERT_EQ(a, 0);
// }
//
// TEST(jitter, test_local_func_call_table_construct) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret = false;
//     L->CompileFile("./jit/test_local_func_call_table_construct.lua", {});
//     L->call("test", std::tie(ret), 2, 1);
//     ASSERT_TRUE(ret);
//     L->call("test", std::tie(ret), 1, 2);
//     ASSERT_FALSE(ret);
//
//     ret = false;
//     L->CompileFile("./jit/test_local_func_call_table_construct.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret), 2, 1);
//     ASSERT_TRUE(ret);
//     L->call("test", std::tie(ret), 1, 2);
//     ASSERT_FALSE(ret);
// }
//
// TEST(jitter, test_local_func_call_string) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     std::string ret;
//     L->CompileFile("./jit/test_local_func_call_string.lua", {});
//     L->call("test", std::tie(ret), 2, 1);
//     ASSERT_EQ(ret, "test_test");
//
//     ret.clear();
//     L->CompileFile("./jit/test_local_func_call_string.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret), 2, 1);
//     ASSERT_EQ(ret, "test_test");
// }
//
// TEST(jitter, test_var_func_call) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret = false;
//     L->CompileFile("./jit/test_var_func_call.lua", {});
//     L->call("test", std::tie(ret), 2, 2);
//     ASSERT_TRUE(ret);
//     L->call("test", std::tie(ret), 1, 2);
//     ASSERT_FALSE(ret);
//
//     ret = false;
//     L->CompileFile("./jit/test_var_func_call.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret), 2, 2);
//     ASSERT_TRUE(ret);
//     L->call("test", std::tie(ret), 1, 2);
//     ASSERT_FALSE(ret);
// }
//
// TEST(jitter, test_table_var_func_call) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     bool ret = false;
//     L->CompileFile("./jit/test_table_var_func_call.lua", {});
//     L->call("test", std::tie(ret), 2, 2);
//     ASSERT_TRUE(ret);
//     L->call("test", std::tie(ret), 1, 2);
//     ASSERT_FALSE(ret);
//
//     ret = false;
//     L->CompileFile("./jit/test_table_var_func_call.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret), 2, 2);
//     ASSERT_TRUE(ret);
//     L->call("test", std::tie(ret), 1, 2);
//     ASSERT_FALSE(ret);
// }
//
// TEST(jitter, test_empty_func_call) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret = 0;
//     L->CompileFile("./jit/test_empty_func_call.lua", {});
//     L->call("test", std::tie(ret), 2, 2);
//     ASSERT_EQ(ret, 1);
//
//     ret = 0;
//     L->CompileFile("./jit/test_empty_func_call.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret), 2, 2);
//     ASSERT_EQ(ret, 1);
// }
//
// TEST(jitter, test_table_get_set) {
//     auto L = FakeluaNewState();
//     ASSERT_NE(L.get(), nullptr);
//
//     int ret = 0;
//     L->CompileFile("./jit/test_table_get_set.lua", {});
//     L->call("test", std::tie(ret), 1, 2);
//     ASSERT_EQ(ret, 3);
//
//     ret = 0;
//     L->CompileFile("./jit/test_table_get_set.lua", {.debug_mode = false});
//     L->call("test", std::tie(ret), 1, 2);
//     ASSERT_EQ(ret, 3);
// }
