#include "fakelua/fakelua.h"
#include <iostream>

using namespace fakelua;

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "usage: [script [args]]" << std::endl;
        return 0;
    }

    auto L = fakelua_newstate();

    fakelua_close(L);

    return 0;
}
