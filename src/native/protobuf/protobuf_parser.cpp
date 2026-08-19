#include "protobuf_parser.h"

#include "util/exception.h"

#include <cctype>
#include <cstring>
#include <format>
#include <unordered_map>
#include <vector>

namespace fakelua::protobuf {

// ─── 关键字 ───

static const std::unordered_map<std::string, int> &Keywords() {
    static const std::unordered_map<std::string, int> kw = {
        {"syntax", 1},  {"package", 2},   {"import", 3},     {"message", 4},
        {"enum", 5},    {"oneof", 6},     {"map", 7},        {"repeated", 8},
        {"optional", 9}, {"reserved", 10}, {"extensions", 11}, {"option", 12},
        {"to", 13},     {"true", 14},     {"false", 15},     {"public", 16},
        {"weak", 17},
    };
    return kw;
}

// ─── 标量类型名 → FieldType ───

static FieldType ParseScalarType(const std::string &name) {
    if (name == "double") return TYPE_DOUBLE;
    if (name == "float") return TYPE_FLOAT;
    if (name == "int64") return TYPE_INT64;
    if (name == "uint64") return TYPE_UINT64;
    if (name == "int32") return TYPE_INT32;
    if (name == "fixed64") return TYPE_FIXED64;
    if (name == "fixed32") return TYPE_FIXED32;
    if (name == "bool") return TYPE_BOOL;
    if (name == "string") return TYPE_STRING;
    if (name == "bytes") return TYPE_BYTES;
    if (name == "uint32") return TYPE_UINT32;
    if (name == "enum") return TYPE_ENUM;
    if (name == "sfixed32") return TYPE_SFIXED32;
    if (name == "sfixed64") return TYPE_SFIXED64;
    if (name == "sint32") return TYPE_SINT32;
    if (name == "sint64") return TYPE_SINT64;
    if (name == "message") return TYPE_MESSAGE;
    return TYPE_INT32;  // 未知类型，后续 resolver 会处理
}

static bool IsScalarType(const std::string &name) {
    return name == "double" || name == "float" || name == "int64" || name == "uint64" ||
           name == "int32" || name == "fixed64" || name == "fixed32" || name == "bool" ||
           name == "string" || name == "bytes" || name == "uint32" || name == "sfixed32" ||
           name == "sfixed64" || name == "sint32" || name == "sint64";
}

// ─── Lexer ───

struct Token {
    enum Type {
        T_EOF = 0,
        T_IDENT,
        T_INT,
        T_FLOAT,
        T_STRING,
        T_SYMBOL,   // { } = ; < > , ( ) [ ] .
        T_KEYWORD,
    };
    Type type = T_EOF;
    std::string value;
    int keyword_id = 0;  // 仅 T_KEYWORD
    int line = 1;
};

class Lexer {
public:
    explicit Lexer(const std::string &text) : text_(text), pos_(0), line_(1) {}

    Token Next() {
        SkipWhitespaceAndComments();
        if (pos_ >= text_.size()) return {Token::T_EOF, "", 0, line_};

        char c = text_[pos_];
        int start_line = line_;

        // 字符串
        if (c == '"' || c == '\'') {
            return ReadString(c, start_line);
        }
        // 数字
        if (std::isdigit(c) || (c == '.' && pos_ + 1 < text_.size() && std::isdigit(text_[pos_ + 1]))) {
            return ReadNumber(start_line);
        }
        // 标识符 / 关键字
        if (std::isalpha(c) || c == '_') {
            return ReadIdent(start_line);
        }
        // 符号
        if (c == '{' || c == '}' || c == '=' || c == ';' || c == '<' || c == '>' ||
            c == ',' || c == '(' || c == ')' || c == '[' || c == ']' || c == '.') {
            pos_++;
            return {Token::T_SYMBOL, std::string(1, c), 0, start_line};
        }

        ThrowFakeluaException(std::format("proto parse error at line {}: unexpected character '{}'", start_line, c));
    }

