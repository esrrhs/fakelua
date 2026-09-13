#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace fakelua {

// 定义 StrContainerPtr
using StrContainerPtr = std::shared_ptr<std::string>;

bool IsNumber(const std::string_view &str);

bool IsInteger(const std::string_view &str);

int64_t ToInteger(const std::string_view &input);

double ToFloat(const std::string_view &input);

// 不抛异常的数字解析（CSV/INI 类型推断）。整串必须都被消费。
bool TryParseInt64(const std::string_view &input, int64_t &out);

bool TryParseDouble(const std::string_view &input, double &out);

std::string JoinString(const std::vector<std::string> &strs, const std::string &sep);

std::string ReplaceEscapeChars(const std::string &str);

void TrimInplace(std::string &str);

}// namespace fakelua
