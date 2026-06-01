#include "common.h"

namespace fakelua {

// 断言失败时抛出异常，附带源文件位置信息
[[noreturn]] void DebugAssertFail(const std::string &str, const std::source_location &source) {
    ThrowFakeluaException(std::format("assert fail: {} at {}:{}:{}", str, source.file_name(), source.line(), source.column()));
}

}// namespace fakelua