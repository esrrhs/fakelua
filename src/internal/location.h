#pragma once

#include "types.h"

class location {
public:
    location() {}

    location(const std::string &file_name, int line_no, int col_no) : m_file_name(file_name), m_line_no(line_no),
                                                                      m_col_no(col_no) {}

    const char *to_string();

public:
    std::string m_file_name = "";
    int m_line_no = 0;
    int m_col_no = 0;
    std::string m_format_str = "";
};
