#include "parser.h"
#include "fake.h"

parser::parser(fake *fk) : m_fk(fk), m_parse_dep(0), m_shh(fk) {
}

parser::~parser() {
}

void parser::reset() {
    m_parsing_file_list.clear();
    m_parsing_file_list_str.clear();
    m_parse_dep = 0;
    // m_shh永久存在
}

void parser::clear() {
    reset();
    m_shh.clear();
}

bool parser::parsestr(const char *str) {
    fake *fk = m_fk;

    FKLOG("parsestr %p %s", fk, str);

    // 清空错误
    fk->clearerr();

    // 输入
    myflexer mf(fk);
    mf.clear();
    bool b = mf.inputstr(str);
    if (!b) {
        FKERR("parse inputstr %s fail", fk, str);
        return false;
    }

    // 进行语法解析
    int ret = yyparse((void *) &mf);
    if (ret != 0) {
        FKERR("parse yyparse %s fail ret %d", fk, str, ret);
        seterror(fk, efk_parse_file_fail, "", mf.get_error_line(), "", "parse string fail for reason : %s",
                 mf.get_error());
        return false;
    }

    FKLOG("parse yyparse %p %s OK", fk, str);

    // 解析前置文件
    for (int i = 0; i < (int) mf.get_include_list().size(); i++) {
        String &name = mf.get_include_list()[i];
        if (!parse_include("", name)) {
            FKERR("%s parse_include %s fail", "", name.c_str());
            return false;
        }
    }

    // 编译
    FKLOG("parse compile %p %s", fk, str);
    compiler mc(fk, &mf);
    mc.clear();
    b = mc.compile();
    if (!b) {
        FKERR("parse %s compile %s fail", fk, str);
        return false;
    }

    // jit
    fk->as.clear();
    assembler &as = fk->as;
    b = as.compile(&fk->bin);
    if (!b) {
        FKERR("parse %s jit %s fail", fk, str);
        return false;
    }

    FKLOG("parse %p %s OK", fk, str);

    return true;
}

bool parser::parse(const char *filename) {
    fake *fk = m_fk;

    if (!filename) {
        FKERR("parse filename is empty");
        seterror(fk, efk_parse_file_fail, "", 0, "", "parse filename is empty");
        return false;
    }

    FKLOG("parse %p %s", fk, filename);

    m_parse_dep++;

    // 检查深度
    if (m_parse_dep >= m_fk->cfg.include_deps) {
        FKERR("parse %s file too deep %d", filename, m_parse_dep);
        seterror(fk, efk_parse_file_fail, filename, 0, "", "parse %s file too deep %d", filename, m_parse_dep);
        return false;
    }

    // 检查当前文件是否在解析中
    if (is_parsing(filename)) {
        FKERR("parse open %s fail", fk, filename);
        seterror(fk, efk_parse_file_fail, filename, 0, "", "already parsing %s file...include list \n%s", filename,
                 get_parsing_file_list());
        return false;
    }

    // 加入
    m_parsing_file_list.push_back(filename);

    // 清空错误
    fk->clearerr();

    // 输入源文件
    FKLOG("parse inputfile %p %s", fk, filename);
    myflexer mf(fk);
    mf.clear();
    bool b = mf.inputfile(filename);
    if (!b) {
        FKERR("parse open %s fail", fk, filename);
        return false;
    }

    // 进行语法解析
    int ret = yyparse((void *) &mf);
    if (ret != 0) {
        FKERR("parse yyparse %s fail ret %d", fk, filename, ret);
        seterror(fk, efk_parse_file_fail, filename, mf.get_error_line(), "", "parse %s file fail for reason : %s",
                 filename, mf.get_error());
        return false;
    }

    FKLOG("parse yyparse %p %s OK", fk, filename);

    // 解析前置文件
    for (int i = 0; i < (int) mf.get_include_list().size(); i++) {
        String &name = mf.get_include_list()[i];
        if (!parse_include(filename, name)) {
            FKERR("%s parse_include %s fail", filename, name.c_str());
            return false;
        }
    }

    // 编译
    FKLOG("parse compile %p %s", fk, filename);
    compiler mc(fk, &mf);
    mc.clear();
    b = mc.compile();
    if (!b) {
        FKERR("parse %s compile %s fail", fk, filename);
        return false;
    }

    // jit
    fk->as.clear();
    assembler &as = fk->as;
    b = as.compile(&fk->bin);
    if (!b) {
        FKERR("fkparse %s jit %s fail", fk, filename);
        return false;
    }

    // 弹出
    assert(m_parsing_file_list.back() == (String) filename);
    m_parsing_file_list.pop_back();

    FKLOG("parse %p %s OK", fk, filename);

    return true;
}

const char *parser::get_parsing_file_list() {
    m_parsing_file_list_str.clear();
    for (int i = 0; i < (int) m_parsing_file_list.size(); i++) {
        m_parsing_file_list_str += m_parsing_file_list[i];
        m_parsing_file_list_str += "\n";
    }
    return m_parsing_file_list_str.c_str();
}

bool parser::is_parsing(const char *filename) {
    bool ret = false;
    for (int i = 0; i < (int) m_parsing_file_list.size(); i++) {
        if (m_parsing_file_list[i] == filename) {
            return true;
        }
    }
    return ret;
}

bool parser::parse_include(const String &srcname, const String &includename) {
    // 拼include的名字
    String dir = srcname;
    std::replace(dir.begin(), dir.end(), '\\', '/');
    int pos = dir.find_last_of('/');
    if (pos == -1) {
        dir = "";
    } else {
        dir = dir.substr(0, pos + 1);
    }
    dir += includename;

    // 解析
    if (!parse(dir.c_str())) {
        FKERR("parse_include %s fail", dir.c_str());
        return false;
    }
    return true;
}

void parser::reg_const_define(const char *constname, const variant &v, int lineno) {
    const_parser_info *p = m_shh.get(constname);
    if (UNLIKE(p != 0)) {
        p->v = v;
        p->lineno = lineno;
        return;
    }

    const_parser_info tmp;
    tmp.v = v;
    tmp.lineno = lineno;

    m_shh.add(constname, tmp);
}


variant *parser::get_const_define(const char *constname) {
    const_parser_info *p = m_shh.get(constname);
    if (LIKE(p != 0)) {
        return &p->v;
    }
    return 0;
}

int parser::get_const_define_lineno(const char *constname) {
    const_parser_info *p = m_shh.get(constname);
    if (LIKE(p != 0)) {
        return p->lineno;
    }
    return 0;
}
