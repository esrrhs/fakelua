#include "myflexer.h"
#include "fuckscript.h"
#include "fuck.h"

// flex通过调用这个方法取得要解析的字符串的
int myflexer::LexerInput(char* buf, int max_size)
{
    if (m_pos == m_num)
    {
        return 0;
    }
    
    strncpy(buf, m_content + m_pos, max_size - 1);
    buf[max_size - 1] = '\0'; 

    int len = strlen(buf);
    m_pos += len;

    FKLOG("LexerInput: %s\n", buf);
    
    return len; 
}

//错误函数
void myflexer::LexerError(const char* msg)
{
    printf("error:%s at line:%d near:%s\n", msg, lineno(), yy_c_buf_p);
    FKLOG("error:%s at line:%d near:%s\n", msg, lineno(), yy_c_buf_p);
}

// 输入文件
bool myflexer::inputfile(const char * filename)
{
    FILE * file = fopen(filename, "r");
    if (!file)
    {
        m_fk->m_efkerror = efk_openfilefail;
        return false;
    }

    int i = 0;

    while (!feof(file))
    {
        i += fread(m_content + i, 1, MAX_FLEX_BUFFER_SIZE, file);
    }

    if (i == 0)
    {
        m_fk->m_efkerror = efk_openfileempty;
        return false;
    }
    
    m_num = i;
    m_pos = 0;
    
    return true;
}

void myflexer::clear()
{
    m_num = 0;
    m_pos = 0;
    m_content[0] = 0;
}

void * myflexer::malloc(size_t size)
{
    return m_fk->m_fkmalloc(size);
}

void myflexer::free(void * p)
{
    return m_fk->m_fkfree(p);
}

void myflexer::add_func_desc(func_desc_node * p)
{
    FKLOG("add_func_desc %s %p", p->funcname.c_str(), p);
    String dump = p->dump(0);
    FKLOG("func %s dump %s", p->funcname.c_str(), dump.c_str());
}

