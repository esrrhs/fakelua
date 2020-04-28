#pragma once

#include "types.h"
#include "hashmap.h"

typedef std::vector<String> parsing_file_list;

struct const_parser_info {
    variant v;
    int lineno;
};

struct fake;

class parser {
public:
    parser(fake *fk);

    ~parser();

    void clear();

    void reset();

    bool parse(const char *filename);

    bool parsestr(const char *str);

    const char *get_parsing_file_list();

    bool is_parsing(const char *filename);

    void reg_const_define(const char *constname, const variant &v, int lineno);

    variant *get_const_define(const char *constname);

    int get_const_define_lineno(const char *constname);

private:
    bool parse_include(const String &srcname, const String &includename);

private:
    fake *m_fk;
    parsing_file_list m_parsing_file_list;
    String m_parsing_file_list_str;
    int m_parse_dep;
    fkhashmap<const char *, const_parser_info> m_shh;
};

