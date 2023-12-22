#include "fakelua.h"
#include "gflags/gflags.h"
#include <iostream>

using namespace fakelua;

// register some global flags
DEFINE_bool(debug, false, "enable debug mode");
DEFINE_string(entry, "main", "entry function name, entry must return code(int) and has no parameter");

int main(int argc, char **argv) {
    gflags::SetUsageMessage("usage: ./lua --help\n"
                            "\n");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, (char ***) &argv, true);

    if (argc < 2) {
        std::cout << gflags::ProgramUsage() << std::endl;
        return 0;
    }

    auto L = fakelua_newstate();
    L->compile_file(argv[1], {debug_mode: FLAGS_debug});

    int code = 0;
    L->call(FLAGS_entry, std::tie(code));
    std::cout << "code: " << code << std::endl;

    return code;
}
