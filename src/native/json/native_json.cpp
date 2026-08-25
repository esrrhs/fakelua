#include "native/json/native_json.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "var/var_table.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace fakelua::json {

static constexpr int kMaxJsonDepth = 64;

// ── JSON Value ──

struct JsonValue {
    enum Type { NIL, BOOL, INT, FLOAT, STRING, ARRAY, OBJECT };
    Type type = NIL;
    bool b = false;
    int64_t i = 0;
    double f = 0.0;
    std::string s;
    std::vector<JsonValue> arr;
    std::vector<std::pair<std::string, std::unique_ptr<JsonValue>>> obj;
};

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

    JsonValue parse_value(int depth = 0) {
        if (depth > kMaxJsonDepth) {
            ThrowFakeluaException("JSON parse error: nesting too deep");
        }
        uint8_t c;
        if (!peek(c)) {
            ThrowFakeluaException("JSON parse error: unexpected end of input");
        }

        switch (c) {
        case 'n': return parse_null();
        case 't': case 'f': return parse_bool();
        case '"': return parse_string();
        case '[': return parse_array(depth);
        case '{': return parse_object(depth);
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

        auto is_digit = [&]() -> bool {
            return pos < len && data[pos] >= '0' && data[pos] <= '9';
        };

        if (pos < len && data[pos] == '-') ++pos;

        // RFC 8259: int = zero / ( digit1-9 *DIGIT )
        if (!is_digit()) {
            ThrowFakeluaException("JSON parse error: invalid number");
        }
        if (data[pos] == '0') {
            ++pos;
            if (is_digit()) {
                ThrowFakeluaException("JSON parse error: invalid number");
            }
        } else {
            while (is_digit()) ++pos;
        }

        // frac = decimal-point 1*DIGIT
        if (pos < len && data[pos] == '.') {
            is_float = true;
            ++pos;
            if (!is_digit()) {
                ThrowFakeluaException("JSON parse error: invalid number");
            }
            while (is_digit()) ++pos;
        }

        // exp = e [ minus / plus ] 1*DIGIT
        if (pos < len && (data[pos] == 'e' || data[pos] == 'E')) {
            is_float = true;
            ++pos;
            if (pos < len && (data[pos] == '+' || data[pos] == '-')) ++pos;
            if (!is_digit()) {
                ThrowFakeluaException("JSON parse error: invalid number");
            }
            while (is_digit()) ++pos;
        }

        std::string num_str(reinterpret_cast<const char*>(data + start), pos - start);
        JsonValue v;
        size_t idx = 0;
        try {
            if (is_float) {
                v.type = JsonValue::FLOAT;
                v.f = std::stod(num_str, &idx);
            } else {
                v.type = JsonValue::INT;
                v.i = std::stoll(num_str, &idx);
            }
        } catch (const std::exception &) {
            ThrowFakeluaException("JSON parse error: invalid number");
        }
        if (idx != num_str.size()) {
            ThrowFakeluaException("JSON parse error: invalid number");
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
                    auto parse_hex4 = [&]() -> uint32_t {
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
                        return cp;
                    };
                    uint32_t cp = parse_hex4();
                    if (cp >= 0xD800 && cp <= 0xDBFF) {
                        if (pos + 6 <= len && data[pos] == '\\' && data[pos + 1] == 'u') {
                            pos += 2;
                            uint32_t low = parse_hex4();
                            if (low < 0xDC00 || low > 0xDFFF) {
                                ThrowFakeluaException("JSON parse error: invalid UTF-16 surrogate pair");
                            }
                            cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                        } else {
                            ThrowFakeluaException("JSON parse error: lone UTF-16 surrogate");
                        }
                    } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
                        ThrowFakeluaException("JSON parse error: lone UTF-16 surrogate");
                    }
                    if (cp < 0x80) {
                        result.push_back(static_cast<char>(cp));
                    } else if (cp < 0x800) {
                        result.push_back(static_cast<char>(0xC0 | (cp >> 6)));
                        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                    } else if (cp < 0x10000) {
                        result.push_back(static_cast<char>(0xE0 | (cp >> 12)));
                        result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                    } else {
                        result.push_back(static_cast<char>(0xF0 | (cp >> 18)));
                        result.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
                        result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                    }
                    break;
                }
                default:
                    ThrowFakeluaException(std::format("JSON parse error: invalid escape character '{}'", (char)esc));
                }
            } else {
                if (c < 0x20) {
                    ThrowFakeluaException("JSON parse error: unescaped control character in string");
                }
                result.push_back(static_cast<char>(c));
            }
        }

        ThrowFakeluaException("JSON parse error: unterminated string");
    }

    JsonValue parse_array(int depth) {
        expect('[');
        JsonValue v{JsonValue::ARRAY};

        uint8_t c;
        if (peek(c) && c == ']') { ++pos; return v; }

        while (true) {
            v.arr.push_back(parse_value(depth + 1));
            if (!peek(c)) ThrowFakeluaException("JSON parse error: unterminated array");
            if (c == ']') { ++pos; break; }
            expect(',');
        }
        return v;
    }

    JsonValue parse_object(int depth) {
        expect('{');
        JsonValue v{JsonValue::OBJECT};

        uint8_t c;
        if (peek(c) && c == '}') { ++pos; return v; }

        while (true) {
            if (!peek(c) || c != '"') ThrowFakeluaException("JSON parse error: expected string key in object");
            JsonValue key = parse_string();
            expect(':');
            v.obj.emplace_back(std::move(key.s), std::make_unique<JsonValue>(parse_value(depth + 1)));
            if (!peek(c)) ThrowFakeluaException("JSON parse error: unterminated object");
            if (c == '}') { ++pos; break; }
            expect(',');
        }
        return v;
    }
};

