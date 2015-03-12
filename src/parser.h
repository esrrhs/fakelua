#pragma once

#include "types.h"

typedef std::vector<String> parsing_file_list;

struct fake;
class parser
{
public:
    parser(fake * fk);
	~parser();

	void clear();

    bool parse(const char * filename);
    bool parsestr(const char * str);
    
    const char * get_parsing_file_list();

    bool is_parsing(const char * filename);

private:
    bool parse_include(const String & srcname, const String & includename);

private:
	fake * m_fk;
	parsing_file_list m_parsing_file_list;
	String m_parsing_file_list_str;
	int m_parse_dep;
};