    int Line() const { return line_; }

private:
    void SkipWhitespaceAndComments() {
        while (pos_ < text_.size()) {
            char c = text_[pos_];
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                if (c == '\n') line_++;
                pos_++;
            } else if (c == '/' && pos_ + 1 < text_.size() && text_[pos_ + 1] == '/') {
                // 单行注释
                pos_ += 2;
                while (pos_ < text_.size() && text_[pos_] != '\n') pos_++;
            } else if (c == '/' && pos_ + 1 < text_.size() && text_[pos_ + 1] == '*') {
                // 多行注释
                pos_ += 2;
                while (pos_ + 1 < text_.size() && !(text_[pos_] == '*' && text_[pos_ + 1] == '/')) {
                    if (text_[pos_] == '\n') line_++;
                    pos_++;
                }
                if (pos_ + 1 < text_.size()) pos_ += 2;
            } else {
                break;
            }
        }
    }

    Token ReadString(char quote, int start_line) {
        pos_++;  // 跳过开头引号
        std::string result;
        while (pos_ < text_.size() && text_[pos_] != quote) {
            if (text_[pos_] == '\\' && pos_ + 1 < text_.size()) {
                pos_++;
                char esc = text_[pos_];
                switch (esc) {
                    case 'n': result += '\n'; break;
                    case 't': result += '\t'; break;
                    case 'r': result += '\r'; break;
                    case '\\': result += '\\'; break;
                    case '\'': result += '\''; break;
                    case '"': result += '"'; break;
                    case '0': result += '\0'; break;
                    default: result += esc; break;
                }
            } else {
                if (text_[pos_] == '\n') line_++;
                result += text_[pos_];
            }
            pos_++;
        }
        if (pos_ < text_.size()) pos_++;  // 跳过结尾引号
        return {Token::T_STRING, result, 0, start_line};
    }

    Token ReadNumber(int start_line) {
        size_t start = pos_;
        bool is_float = false;
        if (text_[pos_] == '0' && pos_ + 1 < text_.size() && (text_[pos_ + 1] == 'x' || text_[pos_ + 1] == 'X')) {
            pos_ += 2;
            while (pos_ < text_.size() && std::isxdigit(text_[pos_])) pos_++;
        } else {
            while (pos_ < text_.size() && std::isdigit(text_[pos_])) pos_++;
            if (pos_ < text_.size() && text_[pos_] == '.') {
                is_float = true;
                pos_++;
                while (pos_ < text_.size() && std::isdigit(text_[pos_])) pos_++;
            }
        }
        return {is_float ? Token::T_FLOAT : Token::T_INT, std::string(text_, start, pos_ - start), 0, start_line};
    }

    Token ReadIdent(int start_line) {
        size_t start = pos_;
        while (pos_ < text_.size() && (std::isalnum(text_[pos_]) || text_[pos_] == '_')) pos_++;
        std::string word(text_, start, pos_ - start);
        auto &kw = Keywords();
        auto it = kw.find(word);
        if (it != kw.end()) {
            return {Token::T_KEYWORD, word, it->second, start_line};
        }
        return {Token::T_IDENT, word, 0, start_line};
    }

    const std::string &text_;
    size_t pos_ = 0;
    int line_ = 1;
};

// ─── Parser ───

class Parser {
public:
    explicit Parser(const std::string &filename) : filename_(filename) {}

    void Parse(Lexer &lexer) {
        current_ = lexer.Next();
        while (current_.type != Token::T_EOF) {
            ParseStatement(lexer, "");
        }
    }

private:
    Token current_;
    std::string filename_;
    std::string package_;        // 当前 package
    std::string syntax_;         // "proto2" / "proto3"

    void Advance(Lexer &lexer) { current_ = lexer.Next(); }

