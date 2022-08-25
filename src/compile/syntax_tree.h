#pragma once

#include "util/common.h"
#include "bison/location.hh"

// syntax tree type
enum syntax_tree_type {
    syntax_tree_type_none = 0,
    syntax_tree_type_block,
    syntax_tree_type_label,
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
