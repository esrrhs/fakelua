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

#define MAX_FLEX_BUFFER_SIZE 1024*1024

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
private:
    fuck * m_fk;
    char m_content[MAX_FLEX_BUFFER_SIZE];
    int m_num;
    int m_pos;
}; 

