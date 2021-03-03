#include "parser.h"
#include "fakelua.h"

parser::parser() {
}

parser::~parser() {
}

int parser::parse(const std::string &file_name, const std::string &content) {


    auto ret = lex(file_name, content);
    if (ret != FAKELUA_OK) {
        return ret;
    }

    ret = parsing();
    if (ret != FAKELUA_OK) {
        return ret;
    }

    ret = compile();
    if (ret != FAKELUA_OK) {
        return ret;
    }

    return ret;
}

int parser::lex(const std::string &file_name, const std::string &content) {
    // replace comment with space
    auto content_no_comment = replace_multi_comment(content);
    content_no_comment = replace_comment(content_no_comment);

    auto[err, params] = split_string(file_name, content_no_comment);

    return FAKELUA_OK;
}

int parser::parsing() {
    // TODO
    return FAKELUA_OK;
}

int parser::compile() {
    // TODO
    return FAKELUA_OK;
}

std::tuple<err, std::vector<std::tuple<std::string, int, int>>> parser::split_string(const std::string &file_name,
                                                                                     const std::string &str) {
    std::vector<std::tuple<std::string, int, int>> ret;

    int curline = 1;
    int curcol = 1;

    std::string curstr;
    int curstr_col = 0;

    bool string_mod = false;
    char string_begin = 0;
    int string_begin_line = 1;
    int string_begin_col = 1;

    auto save = [&](bool force) {
        if (!curstr.empty() || force) {
            ret.push_back({curstr, curline, curstr_col});
            curstr.clear();
            curstr_col = 0;
        }
    };

    for (int i = 0; i < (int) str.size(); i++) {
        auto c = str[i];

        if (string_mod) {
            if (c == '\n') {
                curline++;
                curcol = 1;
            }

            if (curstr_col == 0) {
                curstr_col = curcol;
            }

            if (c == string_begin) {
                save(true);
                curcol++;
                string_mod = false;
                string_begin = 0;
                continue;
            }

            curstr += c;
            curcol++;

        } else {
            if (c == '\n') {
                save(false);
                curline++;
                curcol = 1;
                continue;
            }

            if (c == ' ' || c == '\t') {
                save(false);
                curcol++;
                continue;
            }

            if (curstr_col == 0) {
                curstr_col = curcol;
            }

            if (c == '\'' || c == '"') {
                save(false);
                string_mod = true;
                string_begin = c;
                string_begin_line = curline;
                string_begin_col = curcol;
                curcol++;
                continue;
            }

            curstr += c;
            curcol++;
        }
    }

    save(false);

    if (string_mod) {
        return {{FAKELUA_LEX_FAIL,
                 string_format("unfinished string in file %s:%d,%d", file_name.c_str(), string_begin_line,
                               string_begin_col)}, ret};
    }

    return {{}, ret};
}

std::string parser::change_comment_to_space(std::string str) {
    std::replace_if(str.begin(), str.end(), [](char x) { return x != '\n'; }, ' ');
    return str;
}

std::string parser::replace_multi_comment(std::string str) {
    std::regex e("--\\[\\[(?:[^\\]\\]]|)*\\]\\]");
    std::smatch sm;
    std::string ret = "";
    while (regex_search(str, sm, e)) {
        ret += sm.prefix();
        ret += change_comment_to_space(sm.str());
        str = sm.suffix();
    }
    ret += str;
    return ret;
}

std::string parser::replace_comment(std::string str) {
    std::regex e("--(?:[^\\n]|)*(\\n|$)");
    std::smatch sm;
    std::string ret = "";
    while (regex_search(str, sm, e)) {
        ret += sm.prefix();
        ret += change_comment_to_space(sm.str());
        str = sm.suffix();
    }
    ret += str;
    return ret;
}
