#pragma once

#include "util/common.h"
#include "bison/location.hh"

enum syntax_tree_type {
    syntax_tree_type_none = 0,
    syntax_tree_type_block,
    syntax_tree_type_label,
};

typedef yy::location syntax_tree_location;

class syntax_tree_interface {
public:
    syntax_tree_interface(const syntax_tree_location &loc) : loc_(loc) {}

    virtual ~syntax_tree_interface() {}

    virtual syntax_tree_type type() const = 0;

    virtual std::string dump(int tab = 0) const = 0;

protected:
    std::string gen_tab(int tab) const {
        std::string str;
        for (int i = 0; i < tab; ++i) {
            str += " ";
        }
        return str;
    }

    std::string loc_str() const {
        std::stringstream ss;
        ss << loc_;
        return ss.str();
    }

private:
    syntax_tree_location loc_;
};

typedef std::shared_ptr<syntax_tree_interface> syntax_tree_interface_ptr;

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
