#include "fakelua.h"
#include "util/common.h"
#include "gtest/gtest.h"

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