// ── Convert JsonValue to Lua CVar ──

static CVar json_to_lua(State *s, const JsonValue &v) {
    switch (v.type) {
    case JsonValue::NIL:
        return inter::NativeToFakeluaNil(s);
    case JsonValue::BOOL:
        return inter::NativeToFakeluaBool(s, v.b);
    case JsonValue::INT:
        return inter::NativeToFakeluaLonglong(s, v.i);
    case JsonValue::FLOAT:
        return inter::NativeToFakeluaDouble(s, v.f);
    case JsonValue::STRING:
        return inter::NativeToFakeluaString(s, v.s);
    case JsonValue::ARRAY: {
        CVar tbl = table::TableHelper::CreateTable(s);
        for (size_t i = 0; i < v.arr.size(); i++) {
            CVar elem = json_to_lua(s, v.arr[i]);
            table::TableHelper::SetTableInt(s, tbl, static_cast<int64_t>(i + 1), elem);
        }
        return tbl;
    }
    case JsonValue::OBJECT: {
        CVar tbl = table::TableHelper::CreateTable(s);
        for (auto &kv : v.obj) {
            CVar val = json_to_lua(s, *kv.second);
            table::TableHelper::SetTableStrId(s, tbl, kv.first.c_str(), val);
        }
        return tbl;
    }
    }
    return inter::NativeToFakeluaNil(s);
}

// ── Convert Lua CVar to JsonValue ──

static std::string cvar_to_json_key(CVar k) {
    switch (k.type_) {
    case static_cast<int>(VarType::String):
    case static_cast<int>(VarType::StringId):
        return inter::FakeluaToNativeString(nullptr, k);
    case static_cast<int>(VarType::Int):
        return std::to_string(k.data_.i);
    case static_cast<int>(VarType::Float): {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.17g", k.data_.f);
        return buf;
    }
    case static_cast<int>(VarType::Bool):
        return AsVar(k).GetBool() ? "true" : "false";
    default:
        ThrowFakeluaException(std::format("JSON encode: unsupported object key type {}",
                                          VarTypeToString(AsVar(k).Type())));
    }
}

