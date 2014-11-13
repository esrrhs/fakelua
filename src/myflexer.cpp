#include "myflexer.h"
#include "fakescript.h"
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
    printf("parse error : %s at line:%d near:%s\n", msg, lineno(), yytext);
    FKLOG("parse error : %s at line:%d near:%s\n", msg, lineno(), yytext);
}

// 输入文件
bool myflexer::inputfile(const char * filename)
{
    FILE * file = fopen(filename, "r");
    if (!file)
    {
        seterror(m_fk, efk_open_file_fail, "open %s fail", filename);
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

	if (m_content.empty())
    {
        seterror(m_fk, efk_open_file_empty, "open %s empty", filename);
        return false;
    }
    
	m_num = m_content.size();
    m_pos = 0;
    
    return true;
}

void myflexer::clear()
{
    m_num = 0;
    m_pos = 0;
    m_content.clear();
    for (int i = 0; i < (int)m_funclist.size(); i++)
    {
        func_desc_node * p = m_funclist[i];
        p->recycle();
    }
    m_funclist.clear();
}

void * myflexer::malloc(size_t size, const char * name)
{
    void * p = safe_fkmalloc(m_fk, size);
    FKLOG("flexer malloc %p %d %s", p, (int)size, name);
    return p;
}

void myflexer::free(void * p)
{
    safe_fkfree(m_fk, p);
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
