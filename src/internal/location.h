#pragma once

#include "types.h"

class location {
public:
    location(const std::string &file_name, int line_no, int col_no);

    virtual ~location();

    location(const char *file_name, int line_no, int col_no) = delete;

public:
    std::string to_string();

public:
    std::string_view m_file_name;
    int m_line_no = 0;
    int m_col_no = 0;
};
