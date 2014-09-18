#pragma once

#include <string>
#include <list>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <typeinfo>
#include <stdio.h>
#include "types.h"
#undef yyFlexLexer
#include <FlexLexer.h> 
#include "semantic.h"

class fuck;
class myflexer : public yyFlexLexer
{
public:
    // set parse command
    myflexer(fuck * fk) : m_fk(fk)
    {
        clear();
    }
    virtual ~myflexer() {}
    
    // flex通过调用这个方法取得要解析的字符串的
    virtual int LexerInput(char* buf, int max_size);
    
    //错误函数
    virtual void LexerError(const char* msg);

    // 在flex.cpp中实现
    int yylex(YYSTYPE * yylval);

    // 输入文件
    bool inputfile(const char * filename);

    void clear();

    void * malloc(size_t size);
    void free(void * p);

    void add_func_desc(func_desc_node * p);
    func_desc_list & get_func_list();
private:
    fuck * m_fk;
    String m_content;
    int m_num;
    int m_pos;
    func_desc_list m_funclist;
}; 

