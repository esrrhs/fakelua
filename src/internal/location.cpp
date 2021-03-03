#include "location.h"

location::location(const std::string &file_name, int line_no, int col_no) :
        m_file_name(file_name), m_line_no(line_no), m_col_no(col_no) {
}

location::~location() {
}

std::string location::to_string() {
    return (!m_file_name.empty() && m_line_no != 0 && m_col_no != 0) ?
           string_format("%s:%d,%d", m_file_name.data(), m_line_no, m_col_no) : "";
}
