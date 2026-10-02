#pragma once

// mysql_field_convert.h — 行值类型转换：FieldCell → Lua CVar。
// 此头文件不依赖 mysql.h 或 libevent，可直接在测试代码中 include。

#include <cstdint>
#include <string>

namespace fakelua {
struct CVar;
class State;
}// namespace fakelua

namespace fakelua::mysql {

// MySQL 结果行中的一个字段值。
struct FieldCell {
    bool is_null = false;
    std::string value;
};

// 按列类型把行值转成 Lua CVar。
// mysql_type 为 MariaDB/MySQL 的 enum_field_types 整数值（mysql.h MYSQL_TYPE_*）。
// 整数列（TINY/SHORT/LONG/LONGLONG/INT24/YEAR）→ VarType::Int
// 浮点/小数列（FLOAT/DOUBLE/DECIMAL/NEWDECIMAL）→ VarType::Float
// 其他列（字符串/日期/BLOB 等）→ VarType::String
// NULL 字段（is_null == true）→ VarType::Nil
// 解析失败（如整数列收到非数字值）→ 回退为 VarType::String，不丢数据。
CVar FieldCellToCVar(::fakelua::State *s, const FieldCell &fv, int mysql_type);

}// namespace fakelua::mysql
