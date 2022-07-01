#include "fakelua/fakelua.h"
#include <iostream>
#include "gflags/gflags.h"
#include "glog/logging.h"

using namespace fakelua;

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);

    gflags::SetUsageMessage("usage: [script [args]]\n"
                            "\n");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, (char ***) &argv, true);

    if (argc < 2) {
        std::cout << "usage: [script [args]]" << std::endl;
        return 0;
    }

    auto L = fakelua_newstate();
    if (L.get() == nullptr) {
        std::cout << "failed to create lua state" << std::endl;
        return 1;
    }

    return 0;
}
