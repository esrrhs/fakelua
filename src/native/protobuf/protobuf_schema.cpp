#include "protobuf_schema.h"

#include <algorithm>

namespace fakelua::protobuf {

// ─── MessageDef ───

void MessageDef::BuildIndex() {
    number_to_field.clear();
    for (auto &f : fields) {
        number_to_field[f.number] = &f;
    }
}

// ─── ProtobufState ───

ProtobufState &ProtobufState::Instance() {
    static ProtobufState instance;
    return instance;
}

void ProtobufState::RegisterMessage(MessageDef def) {
    def.BuildIndex();
    messages_[def.name] = std::move(def);
}

void ProtobufState::RegisterEnum(EnumDef def) {
    def.number_to_name.clear();
    for (auto &[name, number] : def.values) {
        def.number_to_name[number] = name;
    }
    enums_[def.name] = std::move(def);
}

const MessageDef *ProtobufState::FindMessage(const std::string &name) const {
    auto it = messages_.find(name);
    if (it != messages_.end()) return &it->second;
    return nullptr;
}

const EnumDef *ProtobufState::FindEnum(const std::string &name) const {
    auto it = enums_.find(name);
    if (it != enums_.end()) return &it->second;
    return nullptr;
}

std::vector<std::string> ProtobufState::MessageNames() const {
    std::vector<std::string> names;
    names.reserve(messages_.size());
    for (auto &[name, _] : messages_) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    return names;
}

void ProtobufState::ResolveAll() {
    // 遍历所有 message 的字段，将 type_name 匹配已注册 enum 的字段从 TYPE_MESSAGE 改为 TYPE_ENUM
    for (auto &[name, msg] : messages_) {
        for (auto &field : msg.fields) {
            if (field.type == TYPE_MESSAGE && !field.type_name.empty()) {
                if (FindEnum(field.type_name)) {
                    field.type = TYPE_ENUM;
                }
            }
        }
        // 递归处理嵌套 message
        for (auto &nested : msg.nested_messages) {
            for (auto &field : nested.fields) {
                if (field.type == TYPE_MESSAGE && !field.type_name.empty()) {
                    if (FindEnum(field.type_name)) {
                        field.type = TYPE_ENUM;
                    }
                }
            }
        }
    }
}

void ProtobufState::Clear() {
    messages_.clear();
    enums_.clear();
}

}  // namespace fakelua::protobuf
