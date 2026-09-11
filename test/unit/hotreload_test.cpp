#include "fk_test.h"

static int cfunc_a() {
    return 1;
}

static int cfunc_b() {
    return 2;
}

TEST_F(FakeEnv, ReregisterCFunction) {
    fkreg(fk, "cfunc", cfunc_a);
    Parse("func main() return cfunc() end");
    EXPECT_EQ(Run<int>(), 1);

    fkreg(fk, "cfunc", cfunc_b);
    ASSERT_EQ(fkerror(fk), efk_ok) << fkerrorstr(fk);
    EXPECT_EQ(Run<int>(), 2);
}

TEST_F(FakeEnv, ReparseReplacesFunction) {
    fkreg(fk, "cfunc", cfunc_a);
    Parse("func main() return cfunc() end");
    EXPECT_EQ(Run<int>(), 1);

    Parse("func main() return cfunc() + 10 end");
    EXPECT_EQ(Run<int>(), 11);
}

TEST_F(FakeEnv, DostringOverwritesHelper) {
    Parse(
            "func helper() return 1 end\n"
            "func main()\n"
            "	dostring(\"func helper() return 2 end\")\n"
            "	return helper()\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 2);
}
