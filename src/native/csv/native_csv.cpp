#include "native/csv/native_csv.h"
#include "native/native_common.h"
#include "native/table/native_table.h"
#include "var/var_table.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace fakelua::csv {

// ── CSV Parser ──

struct CsvParser {
    const uint8_t *data;
    size_t len;
    size_t pos = 0;
    char separator;

    CsvParser(const uint8_t *d, size_t l, char sep = ',') : data(d), len(l), separator(sep) {}

    // Parse entire CSV into a table of rows.
    // Each row is a table of fields (1-indexed).
    void parse(std::vector<std::vector<std::string>> &rows) {
        while (pos <= len) {
            // Skip BOM at start
            if (rows.empty() && pos + 2 < len && data[pos] == 0xEF && data[pos+1] == 0xBB && data[pos+2] == 0xBF) {
                pos += 3;
            }
            // Skip empty lines (just \n or \r\n)
            if (skip_empty_lines()) continue;
            if (pos >= len) break;
            parse_row(rows);
        }
    }

private:
    // Skip lines that are empty except for optional CR/LF. Leading spaces on
    // a real row are field content (RFC 4180) and must not be stripped.
    bool skip_empty_lines() {
        size_t start = pos;
        while (pos < len) {
            uint8_t c = data[pos];
            if (c == ' ' || c == '\t') {
                ++pos;
                continue;
            }
            if (c == '\n') {
                ++pos;
                return true;
            }
            if (c == '\r') {
                ++pos;
                if (pos < len && data[pos] == '\n') ++pos;
                return true;
            }
            pos = start;
            return false;
        }
        if (pos > start) {
            pos = start;
        }
        return false;
    }

    void parse_row(std::vector<std::vector<std::string>> &rows) {
        std::vector<std::string> row;
        while (true) {
            std::string field;
            parse_field(field);
            row.push_back(std::move(field));
            // After field: expect separator, newline, or end
            if (pos >= len) break;
            uint8_t c = data[pos];
            if (c == separator) {
                ++pos;
                continue;
            }
            if (c == '\n') {
                ++pos;
                break;
            }
            if (c == '\r') {
                ++pos;
                if (pos < len && data[pos] == '\n') ++pos;
                break;
            }
            // Unknown char in field — shouldn't happen if parse_field is correct
            break;
        }
        rows.push_back(std::move(row));
    }

    void parse_field(std::string &field) {
        if (pos >= len) return;
        if (data[pos] == '"') {
            parse_quoted_field(field);
        } else {
            parse_unquoted_field(field);
        }
    }

    void parse_quoted_field(std::string &field) {
        ++pos; // skip opening quote
        while (pos < len) {
            uint8_t c = data[pos++];
            if (c == '"') {
                // Could be end of field, or escaped quote
                if (pos < len && data[pos] == '"') {
                    field.push_back('"');
                    ++pos;
                } else {
                    // End of quoted field — only whitespace is allowed until separator/newline/eof
                    while (pos < len) {
                        uint8_t nc = data[pos];
                        if (nc == separator || nc == '\n' || nc == '\r') break;
                        if (nc == ' ' || nc == '\t') {
                            ++pos;
                            continue;
                        }
                        ThrowFakeluaException("CSV parse error: unexpected character after quoted field");
                    }
                    return;
                }
            } else {
                field.push_back(static_cast<char>(c));
            }
        }
        ThrowFakeluaException("CSV parse error: unterminated quoted field");
    }

    void parse_unquoted_field(std::string &field) {
        while (pos < len) {
            uint8_t c = data[pos];
            if (c == separator || c == '\n' || c == '\r') break;
            field.push_back(static_cast<char>(c));
            ++pos;
        }
    }
};

// ── Convert string field to Lua value (number if possible, else string) ──

static CVar field_to_lua(State *s, const std::string &str) {
    if (str.empty()) return inter::NativeToFakeluaString(s, str);

    // Keep leading zeros as strings ("001"); reject inf/nan.
    const char *start = str.c_str();
    bool leading_zero_int = str.size() > 1 && str[0] == '0' && str[1] >= '0' && str[1] <= '9';
    if (!leading_zero_int) {
        char *end = nullptr;
        long long ival = strtoll(start, &end, 10);
        if (end && *end == '\0' && end != start) {
            return inter::NativeToFakeluaLonglong(s, ival);
        }
        double dval = strtod(start, &end);
        if (end && *end == '\0' && end != start && std::isfinite(dval)) {
            return inter::NativeToFakeluaDouble(s, dval);
        }
    }
    return inter::NativeToFakeluaString(s, str);
}

