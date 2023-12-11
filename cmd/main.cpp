#include "fakelua.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include <iostream>

using namespace fakelua;

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);

    gflags::SetUsageMessage("usage: ./lua --help\n"
                            "\n");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, (char ***) &argv, true);

    if (argc < 2) {
        std::cout << gflags::ProgramUsage() << std::endl;
        return 0;
    }

    auto L = fakelua_newstate();
    if (L.get() == nullptr) {
        std::cout << "failed to create lua state" << std::endl;
        return 1;
    }

    return 0;
}
