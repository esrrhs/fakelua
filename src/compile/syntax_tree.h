#pragma once

#include "util/common.h"
#include "bison/location.hh"

namespace fakelua {

// syntax tree type
enum syntax_tree_type {
    syntax_tree_type_none = 0,
    syntax_tree_type_block,
    syntax_tree_type_label,
    syntax_tree_type_return,
    syntax_tree_type_assign,
    syntax_tree_type_varlist,
    syntax_tree_type_explist,
};

// syntax tree location type
typedef yy::location syntax_tree_location;

// syntax tree interface
class syntax_tree_interface {
public:
    syntax_tree_interface(const syntax_tree_location &loc) : loc_(loc) {}

    virtual ~syntax_tree_interface() {}

    // get syntax tree type
    virtual syntax_tree_type type() const = 0;

    // dump syntax tree to string
    virtual std::string dump(int tab = 0) const = 0;

protected:
    // generate tab string
    std::string gen_tab(int tab) const {
        std::string str;
        for (int i = 0; i < tab; ++i) {
            str += "  ";
        }
        return str;
    }

    // generate location string
    std::string loc_str() const {
        // maybe the loc_'s filename ptr is invalid now, so ignore it
        std::string str;
        str += std::to_string(loc_.begin.line);
        str += ":";
        str += std::to_string(loc_.begin.column);
        return str;
    }

private:
    // syntax tree location
    syntax_tree_location loc_;
};

// syntax tree shared pointer
typedef std::shared_ptr<syntax_tree_interface> syntax_tree_interface_ptr;

// block
class syntax_tree_block : public syntax_tree_interface {
public:
    syntax_tree_block(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_block() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_block; }

    virtual std::string dump(int tab = 0) const override;

public:
    void add_stmt(const syntax_tree_interface_ptr &stmt) {
        stmts_.push_back(stmt);
    }

private:
    std::vector<syntax_tree_interface_ptr> stmts_;
};

// label
class syntax_tree_label : public syntax_tree_interface {
public:
    syntax_tree_label(const std::string &name, const syntax_tree_location &loc) : syntax_tree_interface(loc),
                                                                                  name_(name) {}

    virtual ~syntax_tree_label() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_label; }

    virtual std::string dump(int tab = 0) const override;

private:
    std::string name_;
};

// return
class syntax_tree_return : public syntax_tree_interface {
public:
    syntax_tree_return(const syntax_tree_interface_ptr &explist, const syntax_tree_location &loc)
            : syntax_tree_interface(loc), explist_(explist) {}

    virtual ~syntax_tree_return() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_return; }

    virtual std::string dump(int tab = 0) const override;

private:
    syntax_tree_interface_ptr explist_;
};

// assign
class syntax_tree_assign : public syntax_tree_interface {
public:
    syntax_tree_assign(const syntax_tree_interface_ptr &varlist, const syntax_tree_interface_ptr &explist,
                       const syntax_tree_location &loc) : syntax_tree_interface(loc), varlist_(varlist),
                                                          explist_(explist) {}

    virtual ~syntax_tree_assign() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_assign; }

    virtual std::string dump(int tab = 0) const override;

private:
    syntax_tree_interface_ptr varlist_;
    syntax_tree_interface_ptr explist_;
};

// varlist
class syntax_tree_varlist : public syntax_tree_interface {
public:
    syntax_tree_varlist(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_varlist() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_varlist; }

    virtual std::string dump(int tab = 0) const override;

    void add_var(const syntax_tree_interface_ptr &var) {
        vars_.push_back(var);
    }

private:
    std::vector<syntax_tree_interface_ptr> vars_;
};

// explist
class syntax_tree_explist : public syntax_tree_interface {
public:
    syntax_tree_explist(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_explist() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_explist; }

    virtual std::string dump(int tab = 0) const override;

    void add_exp(const syntax_tree_interface_ptr &exp) {
        exps_.push_back(exp);
    }

private:
    std::vector<syntax_tree_interface_ptr> exps_;
};


}
