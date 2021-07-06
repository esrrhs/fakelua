#include "fbenchmark.h"
#include "types.h"
#include "fakelua.h"
#include "location.h"

enum TestEnum {
    One, Two, Three
};

FBENCHMARK(common, enum_type_name) {
    std::string_view n = enum_type_name<TestEnum>();
    USE(n);
}
