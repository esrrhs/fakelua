#include "native/xml/native_xml.h"
#include "native/native_common.h"
#include "native/table/native_table.h"

#include <pugixml.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fakelua::xml {

// ── pugixml node → CVar ──

static CVar node_to_lua(State *s, pugi::xml_node node) {
    if (node.type() == pugi::node_pcdata || node.type() == pugi::node_cdata) {
        return inter::NativeToFakeluaString(s, std::string(node.value()));
    }

    if (node.type() != pugi::node_element) {
        // comments, processing instructions, etc. → empty string
        return inter::NativeToFakeluaString(s, std::string(node.value()));
    }

    // Element node → table
    CVar tbl = table::TableHelper::CreateTable(s);

    // Attributes → "_attr_attrname" (prefix avoids illegal C identifiers)
    for (auto attr = node.first_attribute(); attr; attr = attr.next_attribute()) {
        table::TableHelper::SetTableStrId(s, tbl, std::format("_attr_{}", attr.name()).c_str(),
                                          inter::NativeToFakeluaString(s, std::string(attr.value())));
    }

    // Children: group element children by tag name; text → "_text"
    // Use ordered grouping to preserve document order for same-tag runs.
    struct ChildGroup {
        std::string tag;
        std::vector<pugi::xml_node> nodes;
    };
    std::vector<ChildGroup> groups;
    std::unordered_map<std::string, size_t> group_index;

    for (auto child = node.first_child(); child; child = child.next_sibling()) {
        if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata) {
            std::string text = child.value();
            if (!text.empty()) {
                // Append to existing _text if present
                auto it = group_index.find("_text");
                if (it == group_index.end()) {
                    group_index["_text"] = groups.size();
                    groups.push_back({"_text", {child}});
                } else {
                    groups[it->second].nodes.push_back(child);
                }
            }
            continue;
        }
        if (child.type() != pugi::node_element) continue;
        std::string tag = child.name();
        auto it = group_index.find(tag);
        if (it == group_index.end()) {
            group_index[tag] = groups.size();
            groups.push_back({tag, {child}});
        } else {
            groups[it->second].nodes.push_back(child);
        }
    }

    for (auto &g : groups) {
        if (g.tag == "_text") {
            // Concatenate all text fragments
            std::string combined;
            for (auto &n : g.nodes) combined += n.value();
            table::TableHelper::SetTableStrId(s, tbl, "_text", inter::NativeToFakeluaString(s, combined));
        } else if (g.nodes.size() == 1) {
            table::TableHelper::SetTableStrId(s, tbl, g.tag.c_str(), node_to_lua(s, g.nodes[0]));
        } else {
            // Multiple same-tag siblings → array
            CVar arr = table::TableHelper::CreateTable(s);
            for (size_t i = 0; i < g.nodes.size(); i++) {
                table::TableHelper::SetTableInt(s, arr, static_cast<int64_t>(i + 1), node_to_lua(s, g.nodes[i]));
            }
            table::TableHelper::SetTableStrId(s, tbl, g.tag.c_str(), arr);
        }
    }

    return tbl;
}

// ── CVar → pugixml document ──

static int kMaxXmlDepth = 64;

// Serialize a scalar CVar to its text representation.
static std::string scalar_to_text(CVar v) {
    switch (v.type_) {
    case static_cast<int>(VarType::Bool):
        return AsVar(v).GetBool() ? "true" : "false";
    case static_cast<int>(VarType::Int):
        return std::to_string(v.data_.i);
    case static_cast<int>(VarType::Float): {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.17g", v.data_.f);
        return buf;
    }
    case static_cast<int>(VarType::String):
    case static_cast<int>(VarType::StringId):
        return inter::FakeluaToNativeString(nullptr, v);
    default:
        return "";
    }
}

// Append a key=value attribute to the given node.
static void add_xml_attribute(pugi::xml_node parent, const std::string &attr_name, CVar val) {
    parent.append_attribute(attr_name.c_str()) = scalar_to_text(val).c_str();
}

