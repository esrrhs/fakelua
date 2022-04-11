#include "fakelua.h"
#include <iostream>

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "usage: [script [args]]" << std::endl;
        return 0;
    }

    auto L = fakelua_newstate();

    auto code = fakelua_dofile(L, argv[1]);

    fakelua_close(L);

    return code;
}