// ── CSV Encoder ──

static void encode_field(std::string &out, const std::string &str, char separator) {
    // Check if we need quoting
    bool need_quote = false;
    for (char c : str) {
        if (c == '"' || c == separator || c == '\n' || c == '\r') {
            need_quote = true;
            break;
        }
    }
    if (!need_quote) {
        out += str;
        return;
    }
    out += '"';
    for (char c : str) {
        if (c == '"') out += "\"\"";
        else out += c;
    }
    out += '"';
}

static std::string encode_row(const std::vector<std::string> &row, char separator) {
    std::string out;
    for (size_t i = 0; i < row.size(); i++) {
        if (i > 0) out += separator;
        encode_field(out, row[i], separator);
    }
    return out;
}

// ── Helper: convert any CVar to string for encoding ──

static std::string cvar_to_string(CVar v) {
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

// ── Lua Bindings ──

// csv.decode(str, sep?) → table of rows
static CVar csv_decode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "csv.decode", "csv string expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);
    std::string str = inter::FakeluaToNativeString(s, a0);

    char separator = ',';
    if (n >= 2) {
        CVar a1 = inter::GetNativeArg(s, args, n, 1);
        std::string sep_str = inter::FakeluaToNativeString(s, a1);
        if (!sep_str.empty()) separator = sep_str[0];
    }

    CsvParser parser(reinterpret_cast<const uint8_t*>(str.data()), str.size(), separator);
    std::vector<std::vector<std::string>> rows;
    parser.parse(rows);

    // Convert to Lua table of rows
    CVar tbl = table::TableHelper::CreateTable(s);
    for (size_t i = 0; i < rows.size(); i++) {
        CVar row_tbl = table::TableHelper::CreateTable(s);
        for (size_t j = 0; j < rows[i].size(); j++) {
            CVar val = field_to_lua(s, rows[i][j]);
            table::TableHelper::SetTableInt(s, row_tbl, static_cast<int64_t>(j + 1), val);
        }
        table::TableHelper::SetTableInt(s, tbl, static_cast<int64_t>(i + 1), row_tbl);
    }
    return tbl;
}

// csv.encode(rows, sep?) → CSV string
static CVar csv_encode(State *s, CVar *args, int n) {
    if (n < 1) ThrowBadArgument(1, "csv.encode", "table expected");
    CVar a0 = inter::GetNativeArg(s, args, n, 0);

    char separator = ',';
    if (n >= 2) {
        CVar a1 = inter::GetNativeArg(s, args, n, 1);
        std::string sep_str = inter::FakeluaToNativeString(s, a1);
        if (!sep_str.empty()) separator = sep_str[0];
    }

    std::string out;

    if (a0.type_ == static_cast<int>(VarType::Table)) {
        auto *t = a0.data_.t;
        if (t) {
            struct RowEntry { int64_t key; CVar val; };
            std::vector<RowEntry> row_entries;
            table::TableHelper::ForEachKV(a0, [&](CVar k, CVar val) {
                if (k.type_ == static_cast<int>(VarType::Int)) {
                    row_entries.push_back({k.data_.i, val});
                }
            });

            std::sort(row_entries.begin(), row_entries.end(),
                      [](const RowEntry &a, const RowEntry &b) { return a.key < b.key; });

            for (size_t i = 0; i < row_entries.size(); i++) {
                if (i > 0) out += '\n';
                const CVar &row_val = row_entries[i].val;
                if (row_val.type_ == static_cast<int>(VarType::Table)) {
                    std::vector<std::string> fields;
                    struct FieldEntry { int64_t key; CVar val; };
                    std::vector<FieldEntry> field_entries;
                    table::TableHelper::ForEachKV(row_val, [&](CVar k, CVar val) {
                        if (k.type_ == static_cast<int>(VarType::Int)) {
                            field_entries.push_back({k.data_.i, val});
                        }
                    });
                    std::sort(field_entries.begin(), field_entries.end(),
                              [](const FieldEntry &a, const FieldEntry &b) { return a.key < b.key; });
                    for (auto &fe : field_entries) {
                        fields.push_back(cvar_to_string(fe.val));
                    }
                    out += encode_row(fields, separator);
                } else {
                    out += cvar_to_string(row_val);
                }
            }
        }
    }

    return inter::NativeToFakeluaString(s, out);
}

void RegisterCsvLibraryApi(State *s) {
    if (!s) return;
    RegisterNativeFunction(s, "csv.decode", 1, true, csv_decode);
    RegisterNativeFunction(s, "csv.encode", 1, true, csv_encode);
}

}  // namespace fakelua::csv
