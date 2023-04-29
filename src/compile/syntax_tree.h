#pragma once

#include "bison/location.hh"
#include "util/common.h"

namespace fakelua {

// syntax tree type
enum syntax_tree_type {
    syntax_tree_type_none = 0,
    syntax_tree_type_empty,
    syntax_tree_type_block,
    syntax_tree_type_label,
    syntax_tree_type_return,
    syntax_tree_type_assign,
    syntax_tree_type_varlist,
    syntax_tree_type_explist,
    syntax_tree_type_var,
    syntax_tree_type_functioncall,
    syntax_tree_type_tableconstructor,
    syntax_tree_type_fieldlist,
    syntax_tree_type_field,
    syntax_tree_type_break,
    syntax_tree_type_goto,
    syntax_tree_type_while,
    syntax_tree_type_repeat,
    syntax_tree_type_if,
    syntax_tree_type_elseiflist,
    syntax_tree_type_for_loop,
    syntax_tree_type_for_in,
    syntax_tree_type_namelist,
    syntax_tree_type_function,
    syntax_tree_type_funcnamelist,
    syntax_tree_type_funcname,
    syntax_tree_type_funcbody,
    syntax_tree_type_functiondef,
    syntax_tree_type_parlist,
    syntax_tree_type_local_function,
    syntax_tree_type_local_var,
    syntax_tree_type_exp,
    syntax_tree_type_binop,
    syntax_tree_type_unop,
    syntax_tree_type_args,
    syntax_tree_type_prefixexp,
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

// empty
class syntax_tree_empty : public syntax_tree_interface {
public:
    syntax_tree_empty(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    syntax_tree_type type() const { return syntax_tree_type_empty; }

    std::string dump(int tab = 0) const;
};

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
    syntax_tree_label(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_label() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_label; }

    virtual std::string dump(int tab = 0) const override;

    void set_name(const std::string &name) {
        name_ = name;
    }

private:
    std::string name_;
};

// return
class syntax_tree_return : public syntax_tree_interface {
public:
    syntax_tree_return(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_return() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_return; }

    virtual std::string dump(int tab = 0) const override;

    void set_explist(const syntax_tree_interface_ptr &explist) {
        explist_ = explist;
    }

private:
    syntax_tree_interface_ptr explist_;
};

// assign
class syntax_tree_assign : public syntax_tree_interface {
public:
    syntax_tree_assign(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_assign() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_assign; }

    virtual std::string dump(int tab = 0) const override;

    void set_varlist(const syntax_tree_interface_ptr &varlist) {
        varlist_ = varlist;
    }

    void set_explist(const syntax_tree_interface_ptr &explist) {
        explist_ = explist;
    }

private:
    syntax_tree_interface_ptr varlist_;
    syntax_tree_interface_ptr explist_;
};

// var list
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

// exp list
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

// var
class syntax_tree_var : public syntax_tree_interface {
public:
    syntax_tree_var(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_var() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_var; }

    virtual std::string dump(int tab = 0) const override;

    void set_name(const std::string &name) {
        name_ = name;
    }

    void set_prefixexp(const syntax_tree_interface_ptr &prefixexp) {
        prefixexp_ = prefixexp;
    }

    void set_exp(const syntax_tree_interface_ptr &exp) {
        exp_ = exp;
    }

    void set_type(const std::string &type) {
        type_ = type;
    }

private:
    std::string name_;
    syntax_tree_interface_ptr exp_;
    syntax_tree_interface_ptr prefixexp_;
    std::string type_;
};

// function call
class syntax_tree_functioncall : public syntax_tree_interface {
public:
    syntax_tree_functioncall(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_functioncall() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_functioncall; }

    virtual std::string dump(int tab = 0) const override;

    void set_prefixexp(const syntax_tree_interface_ptr &prefixexp) {
        prefixexp_ = prefixexp;
    }

    void set_args(const syntax_tree_interface_ptr &args) {
        args_ = args;
    }

    void set_name(const std::string &name) {
        name_ = name;
    }

private:
    syntax_tree_interface_ptr prefixexp_;
    syntax_tree_interface_ptr args_;
    std::string name_;
};

// table constructor
class syntax_tree_tableconstructor : public syntax_tree_interface {
public:
    syntax_tree_tableconstructor(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_tableconstructor() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_tableconstructor; }

