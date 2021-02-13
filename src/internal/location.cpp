#include "fakelua.h"
#include "location.h"

const char *location::to_string() {
    if (m_format_str.empty() && !m_file_name.empty() && m_line_no != 0 && m_col_no != 0) {
        m_format_str = string_format("%s:%d,%d", m_file_name.c_str(), m_line_no, m_col_no);
    }
    return m_format_str.c_str();
}
