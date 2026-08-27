#include "native/ini/native_ini.h"
#include "native/native_common.h"
#include "native/table/native_table.h"

#include <ini.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace fakelua::ini {

// ── Value type inference (same rules as csv field_to_lua) ──

static CVar value_to_lua(State *s, const std::string &str) {
    if (str.empty()) return inter::NativeToFakeluaString(s, str);

    // Bool
    if (str == "true" || str == "True" || str == "TRUE" ||
        str == "yes" || str == "Yes" || str == "YES" ||
        str == "on" || str == "On" || str == "ON") {
        return inter::NativeToFakeluaBool(s, true);
    }
    if (str == "false" || str == "False" || str == "FALSE" ||
        str == "no" || str == "No" || str == "NO" ||
        str == "off" || str == "Off" || str == "OFF") {
        return inter::NativeToFakeluaBool(s, false);
    }

    // null
    if (str == "~" || str == "null" || str == "Null" || str == "NULL") {
        return inter::NativeToFakeluaNil(s);
    }

    // Integer
    const char *start = str.c_str();
    if (str.size() > 1 && str[0] == '0' && str[1] >= '0' && str[1] <= '9') {
        return inter::NativeToFakeluaString(s, str);  // leading-zero → string
    }
    char *end = nullptr;
    long long ival = strtoll(start, &end, 10);
    if (end && *end == '\0' && end != start) {
        return inter::NativeToFakeluaLonglong(s, ival);
    }
    // Float
    char *fend = nullptr;
    double dval = strtod(start, &fend);
    if (fend && *fend == '\0' && fend != start && std::isfinite(dval)) {
        return inter::NativeToFakeluaDouble(s, dval);
    }
    return inter::NativeToFakeluaString(s, str);
}

// ── Decode ──

// Stable storage for section tables during parsing.
struct IniDecodeState {
    State *state;
    CVar root;
    // Stable storage: section name → index into section_cvors
    std::unordered_map<std::string, size_t> index;
    struct SectionEntry {
        std::string name;
        CVar tbl;
    };
    std::vector<SectionEntry> sections;
};

static int ini_handler_v2(void *user, const char *section, const char *name, const char *value) {
    auto *st = static_cast<IniDecodeState *>(user);
    if (!section || !name) return 1;
    std::string sec_name(section);
    size_t idx;
    auto it = st->index.find(sec_name);
    if (it == st->index.end()) {
        idx = st->sections.size();
        st->index[sec_name] = idx;
        st->sections.push_back({sec_name, table::TableHelper::CreateTable(st->state)});
    } else {
        idx = it->second;
    }
    table::TableHelper::SetTableStrId(st->state, st->sections[idx].tbl, name, value_to_lua(st->state, value ? value : ""));
    return 1;
}

static CVar ini_decode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "ini.decode", "ini string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string str = inter::FakeluaToNativeString(s, a0);

    IniDecodeState st;
    st.state = s;
    st.root = table::TableHelper::CreateTable(s);

    int rc = ini_parse_string(str.c_str(), ini_handler_v2, &st);
    if (rc < 0) {
        ThrowFakeluaException("INI parse error: invalid input");
    }
    // rc > 0 means line number of first error; still return what we parsed, but warn via exception
    // (inih returns rc = 0 on success, rc = line of first error otherwise; rc < 0 = error code)

    for (auto &sec : st.sections) {
        table::TableHelper::SetTableStrId(s, st.root, sec.name.c_str(), sec.tbl);
    }
    return st.root;
}

// ── Encode ──

static std::string cvar_to_ini_value(CVar v) {
    switch (v.type_) {
    case static_cast<int>(VarType::Int):
        return std::to_string(v.data_.i);
    case static_cast<int>(VarType::Float): {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.15g", v.data_.f);
        return buf;
    }
    case static_cast<int>(VarType::Bool):
        return AsVar(v).GetBool() ? "true" : "false";
    case static_cast<int>(VarType::Nil):
        return "";
    default:
        return inter::FakeluaToNativeString(nullptr, v);
    }
}

static CVar ini_encode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "ini.encode", "value expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);

    if (a0.type_ != static_cast<int>(VarType::Table)) {
        ThrowBadArgument(1, "ini.encode", "table expected (top-level sections)");
    }

    std::string out;
    auto kvs = table::TableHelper::CollectKVPairs(a0);

    // Sort sections by key name for deterministic output
    std::sort(kvs.begin(), kvs.end(), [](const table::TableKV &a, const table::TableKV &b) {
        std::string ak = inter::FakeluaToNativeString(nullptr, a.key);
        std::string bk = inter::FakeluaToNativeString(nullptr, b.key);
        return ak < bk;
    });

    bool first_section = true;
    for (auto &kv : kvs) {
        if (kv.val.type_ != static_cast<int>(VarType::Table)) continue;  // non-table top-level entries skipped
        std::string sec_name = inter::FakeluaToNativeString(nullptr, kv.key);
        auto *t = kv.val.data_.t;
        if (!t) continue;

        if (!first_section) out += '\n';
        first_section = false;
        out += '[';
        out += sec_name;
        out += "]\n";

        auto sec_kvs = table::TableHelper::CollectKVPairs(kv.val);
        // Sort keys within section
        std::sort(sec_kvs.begin(), sec_kvs.end(), [](const table::TableKV &a, const table::TableKV &b) {
            std::string ak = inter::FakeluaToNativeString(nullptr, a.key);
            std::string bk = inter::FakeluaToNativeString(nullptr, b.key);
            return ak < bk;
        });

        for (auto &skv : sec_kvs) {
            std::string key = inter::FakeluaToNativeString(nullptr, skv.key);
            out += key;
            out += " = ";
            if (skv.val.type_ == static_cast<int>(VarType::Table)) {
                // INI has no nested tables; encode array as comma-separated
                auto arr_kvs = table::TableHelper::CollectKVPairs(skv.val);
                std::sort(arr_kvs.begin(), arr_kvs.end(), [](const table::TableKV &a, const table::TableKV &b) {
                    if (a.key.type_ == static_cast<int>(VarType::Int) && b.key.type_ == static_cast<int>(VarType::Int))
                        return a.key.data_.i < b.key.data_.i;
                    return false;
                });
                for (size_t i = 0; i < arr_kvs.size(); i++) {
                    if (i > 0) out += ", ";
                    out += cvar_to_ini_value(arr_kvs[i].val);
                }
            } else {
                out += cvar_to_ini_value(skv.val);
            }
            out += '\n';
        }
    }

    return inter::NativeToFakeluaString(s, out);
}

void RegisterIniLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "ini.decode", 1, false, ini_decode);
    RegisterNativeFunction(s, "ini.encode", 1, false, ini_encode);
}

}  // namespace fakelua::ini
