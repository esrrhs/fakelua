#pragma once

#include "compile/inferred_type.h"
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace fakelua {

// 将 Lua 字段名转换为合法的 C 标识符
inline std::string ToSafeCFieldName(const std::string &name) {
    if (name.empty()) return "_empty";
    std::string result;
    result.reserve(name.size() + 1);
    if (std::isdigit((unsigned char) name[0])) result += '_';
    for (char c: name) {
        if (std::isalnum((unsigned char) c) || c == '_') result += c;
        else result += '_';
    }
    return result;
}

// 一个 record 字段的完整描述
struct FieldDef {
    std::string name;          // Lua 字段名
    std::string c_field_name;  // 安全的 C 标识符
    InferredType type = T_DYNAMIC;
    bool optional = false;     // φ 合并产生，此字段在某条路径上可能不存在
    bool is_int_key = false;   // true 表示由数组式（无键）table 构造字面量产生

    bool operator==(const FieldDef &o) const {
        return name == o.name && c_field_name == o.c_field_name &&
               type == o.type && optional == o.optional && is_int_key == o.is_int_key;
    }
};

// 一个 record/table 的结构化描述
struct ShapeType {
    int shape_id = -1;
    bool is_open = false;  // 开放 record：已知字段走偏移，未知字段走 hash
    std::vector<FieldDef> fields;

    [[nodiscard]] bool HasField(const std::string &name) const {
        return FindField(name) != nullptr;
    }

    [[nodiscard]] const FieldDef *FindField(const std::string &name) const {
        for (const auto &f: fields)
            if (f.name == name) return &f;
        return nullptr;
    }

    FieldDef *FindFieldMut(const std::string &name) {
        for (auto &f: fields)
            if (f.name == name) return &f;
        return nullptr;
    }

    [[nodiscard]] std::string Signature() const {
        std::vector<std::string> parts;
        parts.reserve(fields.size());
        for (const auto &f: fields)
            parts.push_back(f.name + ":" + std::to_string((int) f.type) + ":" + (f.optional ? "1" : "0") + ":" + (f.is_int_key ? "I" : "S"));
        std::sort(parts.begin(), parts.end());
        std::string sig;
        for (const auto &p: parts) { sig += p; sig += ';'; }
        sig += (is_open ? "O" : "C");
        return sig;
    }
};

// 全局 shape 注册表 - 相同签名的 shape 共用同一个 shape_id
class ShapeRegistry {
public:
    static constexpr int kWidenFieldThreshold = 16;
    static constexpr int kWidenIterThreshold = 3;

    // 注册或复用已有 shape，返回 shape_id
    int Intern(ShapeType s) {
        for (auto &f: s.fields)
            if (f.c_field_name.empty())
                f.c_field_name = ToSafeCFieldName(f.name);

        const std::string sig = s.Signature();
        if (auto it = sig_to_id_.find(sig); it != sig_to_id_.end())
            return it->second;

        const int id = (int) shapes_.size();
        s.shape_id = id;
        shapes_.push_back(std::move(s));
        sig_to_id_[sig] = id;
        return id;
    }

    [[nodiscard]] const ShapeType &Get(int id) const { return shapes_.at(id); }
    [[nodiscard]] ShapeType &GetMut(int id) { return shapes_.at(id); }
    [[nodiscard]] int Count() const { return (int) shapes_.size(); }

    // Meet 操作（φ 节点合并两条路径的 shape）
    int Meet(int a_id, int b_id) {
        if (a_id == b_id) return a_id;
        const ShapeType &a = Get(a_id);
        const ShapeType &b = Get(b_id);

        ShapeType result;
        result.is_open = a.is_open || b.is_open;

        for (const auto &fa: a.fields) {
            const FieldDef *fb = b.FindField(fa.name);
            FieldDef fd;
            fd.name = fa.name;
            fd.c_field_name = fa.c_field_name;
            fd.is_int_key = fa.is_int_key;
            if (fb) {
                fd.type = MeetType(fa.type, fb->type);
                fd.optional = fa.optional || fb->optional;
            } else {
                fd.type = fa.type;
                fd.optional = true;
            }
            result.fields.push_back(fd);
        }
        for (const auto &fb: b.fields) {
            if (!a.FindField(fb.name)) {
                FieldDef fd = fb;
                fd.optional = true;
                result.fields.push_back(fd);
            }
        }

        return Intern(std::move(result));
    }

    // 两个 InferredType 的 meet
    static InferredType MeetType(InferredType a, InferredType b) {
        if (a == b) return a;
        if (a == T_DYNAMIC || b == T_DYNAMIC) return T_DYNAMIC;
        if (a == T_NIL) return b;
        if (b == T_NIL) return a;
        if ((a == T_INT && b == T_FLOAT) || (a == T_FLOAT && b == T_INT)) return T_FLOAT;
        return T_DYNAMIC;
    }

    // 迭代次数超限时截断字段集，防止无限增长
    int Widen(int shape_id, int iter_count) {
        const ShapeType &s = Get(shape_id);

        if (iter_count >= kWidenIterThreshold || (int) s.fields.size() > kWidenFieldThreshold) {
            if (s.is_open && (int) s.fields.size() > kWidenFieldThreshold) {
                return -1;  // 退化为 T_DYNAMIC
            }
            ShapeType w = s;
            w.is_open = true;
            if ((int) w.fields.size() > kWidenFieldThreshold)
                w.fields.resize(kWidenFieldThreshold);
            return Intern(std::move(w));
        }
        return shape_id;
    }

private:
    std::vector<ShapeType> shapes_;
    std::unordered_map<std::string, int> sig_to_id_;
};

}// namespace fakelua
