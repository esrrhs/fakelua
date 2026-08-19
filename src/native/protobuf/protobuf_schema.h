#pragma once

#include "protobuf_wire.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace fakelua::protobuf {

// ─── 枚举定义 ───

struct EnumDef {
    std::string name;
    // (名称, 编号) 列表
    std::vector<std::pair<std::string, int>> values;
    // 快速查找：编号 → 名称
    std::unordered_map<int, std::string> number_to_name;
};

// ─── 字段定义 ───

struct FieldDef {
    std::string name;
    int number = 0;                // 字段编号（1-536870911）
    FieldType type = TYPE_INT32;   // 标量类型
    std::string type_name;         // message/enum 类型的完全限定名（含点），标量类型为空
    bool repeated = false;
    bool is_map = false;           // map 字段标志
    bool optional = false;         // proto3 optional（显式 presence）
    int oneof_index = -1;          // -1 表示不属于 oneof

    // map 专用字段
    FieldType map_key_type = TYPE_INT32;
    FieldType map_value_type = TYPE_INT32;
    std::string map_value_type_name;  // map value 为 message/enum 时的类型名
};

// ─── 消息定义 ───

struct MessageDef {
    std::string name;                           // 完全限定名（如 "game.Player"）
    std::vector<FieldDef> fields;               // 按 number 排序
    std::unordered_map<int, const FieldDef*> number_to_field;  // 快速查找

    // 嵌套类型（名称相对于父消息）
    std::vector<MessageDef> nested_messages;
    std::vector<EnumDef> nested_enums;

    // 构建 number_to_field 索引（注册后调用）
    void BuildIndex();
};

// ─── Schema 注册器（进程全局单例） ───

class ProtobufState {
public:
    static ProtobufState &Instance();

    // 注册 message/enum（name 为完全限定名）
    void RegisterMessage(MessageDef def);
    void RegisterEnum(EnumDef def);

    // 查找
    const MessageDef *FindMessage(const std::string &name) const;
    const EnumDef *FindEnum(const std::string &name) const;

    // 已注册的所有 message 名称
    std::vector<std::string> MessageNames() const;

    // 解析完成后，将所有字段中引用已注册 enum 的 TYPE_ENUM 修正
    void ResolveAll();

    // 清空所有注册（测试用）
    void Clear();

private:
    ProtobufState() = default;

    std::unordered_map<std::string, MessageDef> messages_;
    std::unordered_map<std::string, EnumDef> enums_;
};

}  // namespace fakelua::protobuf
