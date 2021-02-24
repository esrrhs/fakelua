#include "location.h"

std::string location::to_string() {
    return (!m_file_name.empty() && m_line_no != 0 && m_col_no != 0) ?
           string_format("%s:%d,%d", m_file_name.data(), m_line_no, m_col_no) : "";
}

location::~location() {
}
