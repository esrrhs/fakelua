#include "host.h"

class test_class1 {
public:
    test_class1() : b(10) {}

    int test_memfunc1(int a) {
        return a - b;
    }

private:
    int b;
};

class test_class2 {
public:
    test_class2() : b(10) {}

    int test_memfunc1(int a) {
        return a + b;
    }

    int test_memfunc2(int a) {
        return a * b;
    }

    test_class2 *self() {
        return this;
    }

private:
    int b;
};

int test_cfunc1(int a, int b) {
    return a - b;
}

static test_class1 *new_test_class1() {
    return new test_class1();
}

static void delete_test_class1(test_class1 *p) {
    delete p;
}

static test_class2 *new_test_class2() {
    return new test_class2();
}

static void delete_test_class2(test_class2 *p) {
    delete p;
}

void fk_bind_sample_host(fake *fk) {
    fkreg(fk, "test_cfunc1", test_cfunc1);
    fkreg(fk, "new_test_class1", new_test_class1);
    fkreg(fk, "new_test_class2", new_test_class2);
    fkreg(fk, "delete_test_class1", delete_test_class1);
    fkreg(fk, "delete_test_class2", delete_test_class2);
    fkreg(fk, "test_memfunc1", &test_class1::test_memfunc1);
    fkreg(fk, "test_memfunc1", &test_class2::test_memfunc1);
    fkreg(fk, "test_memfunc2", &test_class2::test_memfunc2);
    fkreg(fk, "self", &test_class2::self);
}

fake *fk_test_new() {
    fake *fk = newfake();
    fkopenalllib(fk);
    fk_bind_sample_host(fk);
    return fk;
}
