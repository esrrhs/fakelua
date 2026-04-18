#pragma once

#include "compile/compile_common.h"

namespace fakelua {

class TypeEnvironment {
public:
    TypeEnvironment();

    void EnterScope();

    void ExitScope();

    void Define(const std::string &name, InferredType type);

    bool Update(const std::string &name, InferredType type);

    [[nodiscard]] InferredType Lookup(const std::string &name) const;

private:
    static InferredType MergeType(InferredType old_type, InferredType new_type);

private:
    std::vector<std::unordered_map<std::string, InferredType>> scopes_;
};

class TypeInferencer {
public:
    void Process(const CompileResult &cr, const CompileConfig &cfg);

private:
    InferredType InferNode(const SyntaxTreeInterfacePtr &node);

    InferredType InferExp(const std::shared_ptr<SyntaxTreeExp> &exp);

    InferredType InferPrefixExp(const std::shared_ptr<SyntaxTreePrefixexp> &prefix_exp);

    InferredType InferVar(const std::shared_ptr<SyntaxTreeVar> &var);

    void InferBlock(const std::shared_ptr<SyntaxTreeBlock> &block, bool new_scope);

private:
    TypeEnvironment env_;
    int funcbody_depth_ = 0;
};

}// namespace fakelua
