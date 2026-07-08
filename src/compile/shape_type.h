#pragma once

#include "compile/inferred_type.h"
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace fakelua {

// 将 Lua 字段名转换为合法的 C 标识符
// 用于 table 字段需要生成 C struct 成员时的安全转换
// 规则：若首字符为数字则前缀 '_'；所有非（字母/数字/下划线）字符替换为 '_'
inline std::string ToSafeCFieldName(const std::string &name) {
    if (name.empty()) return "_empty";
    std::string result;
    result.reserve(name.size() + 1);
    if (std::isdigit((unsigned char) name[0])) result += '_';
    for (char c: name) {
        if (std::isalnum((unsigned char) c) || c == '_') result += c;
        else
            result += '_';
    }
    return result;
}

// 一个 record 字段的完整描述
// 用于表达 table/record 中某一位成员的静态已知信息
//
// 在 ShapeRegistry 体系中，ShapeType 是 record 的"结构轮廓"（field 的名字、类型、可选性），
// 而 FieldDef 就是轮廓中的"一行"——一个字段。
//
// shape_type.h 中的"Shape"和"Type"（用户/编译前端看到的 table 值）之间的关系是：
//   - 每个具体的 table 值在运行前推断出一个 ShapeType；
//   - ShapeRegistry 去重，把结构相同的 ShapeType 并为一个 shape_id；
//   - shape_id 最终作为 C 结构体的布局依据，落到代码生成里去。
struct FieldDef {
    std::string name;             // 原始 Lua 字段名（例如 "x"、"42"、"_embedded"）
    std::string c_field_name;     // 经过 ToSafeCFieldName 转写后的 C 标识符，保证可作为 struct 成员名
    InferredType type = T_DYNAMIC;// 字段推断出来的静态类型（来自 inferred_type.h 的简单枚举）
    bool optional = false;        // true 表示此字段在某条控制流路径上可能不存在（由 φ 节点 merge 产生）
    // 例如：if cond then t={x=1} else t={x=1,y=2} end  → merge 后 y 是 optional
    bool is_int_key = false;// true 表示该字段来源于数组式（无显式键）table 构造字面量

    // 例如：t = {1, 2, 3} → 三个字段的名字分别是 "1" "2" "3"，但 is_int_key = true

    bool operator==(const FieldDef &o) const {
        return name == o.name && c_field_name == o.c_field_name && type == o.type && optional == o.optional && is_int_key == o.is_int_key;
    }
};

// 一个 record/table 的结构化描述（"Shape" = 表结构/轮廓）
//
// 举例：local t = { x = 1, y = "hi" } 可能对应一个 ShapeType：
//   is_open = false
//   fields = [ {name="x", type=T_INT}, {name="y", type=T_STRING} ]
//
// ShapeType 是不可变快照——一旦被 Intern 进 registry，就通过 shape_id 共享。
// 需要修改时请以拷贝为基础构造新 ShapeType 再 Intern。
struct ShapeType {
    int shape_id = -1;   // 由 ShapeRegistry 分配的全局唯一 id，-1 表示尚未注册
    bool is_open = false;// 开放 record：已知字段走偏移访问，未知字段走 hash 查找
    // is_open = true 通常发生在：迭代 widening 之后；或字面量写了很多字段之后，
    // 此时不能再保证访问安全，但仍可使用已知道字段的偏移
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

    // 计算这个 Shape 的"签名"——全局唯一地描述自身的结构
    //
    // 注意：签名基于"排序后字段描述列表"，因此字段顺序不参与签名
    // （{a=1,b=2} 和 {b=2,a=1} 会得到相同的 shape_id，语义一致）
    //
    // 每个字段的描述片段： name:type:optional:is_int_key
    //  - type 用 (int) 枚举数值参与哈希
    //  - optional 用 "1"/"0" 表示
    //  - is_int_key 用 "I"(int key) / "S"(string key) 表示
    // 最后再追加 "O"(open) 或 "C"(closed)
    [[nodiscard]] std::string Signature() const {
        std::vector<std::string> parts;
        parts.reserve(fields.size());
        for (const auto &f: fields) parts.push_back(f.name + ":" + std::to_string((int) f.type) + ":" + (f.optional ? "1" : "0") + ":" + (f.is_int_key ? "I" : "S"));
        std::sort(parts.begin(), parts.end());
        std::string sig;
        for (const auto &p: parts) {
            sig += p;
            sig += ';';
        }
        sig += (is_open ? "O" : "C");
        return sig;
    }
};

// 全局 shape 注册表
//
// 设计目标：
//   1. 作为 ShapeType 的 Intern 池——结构相同的 shape 共用同一个 shape_id，
//      避免让代码生成侧为每种等价布局各生成一份 C 结构体。
//   2. 作为 Meet/Widen 的工作台——控制流合并和迭代逼近的最终结果
//      从这里走到代码生成。
//
// ShapeRegistry 是"推断器"和"代码生成器"之间的桥梁：
//   推断器 ─▶ Meet / Widen ─▶ ShapeRegistry ─▶ shape_id ─▶ CGen（偏移访问）
class ShapeRegistry {
public:
    // 收敛策略的参数：
    //   - kWidenFieldThreshold：字段数超过此值时触发开放退化（开放 record 下无法固定偏移布局）
    //   - kWidenIterThreshold ：不动点迭代超过此次数时也必须做保守退化，保证算法终止
    static constexpr int kWidenFieldThreshold = 16;
    static constexpr int kWidenIterThreshold = 3;

    // 入注册表：用 Signature 去重；若已有相同签名则直接复用原来的 id。
    // 这一步完成后 s.shape_id 会被写入内部存储的副本（参数按值传入不直接修改外部）。
    // 返回的 shape_id 即可交给代码生成侧使用。
    //
    // 注意 Intern 会先补全每个字段的 c_field_name——当上层只填了 Lua 字段名、没填 C 名称时，
    // 由 ToSafeCFieldName 自动派生。
    int Intern(ShapeType s) {
        for (auto &f: s.fields)
            if (f.c_field_name.empty()) f.c_field_name = ToSafeCFieldName(f.name);

        const std::string sig = s.Signature();
        if (auto it = sig_to_id_.find(sig); it != sig_to_id_.end()) return it->second;

        const int id = (int) shapes_.size();
        s.shape_id = id;
        shapes_.push_back(std::move(s));
        sig_to_id_[sig] = id;
        return id;
    }

    [[nodiscard]] const ShapeType &Get(int id) const {
        return shapes_.at(id);
    }

    [[nodiscard]] ShapeType &GetMut(int id) {
        return shapes_.at(id);
    }

    [[nodiscard]] int Count() const {
        return (int) shapes_.size();
    }

    // Meet 操作：φ 节点合并两条路径的 shape
    //
    // 这是数据流分析里的"窄化"操作：两个来自不同分支的 table 在 φ 点汇合，
    // 需要得到一个新的 shape，既能覆盖 a 也能覆盖 b。
    //
    // 合并规则（双集合的"最小上界"思路）：
    //   - is_open：只要有一侧是开放的，合并后就是开放的
    //   - a 中存在、b 中不存在的字段：原类型保留，但强制 mandatory→optional
    //   （因为从 b 路径运行时这个字段不在那里——这正是"可能不存在"的来源，
    //    见 feedback_table_spec_soundness.md 中对 absent field 的 offset 限制）
    //   - b 中存在、a 中不存在的字段：同理
    //   - 两侧都存在的字段：类型走 MeetType，optional 取 or
    //
    // 注意：Meet 只处理 *简单类型*（InferredType 枚举），它不知道 HM 类型系统那层；
    // HM 类型的 meet 在 unified_type_analyzer 的另一条路径里（hm_type.h）。
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

    // 两个 InferredType（简单枚举类型）的 meet
    //
    // 规则：
    //   - 相同类型 → 不变
    //   - 任一为 T_DYNAMIC → 返回 T_DYNAMIC（已退化，不能再窄化）
    //   - 任一为 T_NIL → 返回对方（nil 是"尚未确定"的初值）
    //   - int 与 float 互相遇 → float（数值提升）
    //   - 任何其他冲突 → T_DYNAMIC
    static InferredType MeetType(InferredType a, InferredType b) {
        if (a == b) return a;
        if (a == T_DYNAMIC || b == T_DYNAMIC) return T_DYNAMIC;
        if (a == T_NIL) return b;
        if (b == T_NIL) return a;
        if ((a == T_INT && b == T_FLOAT) || (a == T_FLOAT && b == T_INT)) return T_FLOAT;
        return T_DYNAMIC;
    }

    // Widen：迭代次数 / 字段数超限时的收敛策略
    //
    // 目的：为 CFG 上的不动点分析保证终止性。具体策略有两层：
    //
    //   第一层（强收敛）：当 iter_count >= kWidenIterThreshold 或字段数超过阈值时，
    //   尝试把 shape 转为"开放 record"。开放 record 的特点：
    //     - 已知道字段仍然用偏移访问，保留基本性能
    //     - 未知字段走 hash，避免追加字段导致形状无穷膨胀
    //
    //   第二层（退化兜底）：已经是开放 record 又超出字段数限制时，彻底放弃结构信息，
    //   返回 -1（由上层解释为 T_DYNAMIC，运行时改用 CVar 不透明表示）。
    //
    // 这个"截单开放→退化"的两阶段设计，能在绝大多数场景保留字段访问性能，
    // 而不会在（例如递归修改同一个 table）的分析里无限发散。
    int Widen(int shape_id, int iter_count) {
        const ShapeType &s = Get(shape_id);

        if (iter_count >= kWidenIterThreshold || (int) s.fields.size() > kWidenFieldThreshold) {
            if (s.is_open && (int) s.fields.size() > kWidenFieldThreshold) {
                return -1;// 退化为 T_DYNAMIC
            }
            ShapeType w = s;
            w.is_open = true;
            if ((int) w.fields.size() > kWidenFieldThreshold) w.fields.resize(kWidenFieldThreshold);
            return Intern(std::move(w));
        }
        return shape_id;
    }

private:
    std::vector<ShapeType> shapes_;                 // 按 shape_id 下标存取的 shape 存储
    std::unordered_map<std::string, int> sig_to_id_;// 签名到 shape_id 的倒排索引
};

}// namespace fakelua