static JsonValue lua_to_json(CVar v, int depth, std::unordered_set<VarTable *> &visited) {
    if (depth > kMaxJsonDepth) {
        ThrowFakeluaException("JSON encode: nesting too deep");
    }
    switch (v.type_) {
    case static_cast<int>(VarType::Nil):
        return JsonValue{JsonValue::NIL};
    case static_cast<int>(VarType::Bool):
        return {JsonValue::BOOL, AsVar(v).GetBool()};
    case static_cast<int>(VarType::Int):
        return {JsonValue::INT, false, v.data_.i};
    case static_cast<int>(VarType::Float):
        return {JsonValue::FLOAT, false, 0, v.data_.f};
    case static_cast<int>(VarType::String):
    case static_cast<int>(VarType::StringId): {
        std::string str = inter::FakeluaToNativeString(nullptr, v);
        return {JsonValue::STRING, false, 0, 0.0, std::move(str)};
    }
    case static_cast<int>(VarType::Table): {
        auto *t = v.data_.t;
        if (!t) return {JsonValue::NIL};
        if (!visited.insert(t).second) {
            ThrowFakeluaException("JSON encode: cyclic table");
        }

        auto kvs = table::TableHelper::CollectKVPairs(v);
        bool is_array = !kvs.empty();
        int64_t max_idx = 0;
        for (auto &kv : kvs) {
            if (kv.key.type_ != static_cast<int>(VarType::Int)) {
                is_array = false;
                break;
            }
            int64_t key = kv.key.data_.i;
            if (key < 1 || key > 1000000) {
                is_array = false;
                break;
            }
            if (key > max_idx) max_idx = key;
        }
        if (is_array && (max_idx <= 0 || static_cast<size_t>(max_idx) != kvs.size())) {
            is_array = false;
        }

        JsonValue result;
        if (is_array) {
            std::sort(kvs.begin(), kvs.end(), [](const table::TableKV &a, const table::TableKV &b) {
                return a.key.data_.i < b.key.data_.i;
            });
            result = JsonValue{JsonValue::ARRAY};
            result.arr.reserve(kvs.size());
            for (auto &kv : kvs) {
                result.arr.push_back(lua_to_json(kv.val, depth + 1, visited));
            }
        } else {
            result = JsonValue{JsonValue::OBJECT};
            for (auto &kv : kvs) {
                result.obj.emplace_back(cvar_to_json_key(kv.key),
                                        std::make_unique<JsonValue>(lua_to_json(kv.val, depth + 1, visited)));
            }
        }
        visited.erase(t);
        return result;
    }
    default:
        ThrowFakeluaException(std::format("JSON encode: unsupported type {}", VarTypeToString(AsVar(v).Type())));
    }
}

// ── JSON Encoder ──

static void json_encode_val(std::string &out, const JsonValue &v) {
    switch (v.type) {
    case JsonValue::NIL:
        out += "null";
        break;
    case JsonValue::BOOL:
        out += v.b ? "true" : "false";
        break;
    case JsonValue::INT:
        out += std::to_string(v.i);
        break;
    case JsonValue::FLOAT: {
        if (!std::isfinite(v.f)) {
            out += "null";
            break;
        }
        char buf[64];
        snprintf(buf, sizeof(buf), "%.17g", v.f);
        out += buf;
        break;
    }
    case JsonValue::STRING:
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
    case JsonValue::ARRAY:
        out += '[';
        for (size_t i = 0; i < v.arr.size(); i++) {
            if (i > 0) out += ',';
            json_encode_val(out, v.arr[i]);
        }
        out += ']';
        break;
    case JsonValue::OBJECT:
        out += '{';
        for (size_t i = 0; i < v.obj.size(); i++) {
            if (i > 0) out += ',';
            JsonValue key{JsonValue::STRING, false, 0, 0.0, v.obj[i].first};
            json_encode_val(out, key);
            out += ':';
            json_encode_val(out, *v.obj[i].second);
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
    parser.skip_ws();
    if (parser.pos < parser.len) {
        ThrowFakeluaException("JSON parse error: trailing garbage");
    }
    return json_to_lua(s, result);
}

// json.encode(value) → JSON string
static CVar json_encode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "json.encode", "value expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::unordered_set<VarTable *> visited;
    auto jv = lua_to_json(a0, 0, visited);
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