    virtual std::string dump(int tab = 0) const override;

    void set_fieldlist(const syntax_tree_interface_ptr &fieldlist) {
        fieldlist_ = fieldlist;
    }

private:
    syntax_tree_interface_ptr fieldlist_;
};

// field list
class syntax_tree_fieldlist : public syntax_tree_interface {
public:
    syntax_tree_fieldlist(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_fieldlist() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_fieldlist; }

    virtual std::string dump(int tab = 0) const override;

    void add_field(const syntax_tree_interface_ptr &field) {
        fields_.push_back(field);
    }

private:
    std::vector<syntax_tree_interface_ptr> fields_;
};

// field assignment
class syntax_tree_field : public syntax_tree_interface {
public:
    syntax_tree_field(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_field() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_field; }

    virtual std::string dump(int tab = 0) const override;

    void set_key(const syntax_tree_interface_ptr &key) {
        key_ = key;
    }

    void set_value(const syntax_tree_interface_ptr &value) {
        value_ = value;
    }

    void set_name(const std::string &name) {
        name_ = name;
    }

    void set_type(const std::string &type) {
        type_ = type;
    }

private:
    syntax_tree_interface_ptr key_;
    syntax_tree_interface_ptr value_;
    std::string name_;
    std::string type_;
};

// break
class syntax_tree_break : public syntax_tree_interface {
public:
    syntax_tree_break(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_break() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_break; }

    virtual std::string dump(int tab = 0) const override;
};

// goto
class syntax_tree_goto : public syntax_tree_interface {
public:
    syntax_tree_goto(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_goto() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_goto; }

    virtual std::string dump(int tab = 0) const override;

    void set_label(const std::string &label) {
        label_ = label;
    }

private:
    std::string label_;
};

// while
class syntax_tree_while : public syntax_tree_interface {
public:
    syntax_tree_while(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_while() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_while; }

    virtual std::string dump(int tab = 0) const override;

    void set_exp(const syntax_tree_interface_ptr &exp) {
        exp_ = exp;
    }

    void set_block(const syntax_tree_interface_ptr &block) {
        block_ = block;
    }

private:
    syntax_tree_interface_ptr exp_;
    syntax_tree_interface_ptr block_;
};

// repeat
class syntax_tree_repeat : public syntax_tree_interface {
public:
    syntax_tree_repeat(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_repeat() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_repeat; }

    virtual std::string dump(int tab = 0) const override;

    void set_exp(const syntax_tree_interface_ptr &exp) {
        exp_ = exp;
    }

    void set_block(const syntax_tree_interface_ptr &block) {
        block_ = block;
    }

private:
    syntax_tree_interface_ptr exp_;
    syntax_tree_interface_ptr block_;
};

// if
class syntax_tree_if : public syntax_tree_interface {
public:
    syntax_tree_if(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_if() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_if; }

    virtual std::string dump(int tab = 0) const override;

    void set_exp(const syntax_tree_interface_ptr &exp) {
        exp_ = exp;
    }

    void set_block(const syntax_tree_interface_ptr &block) {
        block_ = block;
    }

    void set_elseiflist(const syntax_tree_interface_ptr &elseifs) {
        elseifs_ = elseifs;
    }

    void set_else_block(const syntax_tree_interface_ptr &elseblock) {
        elseblock_ = elseblock;
    }

private:
    syntax_tree_interface_ptr exp_;
    syntax_tree_interface_ptr block_;
    syntax_tree_interface_ptr elseifs_;
    syntax_tree_interface_ptr elseblock_;
};

// elseif_list
class syntax_tree_elseiflist : public syntax_tree_interface {
public:
    syntax_tree_elseiflist(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_elseiflist() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_elseiflist; }

    virtual std::string dump(int tab = 0) const override;

    void add_elseif_expr(const syntax_tree_interface_ptr &exp) {
        exps_.push_back(exp);
    }

    void add_elseif_block(const syntax_tree_interface_ptr &block) {
        blocks_.push_back(block);
    }

private:
    std::vector<syntax_tree_interface_ptr> exps_;
    std::vector<syntax_tree_interface_ptr> blocks_;
};

// for loop
class syntax_tree_for_loop : public syntax_tree_interface {
public:
    syntax_tree_for_loop(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_for_loop() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_for_loop; }

