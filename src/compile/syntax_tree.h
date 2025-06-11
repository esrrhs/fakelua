#pragma once

#include "bison/location.hh"
#include "util/common.h"
#include "util/exception.h"

namespace fakelua {

// syntax tree type
enum class syntax_tree_type {
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
    explicit syntax_tree_interface(const syntax_tree_location &loc) : loc_(loc) {
    }

    virtual ~syntax_tree_interface() = default;

    // get syntax tree type
    [[nodiscard]] virtual syntax_tree_type type() const = 0;

    // dump a syntax tree to string
    [[nodiscard]] virtual std::string dump(int tab) const = 0;

    // get syntax tree location
    [[nodiscard]] const syntax_tree_location &loc() const {
        return loc_;
    }

protected:
    // generate tab string
    [[nodiscard]] std::string gen_tab(int tab) const {
        std::string str;
        for (int i = 0; i < tab; ++i) {
            str += "  ";
        }
        return str;
    }

    // generate location string
    [[nodiscard]] std::string loc_str() const {
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
class syntax_tree_empty final : public syntax_tree_interface {
public:
    explicit syntax_tree_empty(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_empty;
    }

    [[nodiscard]] std::string dump(int tab) const override;
};

// block
class syntax_tree_block final : public syntax_tree_interface {
public:
    explicit syntax_tree_block(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_block() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_block;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void add_stmt(const syntax_tree_interface_ptr &stmt) {
        stmts_.push_back(stmt);
    }

    [[nodiscard]] const std::vector<syntax_tree_interface_ptr> &stmts() const {
        return stmts_;
    }

    void set_stmts(const std::vector<syntax_tree_interface_ptr> &stmts) {
        stmts_ = stmts;
    }

private:
    std::vector<syntax_tree_interface_ptr> stmts_;
};

// label
class syntax_tree_label final : public syntax_tree_interface {
public:
    explicit syntax_tree_label(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_label() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_label;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_name(const std::string &name) {
        name_ = name;
    }

private:
    std::string name_;
};

// return
class syntax_tree_return final : public syntax_tree_interface {
public:
    explicit syntax_tree_return(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_return() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_return;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_explist(const syntax_tree_interface_ptr &explist) {
        explist_ = explist;
    }

    [[nodiscard]] syntax_tree_interface_ptr explist() const {
        return explist_;
    }

private:
    syntax_tree_interface_ptr explist_;
};

// assign
class syntax_tree_assign final : public syntax_tree_interface {
public:
    explicit syntax_tree_assign(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_assign() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_assign;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_varlist(const syntax_tree_interface_ptr &varlist) {
        varlist_ = varlist;
    }

    void set_explist(const syntax_tree_interface_ptr &explist) {
        explist_ = explist;
    }

    [[nodiscard]] syntax_tree_interface_ptr varlist() const {
        return varlist_;
    }

    [[nodiscard]] syntax_tree_interface_ptr explist() const {
        return explist_;
    }

private:
    syntax_tree_interface_ptr varlist_;
    syntax_tree_interface_ptr explist_;
};

// var list
class syntax_tree_varlist final : public syntax_tree_interface {
public:
    explicit syntax_tree_varlist(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_varlist() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_varlist;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void add_var(const syntax_tree_interface_ptr &var) {
        vars_.push_back(var);
    }

    std::vector<syntax_tree_interface_ptr> &vars() {
        return vars_;
    }

private:
    std::vector<syntax_tree_interface_ptr> vars_;
};

// exp list
class syntax_tree_explist final : public syntax_tree_interface {
public:
    explicit syntax_tree_explist(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_explist() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_explist;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void add_exp(const syntax_tree_interface_ptr &exp) {
        exps_.push_back(exp);
    }

    std::vector<syntax_tree_interface_ptr> &exps() {
        return exps_;
    }

private:
    std::vector<syntax_tree_interface_ptr> exps_;
};

// var
class syntax_tree_var final : public syntax_tree_interface {
public:
    explicit syntax_tree_var(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_var() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_var;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_name(const std::string &name) {
        name_ = name;
    }

    void set_prefixexp(const syntax_tree_interface_ptr &prefixexp) {
        prefixexp_ = prefixexp;
    }

    syntax_tree_interface_ptr get_prefixexp() {
        return prefixexp_;
    }

    void set_exp(const syntax_tree_interface_ptr &exp) {
        exp_ = exp;
    }

    syntax_tree_interface_ptr get_exp() {
        return exp_;
    }

    void set_type(const std::string &type) {
        type_ = type;
    }

    [[nodiscard]] std::string get_name() const {
        return name_;
    }

    [[nodiscard]] syntax_tree_interface_ptr get_prefixexp() const {
        return prefixexp_;
    }

    [[nodiscard]] syntax_tree_interface_ptr get_exp() const {
        return exp_;
    }

    [[nodiscard]] std::string get_type() const {
        return type_;
    }

private:
    std::string name_;
    syntax_tree_interface_ptr exp_;
    syntax_tree_interface_ptr prefixexp_;
    std::string type_;
};

// function call
class syntax_tree_functioncall final : public syntax_tree_interface {
public:
    explicit syntax_tree_functioncall(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_functioncall() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_functioncall;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_prefixexp(const syntax_tree_interface_ptr &prefixexp) {
        prefixexp_ = prefixexp;
    }

    void set_args(const syntax_tree_interface_ptr &args) {
        args_ = args;
    }

    void set_name(const std::string &name) {
        name_ = name;
    }

    [[nodiscard]] syntax_tree_interface_ptr prefixexp() const {
        return prefixexp_;
    }

    [[nodiscard]] syntax_tree_interface_ptr args() const {
        return args_;
    }

    [[nodiscard]] std::string name() const {
        return name_;
    }

private:
    syntax_tree_interface_ptr prefixexp_;
    syntax_tree_interface_ptr args_;
    std::string name_;
};

// table constructor
class syntax_tree_tableconstructor final : public syntax_tree_interface {
public:
    explicit syntax_tree_tableconstructor(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_tableconstructor() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_tableconstructor;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_fieldlist(const syntax_tree_interface_ptr &fieldlist) {
        fieldlist_ = fieldlist;
    }

    [[nodiscard]] syntax_tree_interface_ptr fieldlist() const {
        return fieldlist_;
    }

private:
    syntax_tree_interface_ptr fieldlist_;
};

// field list
class syntax_tree_fieldlist final : public syntax_tree_interface {
public:
    explicit syntax_tree_fieldlist(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_fieldlist() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_fieldlist;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void add_field(const syntax_tree_interface_ptr &field) {
        fields_.push_back(field);
    }

    std::vector<syntax_tree_interface_ptr> &fields() {
        return fields_;
    }

private:
    std::vector<syntax_tree_interface_ptr> fields_;
};

// field assignment
class syntax_tree_field final : public syntax_tree_interface {
public:
    explicit syntax_tree_field(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_field() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_field;
    }

    [[nodiscard]] std::string dump(int tab) const override;

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

    [[nodiscard]] syntax_tree_interface_ptr key() const {
        return key_;
    }

    [[nodiscard]] syntax_tree_interface_ptr value() const {
        return value_;
    }

    [[nodiscard]] std::string name() const {
        return name_;
    }

    [[nodiscard]] std::string get_type() const {
        return type_;
    }

private:
    syntax_tree_interface_ptr key_;
    syntax_tree_interface_ptr value_;
    std::string name_;
    std::string type_;
};

// break
class syntax_tree_break final : public syntax_tree_interface {
public:
    explicit syntax_tree_break(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_break() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_break;
    }

    [[nodiscard]] std::string dump(int tab) const override;
};

// goto
class syntax_tree_goto final : public syntax_tree_interface {
public:
    explicit syntax_tree_goto(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_goto() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_goto;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_label(const std::string &label) {
        label_ = label;
    }

private:
    std::string label_;
};

// while
class syntax_tree_while final : public syntax_tree_interface {
public:
    explicit syntax_tree_while(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_while() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_while;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_exp(const syntax_tree_interface_ptr &exp) {
        exp_ = exp;
    }

    void set_block(const syntax_tree_interface_ptr &block) {
        block_ = block;
    }

    [[nodiscard]] syntax_tree_interface_ptr exp() const {
        return exp_;
    }

    [[nodiscard]] syntax_tree_interface_ptr block() const {
        return block_;
    }

private:
    syntax_tree_interface_ptr exp_;
    syntax_tree_interface_ptr block_;
};

// repeat
class syntax_tree_repeat final : public syntax_tree_interface {
public:
    explicit syntax_tree_repeat(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_repeat() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_repeat;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_exp(const syntax_tree_interface_ptr &exp) {
        exp_ = exp;
    }

    void set_block(const syntax_tree_interface_ptr &block) {
        block_ = block;
    }

    [[nodiscard]] syntax_tree_interface_ptr exp() const {
        return exp_;
    }

    [[nodiscard]] syntax_tree_interface_ptr block() const {
        return block_;
    }

private:
    syntax_tree_interface_ptr exp_;
    syntax_tree_interface_ptr block_;
};

// if
class syntax_tree_if final : public syntax_tree_interface {
public:
    explicit syntax_tree_if(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_if() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_if;
    }

    [[nodiscard]] std::string dump(int tab) const override;

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

    [[nodiscard]] syntax_tree_interface_ptr exp() const {
        return exp_;
    }

    [[nodiscard]] syntax_tree_interface_ptr block() const {
        return block_;
    }

    [[nodiscard]] syntax_tree_interface_ptr elseifs() const {
        return elseifs_;
    }

    [[nodiscard]] syntax_tree_interface_ptr elseblock() const {
        return elseblock_;
    }

private:
    syntax_tree_interface_ptr exp_;
    syntax_tree_interface_ptr block_;
    syntax_tree_interface_ptr elseifs_;
    syntax_tree_interface_ptr elseblock_;
};

// elseif_list
class syntax_tree_elseiflist final : public syntax_tree_interface {
public:
    explicit syntax_tree_elseiflist(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_elseiflist() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_elseiflist;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void add_elseif_expr(const syntax_tree_interface_ptr &exp) {
        exps_.push_back(exp);
    }

    void add_elseif_block(const syntax_tree_interface_ptr &block) {
        blocks_.push_back(block);
    }

    [[nodiscard]] size_t elseif_size() const {
        return exps_.size();
    }

    [[nodiscard]] const std::vector<syntax_tree_interface_ptr> &elseif_exps() const {
        return exps_;
    }

    [[nodiscard]] const std::vector<syntax_tree_interface_ptr> &elseif_blocks() const {
        return blocks_;
    }

    [[nodiscard]] syntax_tree_interface_ptr elseif_exp(size_t idx) const {
        return exps_[idx];
    }

    [[nodiscard]] syntax_tree_interface_ptr elseif_block(size_t idx) const {
        return blocks_[idx];
    }

private:
    std::vector<syntax_tree_interface_ptr> exps_;
    std::vector<syntax_tree_interface_ptr> blocks_;
};

// for loop
class syntax_tree_for_loop final : public syntax_tree_interface {
public:
    explicit syntax_tree_for_loop(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_for_loop() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_for_loop;
    }

    [[nodiscard]] std::string dump(int tab) const override;

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

    [[nodiscard]] std::string name() const {
        return name_;
    }

    [[nodiscard]] syntax_tree_interface_ptr block() const {
        return block_;
    }

    [[nodiscard]] syntax_tree_interface_ptr exp_begin() const {
        return exp_begin_;
    }

    [[nodiscard]] syntax_tree_interface_ptr exp_end() const {
        return exp_end_;
    }

    [[nodiscard]] syntax_tree_interface_ptr exp_step() const {
        return exp_step_;
    }

private:
    std::string name_;
    syntax_tree_interface_ptr block_;
    syntax_tree_interface_ptr exp_begin_;
    syntax_tree_interface_ptr exp_end_;
    syntax_tree_interface_ptr exp_step_;
};

// for in
class syntax_tree_for_in final : public syntax_tree_interface {
public:
    explicit syntax_tree_for_in(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_for_in() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_for_in;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_namelist(const syntax_tree_interface_ptr &namelist) {
        namelist_ = namelist;
    }

    void set_explist(const syntax_tree_interface_ptr &explist) {
        explist_ = explist;
    }

    void set_block(const syntax_tree_interface_ptr &block) {
        block_ = block;
    }

    [[nodiscard]] syntax_tree_interface_ptr namelist() const {
        return namelist_;
    }

    [[nodiscard]] syntax_tree_interface_ptr explist() const {
        return explist_;
    }

    [[nodiscard]] syntax_tree_interface_ptr block() const {
        return block_;
    }

private:
    syntax_tree_interface_ptr namelist_;
    syntax_tree_interface_ptr explist_;
    syntax_tree_interface_ptr block_;
};

// name list
class syntax_tree_namelist final : public syntax_tree_interface {
public:
    explicit syntax_tree_namelist(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_namelist() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_namelist;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void add_name(const std::string &name) {
        names_.push_back(name);
    }

    void add_attrib(const std::string &attrib) {
        attrib_.push_back(attrib);
    }

    [[nodiscard]] const std::vector<std::string> &names() const {
        return names_;
    }

private:
    std::vector<std::string> names_;
    std::vector<std::string> attrib_;
};

// function
class syntax_tree_function final : public syntax_tree_interface {
public:
    explicit syntax_tree_function(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_function() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_function;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_funcname(const syntax_tree_interface_ptr &funcname) {
        funcname_ = funcname;
    }

    void set_funcbody(const syntax_tree_interface_ptr &funcbody) {
        funcbody_ = funcbody;
    }

    [[nodiscard]] syntax_tree_interface_ptr funcname() const {
        return funcname_;
    }

    [[nodiscard]] syntax_tree_interface_ptr funcbody() const {
        return funcbody_;
    }

private:
    syntax_tree_interface_ptr funcname_;
    syntax_tree_interface_ptr funcbody_;
};

// funcnamelist
class syntax_tree_funcnamelist final : public syntax_tree_interface {
public:
    explicit syntax_tree_funcnamelist(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_funcnamelist() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_funcnamelist;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void add_name(const std::string &funcname) {
        funcnames_.push_back(funcname);
    }

    [[nodiscard]] const std::vector<std::string> &funcnames() const {
        return funcnames_;
    }

private:
    std::vector<std::string> funcnames_;
};

// funcname
class syntax_tree_funcname final : public syntax_tree_interface {
public:
    explicit syntax_tree_funcname(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_funcname() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_funcname;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_funcnamelist(const syntax_tree_interface_ptr &funcnamelist) {
        funcnamelist_ = funcnamelist;
    }

    void set_colon_name(const std::string &name) {
        colon_name_ = name;
    }

    [[nodiscard]] syntax_tree_interface_ptr funcnamelist() const {
        return funcnamelist_;
    }

    [[nodiscard]] std::string colon_name() const {
        return colon_name_;
    }

private:
    syntax_tree_interface_ptr funcnamelist_;
    std::string colon_name_;
};

// funcbody
class syntax_tree_funcbody final : public syntax_tree_interface {
public:
    explicit syntax_tree_funcbody(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_funcbody() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_funcbody;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_parlist(const syntax_tree_interface_ptr &parlist) {
        parlist_ = parlist;
    }

    void set_block(const syntax_tree_interface_ptr &block) {
        block_ = block;
    }

    [[nodiscard]] syntax_tree_interface_ptr parlist() const {
        return parlist_;
    }

    [[nodiscard]] syntax_tree_interface_ptr block() const {
        return block_;
    }

private:
    syntax_tree_interface_ptr parlist_;
    syntax_tree_interface_ptr block_;
};

// functiondef
class syntax_tree_functiondef final : public syntax_tree_interface {
public:
    explicit syntax_tree_functiondef(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_functiondef() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_functiondef;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_funcbody(const syntax_tree_interface_ptr &funcbody) {
        funcbody_ = funcbody;
    }

    [[nodiscard]] syntax_tree_interface_ptr funcbody() const {
        return funcbody_;
    }

private:
    syntax_tree_interface_ptr funcbody_;
};

// parlist
class syntax_tree_parlist final : public syntax_tree_interface {
public:
    explicit syntax_tree_parlist(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_parlist() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_parlist;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_namelist(const syntax_tree_interface_ptr &namelist) {
        namelist_ = namelist;
    }

    void set_var_params(bool var_params) {
        var_params_ = var_params;
    }

    [[nodiscard]] syntax_tree_interface_ptr namelist() const {
        return namelist_;
    }

    [[nodiscard]] bool var_params() const {
        return var_params_;
    }

private:
    syntax_tree_interface_ptr namelist_;
    bool var_params_ = false;
};

// local function
class syntax_tree_local_function final : public syntax_tree_interface {
public:
    explicit syntax_tree_local_function(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_local_function() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_local_function;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_name(const std::string &name) {
        name_ = name;
    }

    void set_funcbody(const syntax_tree_interface_ptr &funcbody) {
        funcbody_ = funcbody;
    }

    [[nodiscard]] std::string name() const {
        return name_;
    }

    [[nodiscard]] syntax_tree_interface_ptr funcbody() const {
        return funcbody_;
    }

private:
    std::string name_;
    syntax_tree_interface_ptr funcbody_;
};

// local var
class syntax_tree_local_var final : public syntax_tree_interface {
public:
    explicit syntax_tree_local_var(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_local_var() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_local_var;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_namelist(const syntax_tree_interface_ptr &namelist) {
        namelist_ = namelist;
    }

    void set_explist(const syntax_tree_interface_ptr &explist) {
        explist_ = explist;
    }

    [[nodiscard]] syntax_tree_interface_ptr namelist() const {
        return namelist_;
    }

    [[nodiscard]] syntax_tree_interface_ptr explist() const {
        return explist_;
    }

private:
    syntax_tree_interface_ptr namelist_;
    syntax_tree_interface_ptr explist_;
};

// exp
class syntax_tree_exp final : public syntax_tree_interface {
public:
    explicit syntax_tree_exp(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_exp() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_exp;
    }

    [[nodiscard]] std::string dump(int tab) const override;

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

    [[nodiscard]] std::string exp_type() const {
        return type_;
    }

    [[nodiscard]] std::string exp_value() const {
        return value_;
    }

    [[nodiscard]] syntax_tree_interface_ptr left() const {
        return left_;
    }

    [[nodiscard]] syntax_tree_interface_ptr op() const {
        return op_;
    }

    [[nodiscard]] syntax_tree_interface_ptr right() const {
        return right_;
    }

private:
    std::string type_;
    std::string value_;
    syntax_tree_interface_ptr left_;
    syntax_tree_interface_ptr op_;
    syntax_tree_interface_ptr right_;
};

// binop
class syntax_tree_binop final : public syntax_tree_interface {
public:
    explicit syntax_tree_binop(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_binop() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_binop;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_op(const std::string &op) {
        op_ = op;
    }

    [[nodiscard]] std::string get_op() const {
        return op_;
    }

private:
    std::string op_;
};

// unop
class syntax_tree_unop final : public syntax_tree_interface {
public:
    explicit syntax_tree_unop(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_unop() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_unop;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_op(const std::string &op) {
        op_ = op;
    }

    [[nodiscard]] std::string get_op() const {
        return op_;
    }

private:
    std::string op_;
};

// args
class syntax_tree_args final : public syntax_tree_interface {
public:
    explicit syntax_tree_args(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_args() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_args;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_explist(const syntax_tree_interface_ptr &explist) {
        explist_ = explist;
    }

    void set_tableconstructor(const syntax_tree_interface_ptr &tableconstructor) {
        tableconstructor_ = tableconstructor;
    }

    void set_string(const syntax_tree_interface_ptr &string) {
        string_ = string;
    }

    void set_type(const std::string &type) {
        type_ = type;
    }

    [[nodiscard]] std::string get_type() const {
        return type_;
    }

    [[nodiscard]] syntax_tree_interface_ptr explist() const {
        return explist_;
    }

    [[nodiscard]] syntax_tree_interface_ptr tableconstructor() const {
        return tableconstructor_;
    }

    [[nodiscard]] syntax_tree_interface_ptr string() const {
        return string_;
    }

private:
    syntax_tree_interface_ptr explist_;
    syntax_tree_interface_ptr tableconstructor_;
    syntax_tree_interface_ptr string_;
    std::string type_;
};

// prefixexp
class syntax_tree_prefixexp final : public syntax_tree_interface {
public:
    explicit syntax_tree_prefixexp(const syntax_tree_location &loc) : syntax_tree_interface(loc) {
    }

    ~syntax_tree_prefixexp() override = default;

    [[nodiscard]] syntax_tree_type type() const override {
        return syntax_tree_type::syntax_tree_type_prefixexp;
    }

    [[nodiscard]] std::string dump(int tab) const override;

    void set_value(const syntax_tree_interface_ptr &value) {
        value_ = value;
    }

    void set_type(const std::string &type) {
        type_ = type;
    }

    [[nodiscard]] syntax_tree_interface_ptr get_value() const {
        return value_;
    }

    [[nodiscard]] std::string get_type() const {
        return type_;
    }

private:
    syntax_tree_interface_ptr value_;
    std::string type_;
};

typedef std::function<void(const syntax_tree_interface_ptr &)> walk_syntax_tree_func;
void walk_syntax_tree(const syntax_tree_interface_ptr &node, const walk_syntax_tree_func& func);

}// namespace fakelua
