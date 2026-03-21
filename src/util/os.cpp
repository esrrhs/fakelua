#include "common.h"

namespace fakelua {

std::string Exec(const char *cmd) {
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), fclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    std::array<char, 128> buffer;
    std::string result;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

}// namespace fakelua