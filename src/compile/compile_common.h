#pragma once

#include <map>
#include <string>

namespace fakelua {

struct compile_config {
    bool skip_jit = false;
    bool debug_mode = true;
};

}// namespace fakelua