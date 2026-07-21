#pragma once

#include "compile/compile_common.h"
#include "fakelua.h"
#include <array>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace fakelua {

class State;

// CGen —— 编译单元级别的 C 代码编排与函数体/表达式代码生成器。
class CGen {
public:
    explicit CGen(State *s);

    // 核心代码生成入口
    GenResult Generate(const ParseResult &pr, const InferResult &ir, const AnalysisResult &ar, const CompileConfig &cfg);

private:
    // 当前写入的输出 section
    enum class Section { Headers, Globals, Decls, Impls, Body, Count };

    // RAII guard：临时切换 section，析构时自动恢复，天然异常安全
    struct SectionGuard {
        CGen &gen;
        Section prev;

        SectionGuard(CGen &g, Section s) : gen(g), prev(g.cur_section_) {
            gen.cur_section_ = s;
        }

        ~SectionGuard() {
            gen.cur_section_ = prev;
        }
    };

    // ==========================================
    // 第一部分：核心调度与编排
    // ==========================================
    // 核心构建主流程：根据 AST 和编译配置构建出最终的 C 语言源码字符串
    GenResult Build(const ParseResult &pr, const CompileConfig &cfg);
    // 为一个 spec 类型发射 typedef + 特化 get/set 函数到 Headers section。
    // 前置到 GenerateHeader 调用，读取 ir.spec_type_metadata（TypeInferencer 预计算）。
    void EmitSpecTypeBoilerplate(const std::string &spec_type, const struct SpecTypeMetadata &meta);
    // 生成文件头部样板代码（包含的头文件、基本类型定义等）
    void GenerateHeader();
    // 生成全局静态常量和全局变量相关的初始化代码
    void GenerateGlobal(const SyntaxTreeInterfacePtr &chunk);
    // 扫描 AST 并生成所有函数的原型前置声明
    void GenerateDecls(const SyntaxTreeInterfacePtr &chunk, GenResult &gr);
    // 生成所有函数具体的 C 语言代码实现
    void GenerateImpl(const SyntaxTreeInterfacePtr &chunk, GenResult &gr);

    // 根据 AST 节点生成符合规范的 C 函数名称
    std::string CompileFuncName(const SyntaxTreeInterfacePtr &ptr);
    // 编译参数列表节点，返回 C 语言格式的参数类型和名称字符串列表
    std::vector<std::string> CompileParList(const SyntaxTreeInterfacePtr &parlist);
    // 编译具体的函数体并将其 C 代码输出至给定的流中
    void CompileFuncBody(const std::string &func_name, const SyntaxTreeInterfacePtr &func_block, int spec_bitmask, std::ostream &out);
    // 为特定函数生成分发入口（用于处理动态调用到特化强类型调用的转发）
    void GenerateEntryDispatcher(const std::string &func_name, const std::vector<std::string> &func_params, const std::vector<int> &math_param_indices);
    // 辅助工具：判断一个基本块（Block）的结尾是否显式包含 return 语句
    [[nodiscard]] static bool BlockEndsWithReturn(const SyntaxTreeInterfacePtr &block);
    // 获取特定特化签名下函数的返回类型
    [[nodiscard]] InferredType GetSpecReturnType(const std::string &func_name, int bitmask) const;

