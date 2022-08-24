#pragma once

#include "util/common.h"

enum syntax_tree_type {
    syntax_tree_type_none = 0,
    syntax_tree_type_block,
    syntax_tree_type_label,
};

class syntax_tree_interface {
public:
    syntax_tree_interface() {}

    virtual ~syntax_tree_interface() {}

    virtual syntax_tree_type type() const = 0;
};

class syntax_tree_block : public syntax_tree_interface {
public:
    syntax_tree_block() {}

    virtual ~syntax_tree_block() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_block; }

private:
    std::vector<syntax_tree_interface *> statements_;
};

class syntax_tree_label : public syntax_tree_interface {
public:
    syntax_tree_label() {}

    virtual ~syntax_tree_label() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_label; }

private:
    std::string name_;
};
