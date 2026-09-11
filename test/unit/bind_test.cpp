#include "fk_test.h"

class counter {
public:
    counter() : n(10) {}

    int add(int x) {
        return x + n;
    }

private:
    int n;
};

static counter *make_counter() {
    return new counter();
}

static void free_counter(counter *p) {
    delete p;
}

TEST_F(FakeEnv, CppMemberFunction) {
    fkreg(fk, "make_counter", make_counter);
    fkreg(fk, "free_counter", free_counter);
    fkreg(fk, "add", &counter::add);
    Parse(
            "func main()\n"
            "	var c = make_counter()\n"
            "	var r = c:add(2)\n"
            "	free_counter(c)\n"
            "	return r\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 12);
}

TEST_F(FakeEnv, SampleHostMemfunc) {
    Parse(
            "func main()\n"
            "	var t = new_test_class2()\n"
            "	var r = t:test_memfunc1(2)\n"
            "	delete_test_class2(t)\n"
            "	return r\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 12);
}

TEST_F(FakeEnv, ChainedSelf) {
    Parse(
            "func main()\n"
            "	var t = new_test_class2()\n"
            "	var r = t:self():test_memfunc2(3)\n"
            "	delete_test_class2(t)\n"
            "	return r\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 30);
}