    // table 特化辅助：从 prefixexp 获取变量的 spec 类型名（空字符串表示无特化）
    [[nodiscard]] std::string GetSpecTypeForVar(const SyntaxTreeInterfacePtr &pe) const;
    // table 特化辅助：检查 key 是否为已知 spec 字段
    [[nodiscard]] bool IsSpecField(const std::string &spec_type, const std::string &key, TableKeyKind kind) const;
    [[nodiscard]] std::string GetSpecFieldCName(const std::string &spec_type, const std::string &key, TableKeyKind kind) const;
    [[nodiscard]] int GetSpecFieldIndex(const std::string &spec_type, const std::string &key, TableKeyKind kind) const;
    [[nodiscard]] InferredType GetSpecFieldType(const std::string &spec_type, const std::string &key, TableKeyKind kind) const;
    // table 特化辅助：生成字段描述符（用于 map key）
    [[nodiscard]] static std::string GetKeyDescriptor(const std::string &key, TableKeyKind kind);
    // ==========================================
    // 第二部分：语句编译
    // ==========================================
    // 递归编译整个语句块
    void CompileStmtBlock(const SyntaxTreeInterfacePtr &block);
    // 编译单条语句（分发到各类型语句的专有编译函数）
    void CompileStmt(const SyntaxTreeInterfacePtr &stmt);
    // 编译 return 语句
    void CompileStmtReturn(const SyntaxTreeInterfacePtr &stmt);
    // 编译 local 局部变量声明语句（例如 local a = 1）
    void CompileStmtLocalVar(const SyntaxTreeInterfacePtr &stmt);
    // 编译赋值语句（例如 a = 1）
    void CompileStmtAssign(const SyntaxTreeInterfacePtr &stmt);
    // 编译 while 循环语句
    void CompileStmtWhile(const SyntaxTreeInterfacePtr &stmt);
    // 编译 repeat-until 循环语句
    void CompileStmtRepeat(const SyntaxTreeInterfacePtr &stmt);
    // 编译 if-elseif-else 条件控制语句
    void CompileStmtIf(const SyntaxTreeInterfacePtr &stmt);
    // 编译 break 退出循环语句
    void CompileStmtBreak(const SyntaxTreeInterfacePtr &stmt);
    // 编译 for 数值循环语句（根据控制变量的类型分发至特化循环编译）
    void CompileStmtForLoop(const SyntaxTreeInterfacePtr &stmt);
    // 编译控制变量特化为数值的高效原生 for 循环
    void CompileTypedNumericForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_stmt, InferredType loop_type);
    // 编译变量类型可能动态变化的通用 for 循环
    void CompileDynamicForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_stmt);
    // 编译泛型 for-in 遍历循环（如 ipairs/pairs 循环）
    void CompileStmtForIn(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtGoto(const SyntaxTreeInterfacePtr &stmt);
    void CompileStmtLabel(const SyntaxTreeInterfacePtr &stmt);

    // 编译条件布尔表达式（用于处理逻辑运算的短路特性与分支预测优化）
    std::string CompileCondBoolExpr(const SyntaxTreeInterfacePtr &exp, const std::string &tmp_prefix);

    // ==========================================
    // 第三部分：表达式编译
    // ==========================================
    // 核心表达式编译器入口，返回计算该表达式的 C 语言表达式源码
    [[nodiscard]] std::string CompileExp(const SyntaxTreeInterfacePtr &exp, bool preserve_multi = false);
    // 编译前缀表达式（如 `(exp)`、变量、表索引、或函数调用等）
    std::string CompilePrefixexp(const SyntaxTreeInterfacePtr &pe, bool preserve_multi = false);
    // 编译变量访问（包括局部变量、全局变量以及表属性字段的读取）
    std::string CompileVar(const SyntaxTreeInterfacePtr &v);
    // 编译普通的函数调用表达式并获取其返回值
    std::string CompileFunctioncall(const SyntaxTreeInterfacePtr &functioncall);
    // 编译表构造函数（即形如 `{ a = 1, b = 2 }` 的表创建字面量）
    std::string CompileTableconstructor(const SyntaxTreeInterfacePtr &tc);
    // 编译二元运算符表达式（如 `a + b`、`a == b` 等）
    std::string CompileBinop(const SyntaxTreeInterfacePtr &exp, const SyntaxTreeInterfacePtr &op);
    // 编译一元操作符表达式（如 `-a`、`#a`、`~a` 等）
    std::string CompileUnop(const SyntaxTreeInterfacePtr &exp, const SyntaxTreeInterfacePtr &op);

    // ==========================================
    // 第四部分：类型推断与原生优化辅助
    // ==========================================
    // 编译并优化可能为数值的表达式，最大程度使用强类型运算
    std::string CompileNumericExp(const SyntaxTreeInterfacePtr &exp);
    // 尝试将指定表达式编译为高效的 C 原生运算符直接计算（如直接输出 `a + b`，规避动态重载）
    std::string TryCompileNativeExpr(const SyntaxTreeInterfacePtr &exp);
    // 尝试将表达式直接编译为原生的布尔值比较/运算
    std::string TryCompileNativeBoolExpr(const SyntaxTreeInterfacePtr &exp);
    // 尝试对强类型数学特化库调用（如 math.sin, math.cos）进行原生直接映射优化
    std::string TryCompileNativeSpecCallExpr(const SyntaxTreeInterfacePtr &functioncall_node);

    // 生成原生二元算术运算的代码（result_type 来自 TypeInferencer 快照）
    std::string CompileNativeArithBinop(const SyntaxTreeInterfacePtr &left, const SyntaxTreeInterfacePtr &right, BinOpKind op_kind, InferredType result_type);
    // 生成底层未包装的原生算术/位运算表达式源码
    std::string CompileRawNativeArithBinop(const SyntaxTreeInterfacePtr &left, const SyntaxTreeInterfacePtr &right, BinOpKind op_kind, InferredType result_type);
    // 生成底层未包装的原生一元运算表达式源码
    std::string CompileRawNativeUnop(const SyntaxTreeInterfacePtr &right, UnOpKind op_kind, InferredType result_type);
    // 生成原生二元比较运算的代码
    std::string CompileNativeCmpBinop(const SyntaxTreeInterfacePtr &left, const SyntaxTreeInterfacePtr &right, BinOpKind op_kind);
    // 生成原生一元运算的代码（result_type 来自 TypeInferencer 快照）
    std::string CompileNativeUnop(const SyntaxTreeInterfacePtr &right, UnOpKind op_kind, InferredType result_type);

    // 获取表达式的推断类型：优先使用 TypeInferencer 预计算的快照，
    // 对简单变量额外查询 CGen 本地作用域（特化参数与 CGen 声明的局部变量）。
    [[nodiscard]] InferredType GetType(const SyntaxTreeInterfacePtr &exp) const;
    // 尝试推断数学特化库调用的参数掩码（bitmask）
    [[nodiscard]] bool TryInferMathCallBitmask(const std::string &callee_name, const std::vector<SyntaxTreeInterfacePtr> &raw_args, int &bitmask) const;
    // 尝试推断数学库特化调用的参数掩码与特化返回类型
    [[nodiscard]] bool TryInferMathCallSpec(const std::string &callee_name, const std::vector<SyntaxTreeInterfacePtr> &raw_args, int &bitmask, InferredType &spec_ret) const;
    // 根据 AST 节点指针在当前快照中查找推断出的类型
    [[nodiscard]] InferredType LookupNodeType(SyntaxTreeInterface *node) const;

    // 检查变量是否为已知强类型的原生变量
    [[nodiscard]] bool IsTypedNativeVar(const std::string &name, const SyntaxTreeInterface* var_node) const {
        if (const auto it = var_to_def_map_.find(var_node); it != var_to_def_map_.end()) {
            if (it->second->is_captured) {
                return false;
            }
        }
        // 1. 检查是否为特化函数参数，且参数类型为原生数值类型
        if (cur_spec_ctx_) {
            if (const auto it = cur_spec_ctx_->param_types.find(name); it != cur_spec_ctx_->param_types.end()) {
                return it->second == T_INT || it->second == T_FLOAT;
            }
        }
        // 2. 检查变量引用是否有定义节点映射，且该定义节点最终推断为原生数值类型
        if (const auto it = ir().var_define_nodes.find(var_node); it != ir().var_define_nodes.end()) {
            const auto def_type = LookupNodeType(const_cast<SyntaxTreeInterface*>(it->second));
            return def_type == T_INT || def_type == T_FLOAT;
        }
        return false;
    }

    // 获取原生强类型变量在作用域中的推断类型
    [[nodiscard]] InferredType GetNativeVarType(const std::string &name, const SyntaxTreeInterface* var_node) const {
        if (const auto it = var_to_def_map_.find(var_node); it != var_to_def_map_.end()) {
            if (it->second->is_captured) {
                return T_DYNAMIC;
            }
        }
        if (cur_spec_ctx_) {
            if (const auto it = cur_spec_ctx_->param_types.find(name); it != cur_spec_ctx_->param_types.end()) {
                return it->second;
            }
        }
        if (const auto it = ir().var_define_nodes.find(var_node); it != ir().var_define_nodes.end()) {
            return LookupNodeType(const_cast<SyntaxTreeInterface*>(it->second));
        }
        return T_DYNAMIC;
    }

    // 快捷访问推断器全局上下文结果
    [[nodiscard]] const InferResult &ir() const {
        return ir_->get();
    }

    // ==========================================
    // 第五部分：杂项辅助
    // ==========================================
    // 代码生成阶段抛出带有上下文树节点的异常错误
    [[noreturn]] void ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr);
    // 根据当前缩进深度生成 C 代码缩进空白字符
    [[nodiscard]] std::string GenTab() const;

    // 返回当前 active 代码段（section）的输出流引用
    std::ostream &Out() {
        return sections_[static_cast<size_t>(cur_section_)];
    }

    // 获取特定代码段（section）已输出的代码字符串内容
    [[nodiscard]] std::string GetSectionStr(Section sec) const {
        return sections_[static_cast<size_t>(sec)].str();
    }

