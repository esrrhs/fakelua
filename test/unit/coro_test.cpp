#include "fk_test.h"

TEST_F(FakeEnv, YieldThenFinish) {
    Parse(
            "func worker()\n"
            "	yield 1\n"
            "	return 7\n"
            "end\n"
            "func main()\n"
            "	fake worker()\n"
            "	return 1\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 1);
}

TEST_F(FakeEnv, WorkerCompletesInSameFkrun) {
    Parse(
            "func worker()\n"
            "	var g = _G()\n"
            "	g[\"done\"] = 9\n"
            "end\n"
            "func main()\n"
            "	fake worker()\n"
            "	yield 1\n"
            "	var g = _G()\n"
            "	return g[\"done\"]\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 9);
}

TEST_F(FakeEnv, SleepReturns) {
    Parse("func main() sleep 1 return 4 end");
    EXPECT_EQ(Run<int>(), 4);
}
