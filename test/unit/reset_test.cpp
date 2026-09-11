#include "fk_test.h"

#include <cstdlib>
#include <cstring>
#include <string>

TEST_F(FakeEnv, ResetDropsRuntimeStringsKeepsBytecode) {
    Parse(
            "func main()\n"
            "	return 1\n"
            "end\n"
            "func fill()\n"
            "	var a = array()\n"
            "	for var i = 0, i < 40, i++ then\n"
            "		a[i] = format(\"u%\", i)\n"
            "	end\n"
            "	var m = map()\n"
            "	m[1] = a\n"
            "	return size(a)\n"
            "end\n"
            "func stats()\n"
            "	return dumpstat()\n"
            "end\n");

    EXPECT_EQ(Run<int>("main"), 1);
    EXPECT_EQ(Run<int>("fill"), 40);

    const char *before = Run<const char *>("stats");
    ASSERT_NE(before, nullptr);
    std::string before_copy(before);
    const char *p = std::strstr(before_copy.c_str(), "Stack String Heap size:");
    ASSERT_NE(p, nullptr);
    p += std::strlen("Stack String Heap size:");
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    int stack_before = atoi(p);
    ASSERT_GT(stack_before, 0);

    fkreset(fk);
    ASSERT_TRUE(fkisfunc(fk, "main"));
    ASSERT_TRUE(fkisfunc(fk, "fill"));

    const char *after = Run<const char *>("stats");
    ASSERT_NE(after, nullptr);
    p = std::strstr(after, "Stack String Heap size:");
    ASSERT_NE(p, nullptr);
    p += std::strlen("Stack String Heap size:");
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    int stack_after = atoi(p);
    EXPECT_LT(stack_after, stack_before);

    EXPECT_EQ(Run<int>("main"), 1);
    EXPECT_EQ(Run<int>("fill"), 40);
}