// Append a child element (or #text) under parent. Recurses for tables.
static void add_xml_child(pugi::xml_document &doc, pugi::xml_node parent, const std::string &key, CVar val, int depth, std::unordered_set<VarTable *> &visited) {
    if (depth > kMaxXmlDepth) {
        ThrowFakeluaException("XML encode: nesting too deep");
    }

    if (key.size() > 6 && key.rfind("_attr_", 0) == 0) {
        add_xml_attribute(parent, key.substr(6), val);
        return;
    }

    if (key == "_text") {
        parent.append_child(pugi::node_pcdata).set_value(scalar_to_text(val).c_str());
        return;
    }

    // Regular child element
    auto child = parent.append_child(key.c_str());

    switch (val.type_) {
    case static_cast<int>(VarType::Nil):
        break;
    case static_cast<int>(VarType::Bool):
    case static_cast<int>(VarType::Int):
    case static_cast<int>(VarType::Float):
    case static_cast<int>(VarType::String):
    case static_cast<int>(VarType::StringId):
        child.append_child(pugi::node_pcdata).set_value(scalar_to_text(val).c_str());
        break;
    case static_cast<int>(VarType::Table): {
        auto *t = val.data_.t;
        if (!t) break;
        if (!visited.insert(t).second) {
            ThrowFakeluaException("XML encode: cyclic table");
        }
        auto kvs = table::TableHelper::CollectKVPairs(val);
        // array detection: contiguous 1..N integer keys
        bool is_array = !kvs.empty();
        int64_t max_idx = 0;
        for (auto &kv : kvs) {
            if (kv.key.type_ != static_cast<int>(VarType::Int)) { is_array = false; break; }
            int64_t k = kv.key.data_.i;
            if (k < 1 || k > 1000000) { is_array = false; break; }
            if (k > max_idx) max_idx = k;
        }
        if (is_array && (max_idx <= 0 || static_cast<size_t>(max_idx) != kvs.size())) {
            is_array = false;
        }
        if (is_array) {
            std::sort(kvs.begin(), kvs.end(), [](const table::TableKV &a, const table::TableKV &b) {
                return a.key.data_.i < b.key.data_.i;
            });
            // Emit each array item as a child named "item" under the parent element.
            for (auto &kv : kvs) {
                add_xml_child(doc, parent, "item", kv.val, depth + 1, visited);
            }
        } else {
            for (auto &kv : kvs) {
                std::string k = inter::FakeluaToNativeString(nullptr, kv.key);
                add_xml_child(doc, child, k, kv.val, depth + 1, visited);
            }
        }
        visited.erase(t);
        break;
    }
    default:
        break;
    }
}

// ── Lua Bindings ──

static CVar xml_decode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "xml.decode", "xml string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string str = inter::FakeluaToNativeString(s, a0);
    pugi::xml_document doc;
    auto result = doc.load_string(str.c_str());
    if (!result) {
        ThrowFakeluaException(std::format("XML parse error: {} at offset {}", result.description(), result.offset));
    }
    // Return the root element as a table; if document has no element root, return empty
    auto root = doc.document_element();
    if (!root) {
        return table::TableHelper::CreateTable(s);
    }
    return node_to_lua(s, root);
}

static CVar xml_encode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "xml.encode", "value expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    try {
        pugi::xml_document doc;
        std::unordered_set<VarTable *> visited;

        if (a0.type_ == static_cast<int>(VarType::Table)) {
            auto *t = a0.data_.t;
            if (!t) return inter::NativeToFakeluaString(s, "");
            if (!visited.insert(t).second) {
                ThrowFakeluaException("XML encode: cyclic table");
            }
            auto kvs = table::TableHelper::CollectKVPairs(a0);
            // Root must be a single element. If table has exactly one string-keyed entry, use it as root.
            // Otherwise wrap in "root".
            if (kvs.size() == 1 && (kvs[0].key.type_ == static_cast<int>(VarType::String) || kvs[0].key.type_ == static_cast<int>(VarType::StringId))) {
                std::string root_name = inter::FakeluaToNativeString(nullptr, kvs[0].key);
                auto root = doc.append_child(root_name.c_str());
                // If the value is a table, merge its children into root directly
                if (kvs[0].val.type_ == static_cast<int>(VarType::Table)) {
                    auto *vt = kvs[0].val.data_.t;
                    if (vt) {
                        if (!visited.insert(vt).second) {
                            ThrowFakeluaException("XML encode: cyclic table");
                        }
                        auto vkvs = table::TableHelper::CollectKVPairs(kvs[0].val);
                        for (auto &vkv : vkvs) {
                            std::string vk = inter::FakeluaToNativeString(nullptr, vkv.key);
                            add_xml_child(doc, root, vk, vkv.val, 1, visited);
                        }
                        visited.erase(vt);
                    }
                } else {
                    root.append_child(pugi::node_pcdata).set_value(scalar_to_text(kvs[0].val).c_str());
                }
            } else {
                auto root = doc.append_child("root");
                for (auto &kv : kvs) {
                    std::string k = inter::FakeluaToNativeString(nullptr, kv.key);
                    add_xml_child(doc, root, k, kv.val, 1, visited);
                }
            }
            visited.erase(t);
        } else {
            // Top-level scalar: wrap in root with text
            auto root = doc.append_child("root");
            std::string txt;
            switch (a0.type_) {
            case static_cast<int>(VarType::Bool): txt = AsVar(a0).GetBool() ? "true" : "false"; break;
            case static_cast<int>(VarType::Int): txt = std::to_string(a0.data_.i); break;
            case static_cast<int>(VarType::Float): { char b[64]; snprintf(b, sizeof(b), "%.17g", a0.data_.f); txt = b; break; }
            case static_cast<int>(VarType::String):
            case static_cast<int>(VarType::StringId): txt = inter::FakeluaToNativeString(nullptr, a0); break;
            default: break;
            }
            root.append_child(pugi::node_pcdata).set_value(txt.c_str());
        }

        std::stringstream ss;
        doc.save(ss, "  ");
        return inter::NativeToFakeluaString(s, ss.str());
    } catch (const std::exception &e) {
        ThrowFakeluaException(std::format("XML encode error: {}", e.what()));
    }
}

void RegisterXmlLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "xml.decode", 1, false, xml_decode);
    RegisterNativeFunction(s, "xml.encode", 1, false, xml_encode);
}

}  // namespace fakelua::xml