    virtual std::string dump(int tab = 0) const override;

    void set_name(const std::string &name) {
        name_ = name;
    }

    void set_block(const syntax_tree_interface_ptr &block) {
        block_ = block;
    }

    void set_exp_begin(const syntax_tree_interface_ptr &exp) {
        exp_begin_ = exp;
    }

    void set_exp_end(const syntax_tree_interface_ptr &exp) {
        exp_end_ = exp;
    }

    void set_exp_step(const syntax_tree_interface_ptr &exp) {
        exp_step_ = exp;
    }

private:
    std::string name_;
    syntax_tree_interface_ptr block_;
    syntax_tree_interface_ptr exp_begin_;
    syntax_tree_interface_ptr exp_end_;
    syntax_tree_interface_ptr exp_step_;
};

// for in
class syntax_tree_for_in : public syntax_tree_interface {
public:
    syntax_tree_for_in(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_for_in() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_for_in; }

    virtual std::string dump(int tab = 0) const override;

    void set_namelist(const syntax_tree_interface_ptr &namelist) {
        namelist_ = namelist;
    }

    void set_explist(const syntax_tree_interface_ptr &explist) {
        explist_ = explist;
    }

    void set_block(const syntax_tree_interface_ptr &block) {
        block_ = block;
    }

private:
    syntax_tree_interface_ptr namelist_;
    syntax_tree_interface_ptr explist_;
    syntax_tree_interface_ptr block_;
};

// name list
class syntax_tree_namelist : public syntax_tree_interface {
public:
    syntax_tree_namelist(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_namelist() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_namelist; }

    virtual std::string dump(int tab = 0) const override;

    void add_name(const std::string &name) {
        names_.push_back(name);
    }

    void add_attrib(const std::string &attrib) {
        attrib_.push_back(attrib);
    }

private:
    std::vector<std::string> names_;
    std::vector<std::string> attrib_;
};

// function
class syntax_tree_function : public syntax_tree_interface {
public:
    syntax_tree_function(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_function() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_function; }

    virtual std::string dump(int tab = 0) const override;

    void set_funcname(const syntax_tree_interface_ptr &funcname) {
        funcname_ = funcname;
    }

    void set_funcbody(const syntax_tree_interface_ptr &funcbody) {
        funcbody_ = funcbody;
    }

private:
    syntax_tree_interface_ptr funcname_;
    syntax_tree_interface_ptr funcbody_;
};

// funcnamelist
class syntax_tree_funcnamelist : public syntax_tree_interface {
public:
    syntax_tree_funcnamelist(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_funcnamelist() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_funcnamelist; }

    virtual std::string dump(int tab = 0) const override;

    void add_name(const std::string &funcname) {
        funcnames_.push_back(funcname);
    }

private:
    std::vector<std::string> funcnames_;
};

// funcname
class syntax_tree_funcname : public syntax_tree_interface {
public:
    syntax_tree_funcname(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_funcname() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_funcname; }

    virtual std::string dump(int tab = 0) const override;

    void set_funcnamelist(const syntax_tree_interface_ptr &funcnamelist) {
        funcnamelist_ = funcnamelist;
    }

    void set_colon_name(const std::string &name) {
        colon_name_ = name;
    }

private:
    syntax_tree_interface_ptr funcnamelist_;
    std::string colon_name_;
};

// funcbody
class syntax_tree_funcbody : public syntax_tree_interface {
public:
    syntax_tree_funcbody(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_funcbody() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_funcbody; }

    virtual std::string dump(int tab = 0) const override;

    void set_parlist(const syntax_tree_interface_ptr &parlist) {
        parlist_ = parlist;
    }

    void set_block(const syntax_tree_interface_ptr &block) {
        block_ = block;
    }

private:
    syntax_tree_interface_ptr parlist_;
    syntax_tree_interface_ptr block_;
};

// functiondef
class syntax_tree_functiondef : public syntax_tree_interface {
public:
    syntax_tree_functiondef(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_functiondef() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_functiondef; }

    virtual std::string dump(int tab = 0) const override;

