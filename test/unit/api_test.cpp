#include "fk_test.h"

#include <cstring>
#include <string>

TEST_F(FakeEnv, ParseEmptyIsOk) {
    EXPECT_TRUE(fkparsestr(fk, ""));
    EXPECT_EQ(fkerror(fk), efk_ok);
}

TEST_F(FakeEnv, ParseCommentOnly) {
    EXPECT_TRUE(fkparsestr(fk, "-- just a comment\n"));
}

TEST_F(FakeEnv, ParseSyntaxError) {
    EXPECT_FALSE(fkparsestr(fk, "func main( end"));
    EXPECT_NE(fkerror(fk), efk_ok);
    EXPECT_STRNE(fkerrorstr(fk), "");
}

TEST_F(FakeEnv, MissingFunction) {
    Parse("func main() return 1 end");
    fkrun<int>(fk, "no_such");
    EXPECT_EQ(fkerror(fk), efk_run_no_func_error);
}

TEST_F(FakeEnv, IsFuncAfterParse) {
    Parse("func foo() return 1 end");
    EXPECT_TRUE(fkisfunc(fk, "foo"));
    EXPECT_FALSE(fkisfunc(fk, "bar"));
}

TEST_F(FakeEnv, ClearDropsBytecodeKeepsBind) {
    Parse("func foo() return test_cfunc1(9, 3) end");
    EXPECT_EQ(Run<int>("foo"), 6);
    fkclear(fk);
    Parse("func foo() return test_cfunc1(5, 1) end");
    EXPECT_EQ(Run<int>("foo"), 4);
}

TEST_F(FakeEnv, ResetKeepsBytecode) {
    Parse("func main() return 3 end");
    EXPECT_EQ(Run<int>(), 3);
    fkreset(fk);
    EXPECT_TRUE(fkisfunc(fk, "main"));
    EXPECT_EQ(Run<int>(), 3);
}

TEST_F(FakeEnv, GmapPersistsUntilReset) {
    Parse(
            "func set()\n"
            "	var g = _G()\n"
            "	g[\"k\"] = 42\n"
            "	return 1\n"
            "end\n"
            "func get()\n"
            "	var g = _G()\n"
            "	return g[\"k\"]\n"
            "end\n");
    EXPECT_EQ(Run<int>("set"), 1);
    EXPECT_EQ(Run<int>("get"), 42);
    fkreset(fk);
    EXPECT_EQ(Run<int>("get"), 0);
}

TEST_F(FakeEnv, ToNumberToString) {
    Parse(
            "func main()\n"
            "	return tonumber(\"12\") + tonumber(tostring(3))\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 15);
}

TEST_F(FakeEnv, DumpstatContainsHeap) {
    Parse("func main() return dumpstat() end");
    const char *s = Run<const char *>();
    ASSERT_NE(s, nullptr);
    EXPECT_TRUE(std::strstr(s, "Stack String Heap size:") != nullptr);
}

TEST_F(FakeEnv, PauseIsNoop) {
    Parse("func main() pause() return 1 end");
    EXPECT_EQ(Run<int>(), 1);
}

TEST_F(FakeEnv, Getcurfunc) {
    Parse("func main() return getcurfunc() end");
    const char *name = Run<const char *>();
    ASSERT_NE(name, nullptr);
    EXPECT_STREQ(name, "main");
}

TEST_F(FakeEnv, IsfuncBuiltin) {
    Parse(
            "func foo() return 1 end\n"
            "func main() return isfunc(\"foo\") end\n");
    EXPECT_NE(Run<int>(), 0);
}
