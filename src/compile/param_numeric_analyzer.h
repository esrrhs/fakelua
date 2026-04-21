#pragma once

#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
#include "fakelua.h"

namespace fakelua {

// Analyzes which function parameters participate in numeric operations.
//
// A parameter is a "math param" if:
//   1. It (or a variable transitively assigned from it via simple var-to-var
//      assignments) appears as an operand of an arithmetic or bitwise binop
//      (+, -, *, /, //, %, ^, &, |, ~, <<, >>, <, <=, >, >=).
//   2. It is never directly reassigned inside the function body.
//
// Results are stored in CompileResult::math_param_positions.
// At most kMaxMathParams (8) math params are identified per function, giving
// at most 2^8 = 256 specializations.
class ParamNumericAnalyzer {
public:
    void Process(CompileResult &cr, const CompileConfig &cfg);

private:
    void AnalyzeFunction(const std::string &name,
                          const std::vector<std::string> &params,
                          const SyntaxTreeInterfacePtr &block,
                          CompileResult &cr);

    static const int kMaxMathParams = 8;
};

}// namespace fakelua
