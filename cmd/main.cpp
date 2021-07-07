#include "fakelua.h"
#include <iostream>

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout<<"usage: [script [args]]"<<std::endl;
        return 0;
    }

    auto state = fakelua_newstate();

    fakelua_close(state);

    return 0;
}