    void Expect(Lexer &lexer, Token::Type type, const std::string &what) {
        if (current_.type != type) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected {}, got '{}'",
                                              current_.line, what, current_.value));
        }
        Advance(lexer);
    }

    void ExpectSymbol(Lexer &lexer, const std::string &sym) {
        if (current_.type != Token::T_SYMBOL || current_.value != sym) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected '{}', got '{}'",
                                              current_.line, sym, current_.value));
        }
        Advance(lexer);
    }

    // ─── 顶层语句 ───

    void ParseStatement(Lexer &lexer, const std::string &prefix) {
        if (current_.type == Token::T_KEYWORD) {
            switch (current_.keyword_id) {
                case 1:  // syntax
                    ParseSyntax(lexer);
                    return;
                case 2:  // package
                    ParsePackage(lexer);
                    return;
                case 3:  // import
                    ParseImport(lexer);
                    return;
                case 4:  // message
                    ParseMessage(lexer, prefix);
                    return;
                case 5:  // enum
                    ParseEnum(lexer, prefix);
                    return;
                case 12: // option
                    ParseOptionStatement(lexer);
                    return;
                default:
                    break;
            }
        }
        if (current_.type == Token::T_SYMBOL && current_.value == ";") {
            Advance(lexer);
            return;
        }
        ThrowFakeluaException(std::format("proto parse error at line {}: unexpected token '{}'",
                                          current_.line, current_.value));
    }

    void ParseSyntax(Lexer &lexer) {
        Advance(lexer);  // 跳过 'syntax'
        ExpectSymbol(lexer, "=");
        if (current_.type != Token::T_STRING) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected syntax string", current_.line));
        }
        syntax_ = current_.value;
        Advance(lexer);
        ExpectSymbol(lexer, ";");
    }

    void ParsePackage(Lexer &lexer) {
        Advance(lexer);  // 跳过 'package'
        package_ = ParseFullyQualifiedIdent(lexer);
        ExpectSymbol(lexer, ";");
    }

    void ParseImport(Lexer &lexer) {
        Advance(lexer);  // 跳过 'import'
        // 跳过 public/weak 修饰符
        if (current_.type == Token::T_KEYWORD && (current_.keyword_id == 16 || current_.keyword_id == 17)) {
            Advance(lexer);
        }
        if (current_.type == Token::T_STRING) Advance(lexer);  // 跳过文件名
        ExpectSymbol(lexer, ";");
        // import 不实际加载文件，由调用者按顺序 load 多个 .proto
    }

    void ParseOptionStatement(Lexer &lexer) {
        Advance(lexer);  // 跳过 'option'
        // 跳过 option 名称和 = 值
        if (current_.type == Token::T_IDENT) Advance(lexer);
        else if (current_.type == Token::T_SYMBOL && current_.value == "(") {
            Advance(lexer);
            int depth = 1;
            while (depth > 0 && current_.type != Token::T_EOF) {
                if (current_.value == "(") depth++;
                else if (current_.value == ")") depth--;
                Advance(lexer);
            }
        }
        if (current_.type == Token::T_SYMBOL && current_.value == "=") {
            Advance(lexer);
            // 跳过值
            if (current_.type == Token::T_IDENT || current_.type == Token::T_INT ||
                current_.type == Token::T_FLOAT || current_.type == Token::T_STRING) {
                Advance(lexer);
            }
        }
        ExpectSymbol(lexer, ";");
    }

    // ─── 完全限定名解析 ───

    std::string ParseFullyQualifiedIdent(Lexer &lexer) {
        std::string name = current_.value;
        Advance(lexer);
        while (current_.type == Token::T_SYMBOL && current_.value == ".") {
            Advance(lexer);
            name += ".";
            if (current_.type == Token::T_IDENT) {
                name += current_.value;
                Advance(lexer);
            }
        }
        return name;
    }

    // 将相对类型名解析为完全限定名
    std::string ResolveTypeName(const std::string &name, const std::string &prefix) {
        // 已经是完全限定名（以 package 开头或含点）
        if (!package_.empty() && name.find('.') != std::string::npos) return name;
        // 以点开头 = 绝对名
        if (!name.empty() && name[0] == '.') return name.substr(1);
        // 有 prefix（嵌套类型内）
        if (!prefix.empty()) {
            // 先在 prefix 下查找
            std::string candidate = prefix + "." + name;
            if (ProtobufState::Instance().FindMessage(candidate) ||
                ProtobufState::Instance().FindEnum(candidate)) {
                return candidate;
            }
        }
        // 在 package 下查找
        if (!package_.empty()) {
            std::string candidate = package_ + "." + name;
            if (ProtobufState::Instance().FindMessage(candidate) ||
                ProtobufState::Instance().FindEnum(candidate)) {
                return candidate;
            }
        }
        // 无法解析，返回原名（后续可能报错）
        return name;
    }

    // ─── Message ───

    void ParseMessage(Lexer &lexer, const std::string &prefix) {
        Advance(lexer);  // 跳过 'message'
        if (current_.type != Token::T_IDENT) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected message name", current_.line));
        }
        std::string msg_name = current_.value;
        Advance(lexer);

        std::string full_name = prefix.empty() ? msg_name : prefix + "." + msg_name;
        if (!package_.empty() && prefix.empty()) {
            full_name = package_ + "." + msg_name;
        }

        MessageDef def;
        def.name = full_name;

        ExpectSymbol(lexer, "{");
        std::string nested_prefix = full_name;

        while (!(current_.type == Token::T_SYMBOL && current_.value == "}")) {
            ParseMessageBody(lexer, def, nested_prefix);
        }
        ExpectSymbol(lexer, "}");

        // 可选的尾随分号
        if (current_.type == Token::T_SYMBOL && current_.value == ";") Advance(lexer);

        ProtobufState::Instance().RegisterMessage(std::move(def));
    }

    void ParseMessageBody(Lexer &lexer, MessageDef &def, const std::string &prefix) {
        if (current_.type == Token::T_KEYWORD) {
            switch (current_.keyword_id) {
                case 4:  // 嵌套 message
                    ParseMessage(lexer, prefix);
                    return;
                case 5:  // 嵌套 enum
                    ParseEnum(lexer, prefix);
                    return;
                case 6:  // oneof
                    ParseOneof(lexer, def, prefix);
                    return;
                case 7:  // map
                    ParseMapField(lexer, def, prefix);
                    return;
                case 8:  // repeated
                    Advance(lexer);  // 跳过 'repeated'，指向类型名
                    ParseField(lexer, def, prefix, true, false);
                    return;
                case 9:  // optional
                    Advance(lexer);  // 跳过 'optional'，指向类型名
                    ParseField(lexer, def, prefix, false, true);
                    return;
                case 10: // reserved
                    ParseReserved(lexer);
                    return;
                case 11: // extensions
                    ParseExtensions(lexer);
                    return;
                case 12: // option
                    ParseOptionStatement(lexer);
                    return;
                default:
                    break;
            }
        }
        if (current_.type == Token::T_SYMBOL && current_.value == ";") {
            Advance(lexer);
            return;
        }
        // 普通字段（无 label）
        ParseField(lexer, def, prefix, false, false);
    }

    // ─── Field ───

    void ParseField(Lexer &lexer, MessageDef &def, const std::string &prefix, bool repeated, bool is_optional) {
        std::string type_name;
        FieldType type = TYPE_INT32;

        if (current_.type == Token::T_IDENT) {
            type_name = current_.value;
            Advance(lexer);
            if (IsScalarType(type_name)) {
                type = ParseScalarType(type_name);
                type_name.clear();
            } else if (ProtobufState::Instance().FindEnum(type_name)) {
                // 已注册的 enum 类型
                type = TYPE_ENUM;
                type_name = ResolveTypeName(type_name, prefix);
            } else {
                // message 类型
                type = TYPE_MESSAGE;
                type_name = ResolveTypeName(type_name, prefix);
            }
        } else {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected field type", current_.line));
        }

        if (current_.type != Token::T_IDENT) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected field name", current_.line));
        }
        std::string field_name = current_.value;
        Advance(lexer);

        ExpectSymbol(lexer, "=");
        if (current_.type != Token::T_INT) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected field number", current_.line));
        }
        int field_number = std::stoi(current_.value);
        Advance(lexer);

        // 跳过 inline options [...]
        if (current_.type == Token::T_SYMBOL && current_.value == "[") {
            Advance(lexer);
            while (!(current_.type == Token::T_SYMBOL && current_.value == "]")) {
                if (current_.type == Token::T_EOF) break;
                Advance(lexer);
            }
            if (current_.type == Token::T_SYMBOL && current_.value == "]") Advance(lexer);
        }

        ExpectSymbol(lexer, ";");

        FieldDef field;
        field.name = field_name;
        field.number = field_number;
        field.type = type;
        field.type_name = type_name;
        field.repeated = repeated;
        field.optional = is_optional;
        def.fields.push_back(std::move(field));
    }

    // ─── Map field ───

    void ParseMapField(Lexer &lexer, MessageDef &def, const std::string &prefix) {
        Advance(lexer);  // 跳过 'map'
        ExpectSymbol(lexer, "<");
        if (current_.type != Token::T_IDENT) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected map key type", current_.line));
        }
        std::string key_type_name = current_.value;
        Advance(lexer);
        ExpectSymbol(lexer, ",");
        if (current_.type != Token::T_IDENT) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected map value type", current_.line));
        }
        std::string val_type_name = current_.value;
        Advance(lexer);
        ExpectSymbol(lexer, ">");

        if (current_.type != Token::T_IDENT) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected map field name", current_.line));
        }
        std::string field_name = current_.value;
        Advance(lexer);

        ExpectSymbol(lexer, "=");
        if (current_.type != Token::T_INT) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected field number", current_.line));
        }
        int field_number = std::stoi(current_.value);
        Advance(lexer);

        // 跳过 inline options
        if (current_.type == Token::T_SYMBOL && current_.value == "[") {
            Advance(lexer);
            while (!(current_.type == Token::T_SYMBOL && current_.value == "]")) {
                if (current_.type == Token::T_EOF) break;
                Advance(lexer);
            }
            if (current_.type == Token::T_SYMBOL && current_.value == "]") Advance(lexer);
        }

        ExpectSymbol(lexer, ";");

        FieldDef field;
        field.name = field_name;
        field.number = field_number;
        field.type = TYPE_MESSAGE;  // map 编码为 message
        field.is_map = true;
        field.map_key_type = ParseScalarType(key_type_name);
        if (IsScalarType(val_type_name)) {
            field.map_value_type = ParseScalarType(val_type_name);
        } else {
            field.map_value_type = TYPE_MESSAGE;
            field.map_value_type_name = ResolveTypeName(val_type_name, prefix);
        }
        def.fields.push_back(std::move(field));
    }

    // ─── Oneof ───

    void ParseOneof(Lexer &lexer, MessageDef &def, const std::string &prefix) {
        Advance(lexer);  // 跳过 'oneof'
        if (current_.type != Token::T_IDENT) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected oneof name", current_.line));
        }
        std::string oneof_name = current_.value;
        Advance(lexer);

        ExpectSymbol(lexer, "{");
        int oneof_index = static_cast<int>(def.fields.size());  // 简单编号

        while (!(current_.type == Token::T_SYMBOL && current_.value == "}")) {
            if (current_.type == Token::T_KEYWORD && current_.keyword_id == 12) {
                ParseOptionStatement(lexer);
                continue;
            }
            if (current_.type == Token::T_SYMBOL && current_.value == ";") {
                Advance(lexer);
                continue;
            }
            // oneof 内字段
            ParseOneofField(lexer, def, prefix, oneof_index);
        }
        ExpectSymbol(lexer, "}");
    }

    void ParseOneofField(Lexer &lexer, MessageDef &def, const std::string &prefix, int oneof_index) {
        std::string type_name;
        FieldType type = TYPE_INT32;

        if (current_.type != Token::T_IDENT) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected field type", current_.line));
        }
        type_name = current_.value;
        Advance(lexer);
        if (IsScalarType(type_name)) {
            type = ParseScalarType(type_name);
            type_name.clear();
        } else {
            type = TYPE_MESSAGE;
            type_name = ResolveTypeName(type_name, prefix);
        }

        if (current_.type != Token::T_IDENT) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected field name", current_.line));
        }
        std::string field_name = current_.value;
        Advance(lexer);

        ExpectSymbol(lexer, "=");
        if (current_.type != Token::T_INT) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected field number", current_.line));
        }
        int field_number = std::stoi(current_.value);
        Advance(lexer);

        // 跳过 inline options
        if (current_.type == Token::T_SYMBOL && current_.value == "[") {
            Advance(lexer);
            while (!(current_.type == Token::T_SYMBOL && current_.value == "]")) {
                if (current_.type == Token::T_EOF) break;
                Advance(lexer);
            }
            if (current_.type == Token::T_SYMBOL && current_.value == "]") Advance(lexer);
        }

        ExpectSymbol(lexer, ";");

        FieldDef field;
        field.name = field_name;
        field.number = field_number;
        field.type = type;
        field.type_name = type_name;
        field.oneof_index = oneof_index;
        def.fields.push_back(std::move(field));
    }

    // ─── Enum ───

    void ParseEnum(Lexer &lexer, const std::string &prefix) {
        Advance(lexer);  // 跳过 'enum'
        if (current_.type != Token::T_IDENT) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected enum name", current_.line));
        }
        std::string enum_name = current_.value;
        Advance(lexer);

        std::string full_name = prefix.empty() ? enum_name : prefix + "." + enum_name;
        if (!package_.empty() && prefix.empty()) {
            full_name = package_ + "." + enum_name;
        }

        EnumDef def;
        def.name = full_name;

        ExpectSymbol(lexer, "{");
        while (!(current_.type == Token::T_SYMBOL && current_.value == "}")) {
            if (current_.type == Token::T_KEYWORD && current_.keyword_id == 12) {
                ParseOptionStatement(lexer);
                continue;
            }
            if (current_.type == Token::T_SYMBOL && current_.value == ";") {
                Advance(lexer);
                continue;
            }
            ParseEnumField(lexer, def);
        }
        ExpectSymbol(lexer, "}");

        if (current_.type == Token::T_SYMBOL && current_.value == ";") Advance(lexer);

        ProtobufState::Instance().RegisterEnum(std::move(def));
    }

    void ParseEnumField(Lexer &lexer, EnumDef &def) {
        if (current_.type != Token::T_IDENT) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected enum value name", current_.line));
        }
        std::string name = current_.value;
        Advance(lexer);
        ExpectSymbol(lexer, "=");
        if (current_.type != Token::T_INT) {
            ThrowFakeluaException(std::format("proto parse error at line {}: expected enum number", current_.line));
        }
        int number = std::stoi(current_.value);
        Advance(lexer);

        // 跳过 inline options
        if (current_.type == Token::T_SYMBOL && current_.value == "[") {
            Advance(lexer);
            while (!(current_.type == Token::T_SYMBOL && current_.value == "]")) {
                if (current_.type == Token::T_EOF) break;
                Advance(lexer);
            }
            if (current_.type == Token::T_SYMBOL && current_.value == "]") Advance(lexer);
        }

        ExpectSymbol(lexer, ";");
        def.values.emplace_back(name, number);
    }

    // ─── Reserved / Extensions ───

    void ParseReserved(Lexer &lexer) {
        Advance(lexer);  // 跳过 'reserved'
        // 跳过 reserved 内容（数字范围或名称列表）
        while (!(current_.type == Token::T_SYMBOL && current_.value == ";")) {
            if (current_.type == Token::T_EOF) break;
            Advance(lexer);
        }
        ExpectSymbol(lexer, ";");
    }

    void ParseExtensions(Lexer &lexer) {
        Advance(lexer);  // 跳过 'extensions'
        while (!(current_.type == Token::T_SYMBOL && current_.value == ";")) {
            if (current_.type == Token::T_EOF) break;
            Advance(lexer);
        }
        ExpectSymbol(lexer, ";");
    }
};

// ─── 公开接口 ───

std::string ParseProto(const std::string &text, const std::string &filename) {
    try {
        Lexer lexer(text);
        Parser parser(filename);
        parser.Parse(lexer);
        ProtobufState::Instance().ResolveAll();  // 解析完成后修正 enum 引用
        return "";  // 成功
    } catch (const std::exception &e) {
        return e.what();
    }
}

}  // namespace fakelua::protobuf
