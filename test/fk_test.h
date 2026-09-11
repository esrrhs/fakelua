#pragma once

#include "host.h"
#include "gtest/gtest.h"

class FakeEnv : public ::testing::Test {
protected:
    fake *fk = nullptr;

    void SetUp() override {
        fk = fk_test_new();
        ASSERT_NE(fk, nullptr);
    }

    void TearDown() override {
        if (fk) {
            delfake(fk);
            fk = nullptr;
        }
    }

    void Parse(const char *src) {
        ASSERT_TRUE(fkparsestr(fk, src)) << fkerrorstr(fk);
    }

    template<typename T>
    T Run(const char *func = "main") {
        T ret = fkrun<T>(fk, func);
        EXPECT_EQ(fkerror(fk), efk_ok) << fkerrorstr(fk);
        return ret;
    }
};