    void set_funcbody(const syntax_tree_interface_ptr &funcbody) {
        funcbody_ = funcbody;
    }

private:
    syntax_tree_interface_ptr funcbody_;
};

// parlist
class syntax_tree_parlist : public syntax_tree_interface {
public:
    syntax_tree_parlist(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_parlist() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_parlist; }

    virtual std::string dump(int tab = 0) const override;

    void set_namelist(const syntax_tree_interface_ptr &namelist) {
        namelist_ = namelist;
    }

    void set_var_params(bool var_params) {
        var_params_ = var_params;
    }

private:
    syntax_tree_interface_ptr namelist_;
    bool var_params_ = false;
};

// local function
class syntax_tree_local_function : public syntax_tree_interface {
public:
    syntax_tree_local_function(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_local_function() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_local_function; }

    virtual std::string dump(int tab = 0) const override;

    void set_name(const std::string &name) {
        name_ = name;
    }

    void set_funcbody(const syntax_tree_interface_ptr &funcbody) {
        funcbody_ = funcbody;
    }

private:
    std::string name_;
    syntax_tree_interface_ptr funcbody_;
};

// local var
class syntax_tree_local_var : public syntax_tree_interface {
public:
    syntax_tree_local_var(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_local_var() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_local_var; }

    virtual std::string dump(int tab = 0) const override;

    void set_namelist(const syntax_tree_interface_ptr &namelist) {
        namelist_ = namelist;
    }

    void set_explist(const syntax_tree_interface_ptr &explist) {
        explist_ = explist;
    }

private:
    syntax_tree_interface_ptr namelist_;
    syntax_tree_interface_ptr explist_;
};

// exp
class syntax_tree_exp : public syntax_tree_interface {
public:
    syntax_tree_exp(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_exp() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_exp; }

    virtual std::string dump(int tab = 0) const override;

    void set_type(const std::string &type) {
        type_ = type;
    }

    void set_value(const std::string &value) {
        value_ = value;
    }

    void set_left(const syntax_tree_interface_ptr &left) {
        left_ = left;
    }

    void set_op(const syntax_tree_interface_ptr &op) {
        op_ = op;
    }

    void set_right(const syntax_tree_interface_ptr &right) {
        right_ = right;
    }

private:
    std::string type_;
    std::string value_;
    syntax_tree_interface_ptr left_;
    syntax_tree_interface_ptr op_;
    syntax_tree_interface_ptr right_;
};

// binop
class syntax_tree_binop : public syntax_tree_interface {
public:
    syntax_tree_binop(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_binop() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_binop; }

    virtual std::string dump(int tab = 0) const override;

    void set_op(const std::string &op) {
        op_ = op;
    }

private:
    std::string op_;
};

// unop
class syntax_tree_unop : public syntax_tree_interface {
public:
    syntax_tree_unop(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_unop() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_unop; }

    virtual std::string dump(int tab = 0) const override;

    void set_op(const std::string &op) {
        op_ = op;
    }

private:
    std::string op_;
};

// args
class syntax_tree_args : public syntax_tree_interface {
public:
    syntax_tree_args(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_args() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_args; }

    virtual std::string dump(int tab = 0) const override;

    void set_explist(const syntax_tree_interface_ptr &explist) {
        explist_ = explist;
    }

    void set_tableconstructor(const syntax_tree_interface_ptr &tableconstructor) {
        tableconstructor_ = tableconstructor;
    }

    void set_string(const std::string &string) {
        string_ = string;
    }

    void set_type(const std::string &type) {
        type_ = type;
    }

private:
    syntax_tree_interface_ptr explist_;
    syntax_tree_interface_ptr tableconstructor_;
    std::string string_;
    std::string type_;
};

// prefixexp
class syntax_tree_prefixexp : public syntax_tree_interface {
public:
    syntax_tree_prefixexp(const syntax_tree_location &loc) : syntax_tree_interface(loc) {}

    virtual ~syntax_tree_prefixexp() {}

    virtual syntax_tree_type type() const override { return syntax_tree_type_prefixexp; }

    virtual std::string dump(int tab = 0) const override;

    void set_value(const syntax_tree_interface_ptr &value) {
        value_ = value;
    }

    void set_type(const std::string &type) {
        type_ = type;
    }

private:
    syntax_tree_interface_ptr value_;
    std::string type_;
};

}// namespace fakelua
