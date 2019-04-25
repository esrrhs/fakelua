#include "myflexer.h"
#include "fake-inc.h"
#include "fake.h"


// flex通过调用这个方法取得要解析的字符串的
int myflexer::LexerInput(char* buf, int max_size)
{
	if (m_pos == m_num)
	{
		return 0;
	}
	
	strncpy(buf, m_content.c_str() + m_pos, max_size - 1);
	buf[max_size - 1] = '\0'; 

	int len = strlen(buf);
	m_pos += len;

	FKLOG("LexerInput: %s\n", buf);
	
	return len; 
}

void myflexer::LexerOutput( const char* buf, int size )
{
}

//错误函数
void myflexer::LexerError(const char* msg)
{
	FKLOG("parse error : %s at line:%d near:%s\n", msg, lineno(), yytext);
	char buff[100];
	tsnprintf(buff, sizeof(buff)-1, "%s at line(%d) near(%s)", msg, lineno(), yytext);
	m_error = buff;
	m_errorline = lineno();
}

// 输入字符串
bool myflexer::inputstr(const char * str)
{
	m_filename = "string";
	m_content = str;
	m_num = m_content.size();
	m_pos = 0;
	return true;
}

// 输入文件
bool myflexer::inputfile(const char * filename)
{
	m_filename = filename;
	FILE * file = fopen(filename, "r");
	if (!file)
	{
		seterror(m_fk, efk_open_file_fail, filename, 0, "", "open %s fail", filename);
		return false;
	}

	int8_t readbuff[100];
	size_t len = 0;
	while (!feof(file))
	{
		len = fread((void*)readbuff, 1, sizeof(readbuff)-1, file);
		readbuff[len] = 0;
		m_content += (char *)readbuff;
	}

	fclose(file);

	if (m_content.empty())
	{
		seterror(m_fk, efk_open_file_empty, filename, 0, "", "open %s empty", filename);
		return false;
	}
	
	m_num = m_content.size();
	m_pos = 0;
	
	return true;
}

const char * myflexer::getfilename()
{
	return m_filename.c_str();
}

void myflexer::clear()
{
	m_num = 0;
	m_pos = 0;
	m_filename.clear();
	m_packagename.clear();
	m_content.clear();
	for (int i = 0; i < (int)m_nodelist.size(); i++)
	{
		syntree_node * p = m_nodelist[i];
		FKLOG("flexer free %p", p);
		p->~syntree_node();
		safe_fkfree(m_fk, p);
	}
	m_nodelist.clear();
	m_funclist.clear();
	m_constmap.clear();
	m_includelist.clear();
	m_struct_list.clear();
	m_error.clear();
	yylineno = 1;
	m_errorline = 0;
}

void * myflexer::malloc(size_t size, const char * name)
{
	void * p = safe_fkmalloc(m_fk, size, emt_flexer);
	FKLOG("flexer malloc %p %d %s", p, (int)size, name);
	m_nodelist.push_back((syntree_node*)p);
	return p;
}

void myflexer::add_func_desc(func_desc_node * p)
{
	FKLOG("add_func_desc %s %p", p->funcname.c_str(), p);
	String dump = p->dump(0);
	FKLOG("func %s dump %s", p->funcname.c_str(), dump.c_str());
	m_funclist.push_back(p);
}

func_desc_list & myflexer::get_func_list()
{
	return m_funclist;
}

bool myflexer::is_have_func(const String & funcname)
{
	for (int i = 0; i < (int)m_funclist.size(); i++)
	{
		func_desc_node * p = m_funclist[i];
		if (p->funcname == funcname)
		{
			return true;
		}
	}
	return false;
}

void myflexer::add_const_desc(const char * name, syntree_node * p)
{
	assert(p->gettype() == est_explicit_value);
	explicit_value_node * ev = dynamic_cast<explicit_value_node *>(p);
	assert(ev);
	FKLOG("add_const_desc %s = %s", name, ev->dump(0).c_str());
	m_constmap[name] = ev;
}

explicit_value_map & myflexer::get_const_map()
{
	return m_constmap;
}

void myflexer::add_include(const char * include_file)
{
	// 加入include list，等待解析完再统一挨个include
	m_includelist.push_back(include_file);
}

include_file_list & myflexer::get_include_list()
{
	return m_includelist;
}

void myflexer::add_struct_desc(const char * name, struct_desc_memlist_node * p)
{
	FKLOG("add_struct_desc %s\n%s", name, p->dump(0).c_str());
	m_struct_list[name] = p->memlist;
}

bool myflexer::is_have_struct(const String & name)
{
	return m_struct_list.find(name) != m_struct_list.end();
}

