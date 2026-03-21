#pragma once

#include "Vm.h"
#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "gcc_jit_handle.h"
#include "var/var.h"
#include <libgccjit++.h>

namespace fakelua {

typedef std::shared_ptr<gccjit::context> GccjitContextPtr;

class GccJitter {
public:
    GccJitter() = default;

    ~GccJitter();

    void Compile(const FakeluaStatePtr &sp, const CompileConfig &cfg, const std::string &file_name, const SyntaxTreeInterfacePtr &chunk);

private:
    void CompileConstDefines(const SyntaxTreeInterfacePtr &chunk);

    void CompileConstDefine(const SyntaxTreeInterfacePtr &stmt);

    void CompileFunctions(const SyntaxTreeInterfacePtr &chunk);

    void CompileFunction(const std::string &name, const SyntaxTreeInterfacePtr &funcbody);

    std::string CompileFuncname(const SyntaxTreeInterfacePtr &ptr);

    gccjit::rvalue CompileExp(gccjit::function &func, const SyntaxTreeInterfacePtr &exp);

    std::vector<std::pair<std::string, gccjit::lvalue>> CompileParlist(gccjit::function &func, SyntaxTreeInterfacePtr parlist,
                                                                       int &IsVariadic);

    void CompileStmt(gccjit::function &func, const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtReturn(gccjit::function &func, const SyntaxTreeInterfacePtr &stmt);

    std::vector<gccjit::rvalue> CompileExplist(gccjit::function &func, const SyntaxTreeInterfacePtr &explist);

    gccjit::rvalue CompilePrefixexp(gccjit::function &func, const SyntaxTreeInterfacePtr &pe);

    gccjit::rvalue CompileVar(gccjit::function &func, const SyntaxTreeInterfacePtr &v);

    gccjit::lvalue CompileVarLvalue(gccjit::function &func, const SyntaxTreeInterfacePtr &v);

    void CompileStmtLocalVar(gccjit::function &function, const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtAssign(gccjit::function &function, const SyntaxTreeInterfacePtr &stmt);

    std::vector<gccjit::lvalue> CompileVarlistLvalue(gccjit::function &func, const SyntaxTreeInterfacePtr &explist);

    gccjit::rvalue CompileTableconstructor(gccjit::function &func, const SyntaxTreeInterfacePtr &tc);

    std::vector<gccjit::rvalue> CompileFieldlist(gccjit::function &func, const SyntaxTreeInterfacePtr &fieldlist);

    std::pair<gccjit::rvalue, gccjit::rvalue> CompileField(gccjit::function &func, const SyntaxTreeInterfacePtr &field);

    gccjit::rvalue CompileBinop(gccjit::function &func, const SyntaxTreeInterfacePtr &left, const SyntaxTreeInterfacePtr &right,
                                const SyntaxTreeInterfacePtr &op);

    gccjit::rvalue CompileUnop(gccjit::function &func, const SyntaxTreeInterfacePtr &right, const SyntaxTreeInterfacePtr &op);

    gccjit::rvalue CompileFunctioncall(gccjit::function &func, const SyntaxTreeInterfacePtr &functioncall);

    std::vector<gccjit::rvalue> CompileArgs(gccjit::function &func, const SyntaxTreeInterfacePtr &args);

    void CompileStmtFunctioncall(gccjit::function &func, const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtLabel(gccjit::function &func, const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtBlock(gccjit::function &func, const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtWhile(gccjit::function &func, const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtRepeat(gccjit::function &func, const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtIf(gccjit::function &func, const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtBreak(gccjit::function &func, const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtForLoop(gccjit::function &func, const SyntaxTreeInterfacePtr &stmt);

    void CompileStmtForIn(gccjit::function &func, const SyntaxTreeInterfacePtr &stmt);

private:
    gccjit::location NewLocation(const SyntaxTreeInterfacePtr &ptr);

    std::string NewBlockName(const std::string &name, const SyntaxTreeInterfacePtr &ptr);

    void CallGlobalInitFunc();

    std::string LocationStr(const SyntaxTreeInterfacePtr &ptr);

    gccjit::lvalue FindLvalueByName(const std::string &name, const SyntaxTreeInterfacePtr &ptr);

    std::optional<gccjit::lvalue> TryFindLvalueByName(const std::string &name, const SyntaxTreeInterfacePtr &ptr);

    void SaveStackLvalueByName(const std::string &name, const gccjit::lvalue &value, const SyntaxTreeInterfacePtr &ptr);

    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr);

    void CheckReturnBlock(gccjit::function &func, const SyntaxTreeInterfacePtr &ptr);

    bool IsBlockEnded();

    void InitGlobalConstVar(gccjit::function &func);

private:
    void PrepareCompile(const FakeluaStatePtr &sp, const CompileConfig &cfg, const std::string &file_name);

    bool IsSimpleAssign(const SyntaxTreeInterfacePtr &vars, const SyntaxTreeInterfacePtr &exps);

    bool IsSimpleArgs(const SyntaxTreeInterfacePtr &args);

    bool IsSimpleExplist(const SyntaxTreeInterfacePtr &explist);

    bool IsSimpleExp(const SyntaxTreeInterfacePtr &exp);

    bool IsSimplePrefixexp(const SyntaxTreeInterfacePtr &pe);

    bool IsSimpleTableconstructor(const SyntaxTreeInterfacePtr &tc);

    bool IsSimpleField(const SyntaxTreeInterfacePtr &fieldlist);

    std::string GetSimplePrefixexpName(const SyntaxTreeInterfacePtr &pe);

    std::string GetSimpleVarName(const SyntaxTreeInterfacePtr &v);

    bool IsJitBuiltinFunction(const std::string &name);

    std::string GetJitBuiltinFunctionVmName(const std::string &name);

    void SetVarInt(gccjit::lvalue &var, int64_t v, bool is_const, const SyntaxTreeInterfacePtr &p);

    void SetVarFloat(gccjit::lvalue &var, double v, bool is_const, const SyntaxTreeInterfacePtr &p);

    void SetVarString(gccjit::lvalue &var, const std::string &v, bool is_const, const SyntaxTreeInterfacePtr &p);

private:
    // the helper type in gccjit
    gccjit::type void_ptr_type_;
    gccjit::type int_type_;
    gccjit::type int64_type_;
    gccjit::type double_type_;
    gccjit::type bool_type_;
    gccjit::type const_char_ptr_type_;
    gccjit::type size_t_type_;

    gccjit::field var_type_field_;
    gccjit::field var_flag_field_;
    gccjit::field var_data_b_field_;
    gccjit::field var_data_i_field_;
    gccjit::field var_data_f_field_;
    gccjit::field var_data_s_field_;
    gccjit::field var_data_t_field_;
    gccjit::field var_data_field_;
    gccjit::type var_data_type_;
    gccjit::type var_struct_;

    gccjit::lvalue global_const_null_var_;
    gccjit::lvalue global_const_false_var_;
    gccjit::lvalue global_const_true_var_;

private:
    // the state contains the running environment we need.
    FakeluaStatePtr sp_;
    // the Compiler config
    std::string file_name_;
    // gccjit context
    GccjitContextPtr gccjit_context_;
    GccJitHandlePtr gcc_jit_handle_;
    // function info saves here
    struct FunctionInfo {
        int params_count = 0;
        bool IsVariadic = false;
        gccjit::function func;
    };
    // function name -> function info
    std::unordered_map<std::string, FunctionInfo> function_infos_;
    // global const var name -> gcc_jit_lvalue
    std::unordered_map<std::string, std::pair<gccjit::lvalue, SyntaxTreeInterfacePtr>> global_const_vars_;
    // compiling function tmp data
    struct FunctionData {
        // save stack frame vars
        struct StackFrame {
            std::unordered_map<std::string, gccjit::lvalue> local_vars;
        };
        // every block stack frame
        std::vector<StackFrame> stack_frames;
        // record the block ended
        std::unordered_set<gcc_jit_block *> ended_blocks;
        // use to save the current block
        gccjit::block cur_block;
        // used for jmp to
        std::vector<gccjit::block> stack_end_blocks;
        // temp var name counter
        int tmp_index = 0;
        // mark cur func if is const
        bool is_const = false;
        // current compiling function name
        std::string cur_function_name;
        // current compiling function
        gccjit::function cur_gccjit_func;
        // current func variadic param start index, if IsVariadic, it is the index of '...' param
        int cur_variadic_param_start_index = -1;
    };
    // current compiling function data
    FunctionData cur_function_data_;
    // save the preprocess trunk new stmt
    std::vector<SyntaxTreeInterfacePtr> preprocess_trunk_new_stmt_;
};

typedef std::shared_ptr<GccJitter> GccJitterPtr;

}// namespace fakelua
