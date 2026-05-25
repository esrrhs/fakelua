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

}// namespace fakelua
