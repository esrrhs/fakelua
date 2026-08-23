#include "native/json/native_json.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "var/var_table.h"

#include <cstdint>
#include <string>
#include <vector>

namespace fakelua::json {

// ── JSON Parser ──

struct JsonParser {
    const uint8_t *data;
    size_t len;
    size_t pos = 0;

    JsonParser(const uint8_t *d, size_t l) : data(d), len(l) {}

    void skip_ws() {
        while (pos < len) {
            uint8_t c = data[pos];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                ++pos;
            } else {
                break;
            }
        }
    }

    bool peek(uint8_t &c) {
        skip_ws();
        if (pos < len) { c = data[pos]; return true; }
        return false;
    }

    void expect(uint8_t c) {
        skip_ws();
        if (pos >= len || data[pos] != c) {
            ThrowFakeluaException(std::format("JSON parse error: expected '{}' at pos {}", (char)c, pos));
        }
        ++pos;
    }

    // Parse a JSON value, return as string (for encoding to Lua)
    // We build a Lua value directly
    struct JsonValue {
        enum Type { NIL, BOOL, INT, FLOAT, STRING, ARRAY, OBJECT };
        Type type = NIL;
        bool b = false;
        int64_t i = 0;
        double f = 0.0;
        std::string s;
        std::vector<JsonValue> arr;
        std::vector<std::pair<std::string, JsonValue>> obj;
    };

    JsonValue parse_value() {
        uint8_t c;
        if (!peek(c)) {
            ThrowFakeluaException("JSON parse error: unexpected end of input");
        }

        switch (c) {
        case 'n': return parse_null();
        case 't': case 'f': return parse_bool();
        case '"': return parse_string();
        case '[': return parse_array();
        case '{': return parse_object();
        default:
            if (c == '-' || (c >= '0' && c <= '9')) return parse_number();
            ThrowFakeluaException(std::format("JSON parse error: unexpected character '{}' at pos {}", (char)c, pos));
        }
    }

private:
    JsonValue parse_null() {
        if (pos + 3 < len && data[pos+1] == 'u' && data[pos+2] == 'l' && data[pos+3] == 'l') {
            pos += 4;
            return JsonValue{JsonValue::NIL};
        }
        ThrowFakeluaException("JSON parse error: invalid null");
    }

    JsonValue parse_bool() {
        JsonValue v{JsonValue::BOOL};
        if (pos + 3 < len && data[pos+1] == 'r' && data[pos+2] == 'u' && data[pos+3] == 'e') {
            v.b = true; pos += 4;
        } else if (pos + 4 < len && data[pos+1] == 'a' && data[pos+2] == 'l' && data[pos+3] == 's' && data[pos+4] == 'e') {
            v.b = false; pos += 5;
        } else {
            ThrowFakeluaException("JSON parse error: invalid boolean");
        }
        return v;
    }

    JsonValue parse_number() {
        size_t start = pos;
        bool is_float = false;

        if (pos < len && data[pos] == '-') ++pos;

        while (pos < len) {
            uint8_t c = data[pos];
            if (c >= '0' && c <= '9') {
                ++pos;
            } else if (c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-') {
                is_float = true;
                ++pos;
            } else {
                break;
            }
        }

        std::string num_str(reinterpret_cast<const char*>(data + start), pos - start);

        JsonValue v;
        if (is_float) {
            v.type = JsonValue::FLOAT;
            v.f = std::stod(num_str);
        } else {
            v.type = JsonValue::INT;
            v.i = std::stoll(num_str);
        }
        return v;
    }

