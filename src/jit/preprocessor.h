#pragma once

#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "var/var.h"

namespace fakelua {

class PreProcessor {
public:
    PreProcessor(State *s);

    ~PreProcessor() = default;

    void Process(const FakeluaStatePtr &sp, const CompileConfig &cfg, const std::string &file_name, const SyntaxTreeInterfacePtr &chunk);

private:
    void PreprocessConst(const SyntaxTreeInterfacePtr &chunk);

    void PreprocessConstDefine(const SyntaxTreeInterfacePtr &stmt);

    void PreprocessFunctionsName(const SyntaxTreeInterfacePtr &chunk);

    void PreprocessFunctionName(const SyntaxTreeInterfacePtr &func);

    void SavePreprocessGlobalInit(const SyntaxTreeInterfacePtr &chunk);

    void PreprocessTableAssigns(const SyntaxTreeInterfacePtr &chunk);

    void PreprocessTableAssign(const SyntaxTreeInterfacePtr &funcbody);

    void PreprocessExtractsLiteralConstants(const SyntaxTreeInterfacePtr &chunk);

private:
    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr);

    std::string LocationStr(const SyntaxTreeInterfacePtr &ptr);

    void DumpDebugFile(const SyntaxTreeInterfacePtr &chunk, int step);

private:
    // the state contains the running environment we need.
    FakeluaStatePtr sp_;
    // the Compiler config
    std::string file_name_;
    // save the preprocess trunk new stmt in global_init func
    std::vector<SyntaxTreeInterfacePtr> global_init_new_stmt_;
    // temp var name counter
    int pre_index_ = 0;
};

}// namespace fakelua
