#include "types.h"

void log(const std::string_view &message, const std::experimental::source_location &location) {
    std::cout << "debug:"
              << location.file_name() << ":"
              << location.line() << " "
              << message << '\n';
}