    JsonValue parse_string() {
        expect('"');
        std::string result;

        while (pos < len) {
            uint8_t c = data[pos++];
            if (c == '"') {
                return JsonValue{JsonValue::STRING, false, 0, 0.0, std::move(result)};
            }
            if (c == '\\') {
                if (pos >= len) ThrowFakeluaException("JSON parse error: unexpected end in string escape");
                uint8_t esc = data[pos++];
                switch (esc) {
                case '"': result.push_back('"'); break;
                case '\\': result.push_back('\\'); break;
                case '/': result.push_back('/'); break;
                case 'n': result.push_back('\n'); break;
                case 'r': result.push_back('\r'); break;
                case 't': result.push_back('\t'); break;
                case 'b': result.push_back('\b'); break;
                case 'f': result.push_back('\f'); break;
                case 'u': {
                    if (pos + 4 > len) ThrowFakeluaException("JSON parse error: invalid \\u escape");
                    uint32_t cp = 0;
                    for (int k = 0; k < 4; k++) {
                        uint8_t h = data[pos++];
                        cp <<= 4;
                        if (h >= '0' && h <= '9') cp |= (h - '0');
                        else if (h >= 'a' && h <= 'f') cp |= (h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F') cp |= (h - 'A' + 10);
                        else ThrowFakeluaException("JSON parse error: invalid hex in \\u escape");
                    }
                    // Encode UTF-8
                    if (cp < 0x80) {
                        result.push_back(static_cast<char>(cp));
                    } else if (cp < 0x800) {
                        result.push_back(static_cast<char>(0xC0 | (cp >> 6)));
                        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                    } else {
                        result.push_back(static_cast<char>(0xE0 | (cp >> 12)));
                        result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                    }
                    break;
                }
                default:
                    ThrowFakeluaException(std::format("JSON parse error: invalid escape character '{}'", (char)esc));
                }
            } else {
                result.push_back(static_cast<char>(c));
            }
        }

        ThrowFakeluaException("JSON parse error: unterminated string");
    }

    JsonValue parse_array() {
        expect('[');
        JsonValue v{JsonValue::ARRAY};

        uint8_t c;
        if (peek(c) && c == ']') { ++pos; return v; }

        while (true) {
            v.arr.push_back(parse_value());
            if (!peek(c)) ThrowFakeluaException("JSON parse error: unterminated array");
            if (c == ']') { ++pos; break; }
            expect(',');
        }
        return v;
    }

    JsonValue parse_object() {
        expect('{');
        JsonValue v{JsonValue::OBJECT};

        uint8_t c;
        if (peek(c) && c == '}') { ++pos; return v; }

        while (true) {
            if (!peek(c) || c != '"') ThrowFakeluaException("JSON parse error: expected string key in object");
            JsonValue key = parse_string();
            expect(':');
            v.obj.emplace_back(std::move(key.s), parse_value());
            if (!peek(c)) ThrowFakeluaException("JSON parse error: unterminated object");
            if (c == '}') { ++pos; break; }
            expect(',');
        }
        return v;
    }
};

// ── Convert JsonValue to Lua CVar ──

static CVar json_to_lua(State *s, const JsonParser::JsonValue &v) {
    switch (v.type) {
    case JsonParser::JsonValue::NIL:
        return inter::NativeToFakeluaNil(s);
    case JsonParser::JsonValue::BOOL:
        return inter::NativeToFakeluaBool(s, v.b);
    case JsonParser::JsonValue::INT:
        return inter::NativeToFakeluaInt(s, v.i);
    case JsonParser::JsonValue::FLOAT:
        return inter::NativeToFakeluaFloat(s, v.f);
    case JsonParser::JsonValue::STRING:
        return inter::NativeToFakeluaString(s, v.s);
    case JsonParser::JsonValue::ARRAY: {
        CVar tbl = table::TableHelper::CreateTable(s);
        for (size_t i = 0; i < v.arr.size(); i++) {
            CVar elem = json_to_lua(s, v.arr[i]);
            table::TableHelper::SetTableInt(s, tbl, static_cast<int64_t>(i + 1), elem);
        }
        return tbl;
    }
    case JsonParser::JsonValue::OBJECT: {
        CVar tbl = table::TableHelper::CreateTable(s);
        for (auto &kv : v.obj) {
            CVar val = json_to_lua(s, kv.second);
            table::TableHelper::SetTableStrId(s, tbl, kv.first.c_str(), val);
        }
        return tbl;
    }
    }
    return inter::NativeToFakeluaNil(s);
}

// ── Convert Lua CVar to JsonValue ──

static JsonParser::JsonValue lua_to_json(CVar v) {
    switch (v.type_) {
    case static_cast<int>(VarType::Nil):
        return JsonParser::JsonValue{JsonParser::JsonValue::NIL};
    case static_cast<int>(VarType::Bool):
        return {JsonParser::JsonValue::BOOL, AsVar(v).GetBool()};
    case static_cast<int>(VarType::Int):
        return {JsonParser::JsonValue::INT, false, v.data_.i};
    case static_cast<int>(VarType::Float):
        return {JsonParser::JsonValue::FLOAT, false, 0, v.data_.f};
    case static_cast<int>(VarType::String):
    case static_cast<int>(VarType::StringId): {
        std::string str = inter::FakeluaToNativeString(nullptr, v);
        return {JsonParser::JsonValue::STRING, false, 0, 0.0, std::move(str)};
    }
    case static_cast<int>(VarType::Table): {
        auto *t = v.data_.t;
        if (!t) return {JsonParser::JsonValue::NIL};

        // Collect all key-value pairs
        std::vector<std::pair<CVar, CVar>> kvs;
        if (t->spec_keys && t->spec_vals && t->spec_count > 0) {
            for (uint32_t i = 0; i < t->spec_count; i++) {
                if (t->spec_keys[i].type_ == static_cast<int>(VarType::Nil)) continue;
                kvs.emplace_back(t->spec_keys[i], t->spec_vals[i]);
            }
        }
        for (uint32_t i = 0; i < VarTable::QUICK_DATA_SIZE; i++) {
            if (t->quick_data_[i].key.type_ == static_cast<int>(VarType::Nil)) continue;
            kvs.emplace_back(t->quick_data_[i].key, t->quick_data_[i].val);
        }
        if (t->nodes_ && t->bucket_count_ > 0 && t->active_list_) {
            for (uint32_t i = 0; i < t->count_; i++) {
                uint32_t node_idx = t->active_list_[i];
                auto &entry = t->nodes_[node_idx].entry;
                if (entry.key.type_ == static_cast<int>(VarType::Nil)) continue;
                kvs.emplace_back(entry.key, entry.val);
            }
        }

        // Check if it's an array: all keys are consecutive ints starting from 1
        bool is_array = !kvs.empty();
        int max_idx = 0;
        for (auto &kv : kvs) {
            if (kv.first.type_ != static_cast<int>(VarType::Int)) {
                is_array = false;
                break;
            }
            int64_t key = kv.first.data_.i;
            if (key < 1 || key > 1000000) { is_array = false; break; }
            if (static_cast<int>(key) > max_idx) max_idx = static_cast<int>(key);
        }
        if (max_idx > 0 && static_cast<uint32_t>(max_idx) != kvs.size()) {
            is_array = false;
        }

        if (is_array && !kvs.empty()) {
            JsonParser::JsonValue arr{JsonParser::JsonValue::ARRAY};
            arr.arr.reserve(kvs.size());
            for (auto &kv : kvs) {
                arr.arr.push_back(lua_to_json(kv.second));
            }
            return arr;
        } else {
            JsonParser::JsonValue obj{JsonParser::JsonValue::OBJECT};
            for (auto &kv : kvs) {
                std::string key = inter::FakeluaToNativeString(nullptr, kv.first);
                obj.obj.emplace_back(std::move(key), lua_to_json(kv.second));
            }
            return obj;
        }
    }
    default:
        ThrowFakeluaException(std::format("JSON encode: unsupported type {}", VarTypeToString(AsVar(v).Type())));
    }
}

// ── JSON Encoder ──

static void json_encode_val(std::string &out, const JsonParser::JsonValue &v) {
    switch (v.type) {
    case JsonParser::JsonValue::NIL:
        out += "null";
        break;
    case JsonParser::JsonValue::BOOL:
        out += v.b ? "true" : "false";
        break;
    case JsonParser::JsonValue::INT:
        out += std::to_string(v.i);
        break;
    case JsonParser::JsonValue::FLOAT: {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.17g", v.f);
        out += buf;
        break;
    }
    case JsonParser::JsonValue::STRING:
        out += '"';
        for (uint8_t c : v.s) {
            switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
            }
        }
        out += '"';
        break;
    case JsonParser::JsonValue::ARRAY:
        out += '[';
        for (size_t i = 0; i < v.arr.size(); i++) {
            if (i > 0) out += ',';
            json_encode_val(out, v.arr[i]);
        }
        out += ']';
        break;
    case JsonParser::JsonValue::OBJECT:
        out += '{';
        for (size_t i = 0; i < v.obj.size(); i++) {
            if (i > 0) out += ',';
            JsonParser::JsonValue key{JsonParser::JsonValue::STRING, false, 0, 0.0, v.obj[i].first};
            json_encode_val(out, key);
            out += ':';
            json_encode_val(out, v.obj[i].second);
        }
        out += '}';
        break;
    }
}

// ── Lua Bindings ──

// json.decode(json_str) → Lua value
static CVar json_decode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "json.decode", "json string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string str = inter::FakeluaToNativeString(s, a0);

    JsonParser parser(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    auto result = parser.parse_value();
    return json_to_lua(s, result);
}

// json.encode(value) → JSON string
static CVar json_encode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "json.encode", "value expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    auto jv = lua_to_json(a0);
    std::string out;
    out.reserve(256);
    json_encode_val(out, jv);
    return inter::NativeToFakeluaString(s, out);
}

void RegisterJsonLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "json.decode", 1, false, json_decode);
    RegisterNativeFunction(s, "json.encode", 1, false, json_encode);
}

}  // namespace fakelua::json
