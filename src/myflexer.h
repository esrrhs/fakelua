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

typedef std::vector<String> include_file_list;

typedef std::vector<String> struct_mem_list;
typedef std::map<String, struct_mem_list> struct_list;

struct fake;

class myflexer : public yyFlexLexer {
public:
    // set parse command
    myflexer(fake *fk) : m_fk(fk) {
        clear();
    }

    virtual ~myflexer() {
        clear();
    }


    // flex通过调用这个方法取得要解析的字符串的
    virtual int LexerInput(char *buf, int max_size);

    virtual void LexerOutput(const char *buf, int size);


    //错误函数
    virtual void LexerError(const char *msg);

    // 在flex.cpp中实现
    int yylex(YYSTYPE *yylval, YYLTYPE *loc);

    fake *getfake() {
        return m_fk;
    }

    // 输入文件
    bool inputfile(const char *filename);

    const char *getfilename();

    // 输入字符串
    bool inputstr(const char *str);

    void clear();

    void *malloc(size_t size, const char *name);

    void add_func_desc(func_desc_node *p);

    func_desc_list &get_func_list();

    bool is_have_func(const String &funcname);

    void add_const_desc(const char *name, syntree_node *p);

    explicit_value_map &get_const_map();

    const char *get_error() const {
        return m_error.c_str();
    }

    int get_error_line() const {
        return m_errorline;
    }

    void set_package(const char *package_name) {
        m_packagename = package_name;
    }

    const char *get_package() {
        return m_packagename.c_str();
    }

    bool is_package() {
        return !m_packagename.empty();
    }

    void add_include(const char *include_file);

    include_file_list &get_include_list();

    void add_struct_desc(const char *name, struct_desc_memlist_node *p);

    bool is_have_struct(const String &name);

private:
    fake *m_fk;
    String m_filename;
    String m_packagename;
    String m_content;
    int m_num;
    int m_pos;
    syntree_node_list m_nodelist;
    func_desc_list m_funclist;
    explicit_value_map m_constmap;
    include_file_list m_includelist;
    struct_list m_struct_list;
    String m_error;
    int m_errorline;
}; 