private:
    State *s_;                                                      // 全局虚拟机状态 State 引用
    std::string file_name_;                                         // 当前编译的源码文件名称
    std::optional<std::reference_wrapper<const InferResult>> ir_;   // 全局类型推断结果快照引用
    std::optional<std::reference_wrapper<const AnalysisResult>> ar_;// 语义与控制流分析结果引用

    int tmp_var_counter_ = 0;// 临时变量生成计数器

    std::unordered_map<std::string, JitFunctionInfo> local_func_names_;// 本地函数（非全局）的名称映射及作用域标识

    [[nodiscard]] const AnalysisResult &ar() const {
        return ar_->get();
    }

    Section cur_section_ = Section::Headers;// 当前输出对应的 CGen 逻辑代码段

    std::array<std::stringstream, static_cast<size_t>(Section::Count)> sections_;// C 代码分区输出流数组

    const struct SpecFuncContext *cur_spec_ctx_ = nullptr;           // 当前特化版本的上下文（func_name/bitmask/snapshot）

    std::stringstream func_temp_decls_;// 用于临时存放函数体内部临时 C 变量声明的代码流
    int cur_tab_ = 0;                  // 当前 C 代码生成器所处的缩进级别深度

public:
    struct FuncInfo;

    struct VarDef {
        std::string name;
        const SyntaxTreeInterface *def_node = nullptr; // local stmt, parameter list, or loop stmt
        FuncInfo *defining_func = nullptr;
        bool is_captured = false;
    };

    struct Scope {
        FuncInfo *func = nullptr;
        std::unordered_map<std::string, VarDef *> vars;
    };

    struct FuncInfo {
        const SyntaxTreeInterface *node = nullptr; // Function, LocalFunction, or FunctionDef node
        SyntaxTreeInterfacePtr funcbody;
        FuncInfo *parent = nullptr;
        std::string name;
        std::string unique_c_name;
        std::vector<std::string> params;
        bool is_vararg = false;
        std::vector<VarDef *> captured_vars; // upvalues captured
        std::unordered_set<VarDef *> captured_set;
    };

private:
    std::vector<std::unique_ptr<VarDef>> all_defs_;
    std::vector<std::unique_ptr<FuncInfo>> all_funcs_;
    std::unordered_map<const SyntaxTreeInterface *, FuncInfo *> func_map_;
    
    struct PairHash {
        template <class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2> &p) const {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            return h1 ^ (h2 << 1);
        }
    };
    
    std::unordered_map<std::pair<const SyntaxTreeInterface *, std::string>, VarDef *, PairHash> stmt_var_to_def_;
    std::unordered_map<const SyntaxTreeInterface *, VarDef *> var_to_def_map_;
    
    FuncInfo *cur_func_info_ = nullptr; // The function currently being compiled
    std::string cur_package_name_; // Current package name declared in chunk

    void ResolveScopes(const SyntaxTreeInterfacePtr &node,
                       std::vector<Scope> &scopes,
                       std::vector<FuncInfo *> &func_stack,
                       FuncInfo *cur_func);

    bool IsPackageHeaderStmt(const SyntaxTreeInterfacePtr &stmt) const;
    std::string CompileUpvaluePointer(VarDef *def);
    void CompileStmtLocalFunction(const SyntaxTreeInterfacePtr &stmt);

};

} // namespace fakelua

