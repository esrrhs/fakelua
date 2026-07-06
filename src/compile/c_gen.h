#pragma once

#include "compile/compile_common.h"
#include "fakelua.h"
#include <array>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fakelua {

class State;

// CGen —— 编译单元级别的 C 代码编排与函数体/表达式代码生成器。
// 当前为纯 CVar 代码生成器（无特化），所有数值/表操作走 CVar 运行时。
class CGen {
public:
    explicit CGen(State *s);

    // 核心代码生成入口
    GenResult Generate(const ParseResult &pr, const InferResult &ir, const AnalysisResult &ar, const CompileConfig &cfg);

private:
    // 当前写入的输出 section
    enum class Section { Headers, Globals, Decls, Impls, Body, Count };

    // RAII guard：临时切换 section，析构时自动恢复
    struct SectionGuard {
        CGen &gen;
        Section prev;
        SectionGuard(CGen &g, Section s) : gen(g), prev(g.cur_section_) { gen.cur_section_ = s; }
        ~SectionGuard() { gen.cur_section_ = prev; }
    };

    // ==========================================
    // 第一部分：核心调度与编排
    // ==========================================
    GenResult Build(const ParseResult &pr, const CompileConfig &cfg);
    void GenerateHeader();
    void GenerateShapeStructs();
    void GenerateGlobal(const SyntaxTreeInterfacePtr &chunk);
    void GenerateDecls(const SyntaxTreeInterfacePtr &chunk, GenResult &gr);
    void GenerateImpl(const SyntaxTreeInterfacePtr &chunk, GenResult &gr);

    std::string CompileFuncName(const SyntaxTreeInterfacePtr &ptr);
    std::vector<std::string> CompileParList(const SyntaxTreeInterfacePtr &parlist);
    void CompileFuncBody(const std::string &func_name, const std::vector<std::string> &func_params,
                         const SyntaxTreeInterfacePtr &func_block, std::ostream &out);
    [[nodiscard]] static bool BlockEndsWithReturn(const SyntaxTreeInterfacePtr &block);

    // ==========================================
    // 第二部分：语句编译
    // ==========================================
    void CompileStmtBlock(const SyntaxTreeInterfacePtr &block);
    void CompileStmt(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtReturn(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtLocalVar(const SyntaxTreeInterfacePtr &stmt);
    // 尝试以 struct literal 形式编译局部变量初始化，成功时返回 true。
    bool TryCompileLocalStructInit(const std::string &var_name, const SyntaxTreeInterfacePtr &exp_node);
    void CompileStmtAssign(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtFunctioncall(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtWhile(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtRepeat(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtIf(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtBreak(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtGoto(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtLabel(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtForLoop(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtForIn(const SyntaxTreeInterfacePtr &stmt);
    void CompileDynamicForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_stmt);
    void CompileScopedBlock(const SyntaxTreeInterfacePtr &block);
    std::string CompileCondBoolExpr(const SyntaxTreeInterfacePtr &exp, const std::string &tmp_prefix);

    // ==========================================
    // 第三部分：表达式编译
    // ==========================================
    [[nodiscard]] std::string CompileExp(const SyntaxTreeInterfacePtr &exp, bool preserve_multi = false);
    std::string CompilePrefixexp(const SyntaxTreeInterfacePtr &pe, bool preserve_multi = false);
    std::string CompileVar(const SyntaxTreeInterfacePtr &v);
    std::string CompileFunctioncall(const SyntaxTreeInterfacePtr &functioncall);
    std::string CompileTableconstructor(const SyntaxTreeInterfacePtr &tc);
    std::string CompileBinop(const SyntaxTreeInterfacePtr &left, const SyntaxTreeInterfacePtr &right, const SyntaxTreeInterfacePtr &op);
    std::string CompileUnop(const SyntaxTreeInterfacePtr &right, const SyntaxTreeInterfacePtr &op);

    // ==========================================
    // 第四部分：辅助
    // ==========================================
    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr);
    [[nodiscard]] std::string GenTab() const;

    // 返回当前 active 代码段（section）的输出流引用
    std::ostream &Out() { return sections_[static_cast<size_t>(cur_section_)]; }

    // 获取特定代码段（section）已输出的代码字符串内容
    [[nodiscard]] std::string GetSectionStr(Section sec) const { return sections_[static_cast<size_t>(sec)].str(); }

    // 快捷访问推断器全局上下文结果
    [[nodiscard]] const InferResult &ir() const { return ir_->get(); }
    [[nodiscard]] const AnalysisResult &ar() const { return ar_->get(); }

private:
    State *s_;
    std::string file_name_;
    std::optional<std::reference_wrapper<const InferResult>> ir_;
    std::optional<std::reference_wrapper<const AnalysisResult>> ar_;

    int tmp_var_counter_ = 0;
    std::unordered_map<std::string, JitFunctionInfo> local_func_names_;

    Section cur_section_ = Section::Headers;
    std::array<std::stringstream, static_cast<size_t>(Section::Count)> sections_;

    std::stringstream func_temp_decls_;
    int cur_tab_ = 0;

    // 当前正在编译的函数名（供逃逸判断/形状查询使用）
    std::string cur_func_name_;
};

}// namespace fakelua
