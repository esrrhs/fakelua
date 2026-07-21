#include "compile/c_gen.h"

#include <ranges>

#include "compile/c_runtime_header.h"
#include "state/state.h"
#include "util/common.h"
#include "util/exception.h"
#include "util/file_util.h"

namespace fakelua {

namespace {

// 剥离 Lua 字符串字面量的引号（"..." 或 '...'），返回内容部分。
// 若不以引号开头则原样返回。
std::string StripLuaStringQuotes(const std::string &raw) {
    if (raw.size() >= 2 && (raw.front() == '"' || raw.front() == '\'')) {
        return raw.substr(1, raw.size() - 2);
    }
    return raw;
}

// 尝试从一条语句中提取 package 声明名。
// 支持两种 AST 形态：FunctionCall（package "xxx"）和 Assign（package = "xxx"）。
// 成功则 out_name 被赋值，返回 true；否则返回 false。
bool ExtractPackageName(const SyntaxTreeInterfacePtr &stmt, std::string &out_name) {
    if (stmt->Type() == SyntaxTreeType::FunctionCall) {
        const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(stmt);
        if (fc && fc->prefixexp() && fc->prefixexp()->Type() == SyntaxTreeType::PrefixExp) {
            const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
            if (pe && pe->GetPrefixKind() == PrefixExpKind::kVar && pe->GetValue()) {
                const auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue());
                if (v && v->GetName() == "package" && fc->Args()) {
                    const auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(fc->Args());
                    if (args && args->GetArgsKind() == ArgsKind::kString && args->String()) {
                        const auto str_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(args->String());
                        if (str_exp) {
                            out_name = StripLuaStringQuotes(str_exp->ExpValue());
                            return true;
                        }
                    } else if (args && args->GetArgsKind() == ArgsKind::kExpList && args->Explist()) {
                        const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(args->Explist());
                        if (el && !el->Exps().empty()) {
                            const auto str_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(el->Exps()[0]);
                            if (str_exp && str_exp->GetExpKind() == ExpKind::kString) {
                                out_name = StripLuaStringQuotes(str_exp->ExpValue());
                                return true;
                            }
                        }
                    }
                }
            }
        }
    } else if (stmt->Type() == SyntaxTreeType::Assign) {
        const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);
        if (assign && assign->Varlist() && assign->Explist()) {
            const auto vl = std::dynamic_pointer_cast<SyntaxTreeVarlist>(assign->Varlist());
            const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(assign->Explist());
            if (vl && !vl->Vars().empty() && el && !el->Exps().empty()) {
                const auto v = std::dynamic_pointer_cast<SyntaxTreeVar>(vl->Vars()[0]);
                if (v && v->GetName() == "package") {
                    const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(el->Exps()[0]);
                    if (exp && exp->GetExpKind() == ExpKind::kString) {
                        out_name = StripLuaStringQuotes(exp->ExpValue());
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// 判断一条语句是否为 package 声明语句（只检查形态，不提取名字）。
bool IsPackageDeclStmt(const SyntaxTreeInterfacePtr &stmt) {
    std::string ignored;
    return ExtractPackageName(stmt, ignored);
}

} // anonymous namespace

// ===========================================================================
// 第一部分：核心调度与编排
// ===========================================================================

CGen::CGen(State *s) : s_(s) {
}

// 核心编译入口函数：为输入的 AST、推断结果及配置生成 C 代码
GenResult CGen::Generate(const ParseResult &pr, const InferResult &ir, const AnalysisResult &ar, const CompileConfig &cfg) {
    LOG_INFO("start CGen::Generate {}", pr.file_name);

    file_name_ = pr.file_name;
    ir_ = ir;
    ar_ = ar;

    // 运行构建主流程
    GenResult gr = Build(pr, cfg);

    // 如果开启了调试模式，将生成的 C 代码转储到临时文件中以供调试
    if (cfg.debug_mode) {
        const auto dumpfile = GenerateTmpFilename("fakelua_jit_", ".c");
        if (std::ofstream ofs(dumpfile); ofs.is_open()) {
            ofs << gr.c_code;
            ofs.close();
            std::cerr << "CGen::Generate: C code dumped to " << dumpfile << std::endl;
            LOG_INFO("C code generated: {}", dumpfile);
        } else {
            LOG_ERROR("Failed to open output file: {}", dumpfile);
        }
    }

    LOG_INFO("end CGen::Generate {}, functions: {}", pr.file_name, gr.function_names.size());
    return gr;
}

// 内部核心流水线：分别生成头文件、全局区、声明区和实现区代码，最后将它们拼接
GenResult CGen::Build(const ParseResult &pr, const CompileConfig &cfg) {
    GenResult gr;

    cur_package_name_.clear();
    if (pr.chunk && pr.chunk->Type() == SyntaxTreeType::Block) {
        const auto blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(pr.chunk);
        const std::vector<SyntaxTreeInterfacePtr> *stmts_to_check = &blk->Stmts();
        for (const auto &stmt : blk->Stmts()) {
            if (stmt && stmt->Type() == SyntaxTreeType::Function) {
                const auto func = std::dynamic_pointer_cast<SyntaxTreeFunction>(stmt);
                if (func && func->Funcname()) {
                    const auto fn = std::dynamic_pointer_cast<SyntaxTreeFuncname>(func->Funcname());
                    if (fn && fn->FuncNameList()) {
                        const auto fnl = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(fn->FuncNameList());
                        if (fnl && fnl->Funcnames().size() == 1 && fnl->Funcnames()[0] == kInitFunctionName) {
                            if (func->Funcbody()) {
                                const auto fbody = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(func->Funcbody());
                                if (fbody && fbody->Block()) {
                                    const auto init_blk = std::dynamic_pointer_cast<SyntaxTreeBlock>(fbody->Block());
                                    if (init_blk) {
                                        stmts_to_check = &init_blk->Stmts();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!stmts_to_check->empty()) {
            ExtractPackageName((*stmts_to_check)[0], cur_package_name_);
        }
    }

    // 0. Resolve lexical scopes and closure upvalues
    std::vector<Scope> scopes;
    std::vector<FuncInfo *> func_stack;
    ResolveScopes(pr.chunk, scopes, func_stack, nullptr);

    // 1. 生成 C 文件的头文件包含、基础结构体及宏定义
    GenerateHeader();

    // 2. 扫描 AST 并生成全局静态常量、全局变量的初始化代码
    GenerateGlobal(pr.chunk);

    // 3. 扫描 AST 树以搜集所有定义的函数，并为它们生成统一的 C 函数前置声明原型
    GenerateDecls(pr.chunk, gr);
    local_func_names_ = gr.function_names;

    // 4. 遍历生成所有函数特化与通用的具体 C 代码实现
    GenerateImpl(pr.chunk, gr);

    // 5. 根据配置记录生成的 C 核心区代码，或组合全部代码块输出最终结果
    if (cfg.record_c_code) {
        gr.recorded_c_code = GetSectionStr(Section::Globals) + GetSectionStr(Section::Decls) + GetSectionStr(Section::Impls);
    }
    gr.c_code = GetSectionStr(Section::Headers) + GetSectionStr(Section::Globals) + GetSectionStr(Section::Decls) + GetSectionStr(Section::Impls);
    return gr;
}

void CGen::EmitSpecTypeBoilerplate(const std::string &spec_type, const SpecTypeMetadata &meta) {
    // 发射 typedef + 特化 get/set 函数到 Headers section（每个 spec 类型仅一次）。
    SectionGuard sg(*this, Section::Headers);
    const auto get_fn = std::format("FlGetTableStrId_{}", spec_type);
    const auto set_fn = std::format("FlSetTableStrId_{}", spec_type);

    // typedef
    Out() << "typedef struct " << spec_type << " {\n";
    for (const auto &f: meta.fields) {
        Out() << "    CVar " << f.c_field_name << ";\n";
    }
    Out() << "} " << spec_type << ";\n\n";

    // get 函数：接受 CVar k + bool *finish，命中设 *finish=true
    Out() << "static CVar " << get_fn << "(VarTable *tbl, CVar k, bool *__finish) {\n";
    Out() << "    " << spec_type << " *s = (" << spec_type << " *)tbl->spec;\n";

    if (meta.has_string_keys) {
        Out() << "    if (LIKELY(k.type_ == VAR_STRINGID)) {\n";
        Out() << "        switch (k.data_.i) {\n";
        for (const auto &f: meta.fields) {
            if (f.key_kind == TableKeyKind::kString) {
                Out() << "            case " << s_->GetConstString().Alloc(f.key) << ": *__finish = true; return s->" << f.c_field_name << ";\n";
            }
        }
        Out() << "            default: break;\n";
        Out() << "        }\n";
        Out() << "    } else if (k.type_ == VAR_STRING) {\n";
        Out() << "        VarString *__vs = k.data_.s;\n";
        for (const auto &f: meta.fields) {
            if (f.key_kind == TableKeyKind::kString) {
                Out() << "        if (__vs->size_ == " << f.key.size() << " && memcmp(__vs->data_, \"" << f.key << "\", " << f.key.size() << ") == 0) { *__finish = true; return s->" << f.c_field_name << "; }\n";
            }
        }
        Out() << "    }\n";
    }

    if (meta.has_int_keys) {
        Out() << "    if (k.type_ == VAR_INT) {\n";
        Out() << "        switch (k.data_.i) {\n";
        for (const auto &f: meta.fields) {
            if (f.key_kind == TableKeyKind::kInt) {
                Out() << "            case " << f.int_value << ": *__finish = true; return s->" << f.c_field_name << ";\n";
            }
        }
        Out() << "            default: break;\n";
        Out() << "        }\n";
        Out() << "    } else if (k.type_ == VAR_FLOAT) {\n";
        Out() << "        double __fval = k.data_.f;\n";
        for (const auto &f: meta.fields) {
            if (f.key_kind == TableKeyKind::kInt) {
                Out() << "        if (__fval == (double)" << f.int_value << ") { *__finish = true; return s->" << f.c_field_name << "; }\n";
            }
        }
        Out() << "    }\n";
    }

    if (meta.has_float_keys) {
        Out() << "    if (k.type_ == VAR_FLOAT) {\n";
        Out() << "        double __fval = k.data_.f;\n";
        for (const auto &f: meta.fields) {
            if (f.key_kind == TableKeyKind::kFloat) {
                Out() << "        if (__fval == " << f.float_value << ") { *__finish = true; return s->" << f.c_field_name << "; }\n";
            }
        }
        Out() << "    } else if (k.type_ == VAR_INT) {\n";
        Out() << "        int64_t __ival = k.data_.i;\n";
        for (const auto &f: meta.fields) {
            if (f.key_kind == TableKeyKind::kFloat) {
                Out() << "        if ((double)__ival == " << f.float_value << ") { *__finish = true; return s->" << f.c_field_name << "; }\n";
            }
        }
        Out() << "    }\n";
    }

    if (meta.has_bool_keys) {
        Out() << "    if (k.type_ == VAR_BOOL) {\n";
        Out() << "        bool __bval = k.data_.b;\n";
        for (const auto &f: meta.fields) {
            if (f.key_kind == TableKeyKind::kBool) {
                Out() << "        if (__bval == " << (f.bool_value ? "true" : "false") << ") { *__finish = true; return s->" << f.c_field_name << "; }\n";
            }
        }
        Out() << "    }\n";
    }

    Out() << "    *__finish = false;\n";
    Out() << "    return (CVar){VAR_NIL};\n";
    Out() << "}\n\n";

    // set 函数：同理
    Out() << "static void " << set_fn << "(VarTable *tbl, CVar k, CVar v, bool *__finish) {\n";
    Out() << "    " << spec_type << " *s = (" << spec_type << " *)tbl->spec;\n";

    if (meta.has_string_keys) {
        Out() << "    if (LIKELY(k.type_ == VAR_STRINGID)) {\n";
        Out() << "        switch (k.data_.i) {\n";
        int f_idx = 0;
        for (const auto &f: meta.fields) {
            if (f.key_kind == TableKeyKind::kString) {
                Out() << "            case " << s_->GetConstString().Alloc(f.key) << ": s->" << f.c_field_name << " = v; tbl->spec_vals[" << f_idx << "] = v; tbl->spec_keys[" << f_idx << "] = k; *__finish = true; return;\n";
            }
            f_idx++;
        }
        Out() << "            default: break;\n";
        Out() << "        }\n";
        Out() << "    } else if (k.type_ == VAR_STRING) {\n";
        Out() << "        VarString *__vs = k.data_.s;\n";
        f_idx = 0;
        for (const auto &f: meta.fields) {
            if (f.key_kind == TableKeyKind::kString) {
                const auto id = s_->GetConstString().Alloc(f.key);
                Out() << "        if (__vs->size_ == " << f.key.size() << " && memcmp(__vs->data_, \"" << f.key << "\", " << f.key.size() << ") == 0) { s->" << f.c_field_name << " = v; tbl->spec_vals[" << f_idx << "] = v; tbl->spec_keys[" << f_idx << "] = (CVar){.type_ = VAR_STRINGID, .data_.i = " << id << "}; *__finish = true; return; }\n";
            }
            f_idx++;
        }
        Out() << "    }\n";
    }

    if (meta.has_int_keys) {
        Out() << "    if (k.type_ == VAR_INT) {\n";
        Out() << "        switch (k.data_.i) {\n";
        int f_idx = 0;
        for (const auto &f: meta.fields) {
            if (f.key_kind == TableKeyKind::kInt) {
                Out() << "            case " << f.int_value << ": s->" << f.c_field_name << " = v; tbl->spec_vals[" << f_idx << "] = v; tbl->spec_keys[" << f_idx << "] = k; *__finish = true; return;\n";
            }
            f_idx++;
        }
        Out() << "            default: break;\n";
        Out() << "        }\n";
        Out() << "    } else if (k.type_ == VAR_FLOAT) {\n";
        Out() << "        double __fval = k.data_.f;\n";
        f_idx = 0;
        for (const auto &f: meta.fields) {
            if (f.key_kind == TableKeyKind::kInt) {
                Out() << "        if (__fval == (double)" << f.int_value << ") { s->" << f.c_field_name << " = v; tbl->spec_vals[" << f_idx << "] = v; tbl->spec_keys[" << f_idx << "] = k; *__finish = true; return; }\n";
            }
            f_idx++;
        }
        Out() << "    }\n";
    }

    if (meta.has_float_keys) {
        Out() << "    if (k.type_ == VAR_FLOAT) {\n";
        Out() << "        double __fval = k.data_.f;\n";
        int f_idx = 0;
        for (const auto &f: meta.fields) {
            if (f.key_kind == TableKeyKind::kFloat) {
                Out() << "            if (__fval == " << f.float_value << ") { s->" << f.c_field_name << " = v; tbl->spec_vals[" << f_idx << "] = v; tbl->spec_keys[" << f_idx << "] = k; *__finish = true; return; }\n";
            }
            f_idx++;
        }
        Out() << "    } else if (k.type_ == VAR_INT) {\n";
        Out() << "        int64_t __ival = k.data_.i;\n";
        f_idx = 0;
        for (const auto &f: meta.fields) {
            if (f.key_kind == TableKeyKind::kFloat) {
                Out() << "        if ((double)__ival == " << f.float_value << ") { s->" << f.c_field_name << " = v; tbl->spec_vals[" << f_idx << "] = v; tbl->spec_keys[" << f_idx << "] = k; *__finish = true; return; }\n";
            }
            f_idx++;
        }
        Out() << "    }\n";
    }

    if (meta.has_bool_keys) {
        Out() << "    if (k.type_ == VAR_BOOL) {\n";
        Out() << "        bool __bval = k.data_.b;\n";
        int f_idx = 0;
        for (const auto &f: meta.fields) {
            if (f.key_kind == TableKeyKind::kBool) {
                Out() << "        if (__bval == " << (f.bool_value ? "true" : "false") << ") { s->" << f.c_field_name << " = v; tbl->spec_vals[" << f_idx << "] = v; tbl->spec_keys[" << f_idx << "] = k; *__finish = true; return; }\n";
            }
            f_idx++;
        }
        Out() << "    }\n";
    }

    Out() << "    *__finish = false;\n";
    Out() << "}\n\n";
}

void CGen::GenerateHeader() {
    SectionGuard sg(*this, Section::Headers);
    Out() << R"(// Generated by FakeLua CGen
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

)";

    // 硬编码 state 指针到生成的 C 代码中。
    // 生命周期保证：生成的代码最终由 TCCState/GCC 产物承载，这些产物通过 TCCHandle/JITHandle
    // 被 VmFunction 以 shared_ptr 持有，而 VmFunction 注册在本 State 的 Vm 中（见
    // src/jit/tcc_jit.cpp 与 src/jit/vm_function.h）。因此生成代码里的 _S 永远不会
    // 比其宿主 State 活得更久——State 析构即销毁 Vm，进而释放 handle 和代码页。
    // 同一份代码也不会被跨 State 复用（每次编译都会重新生成）。
    Out() << "static void * _S = (void *) " << s_ << ";\n";
    Out() << "static bool __fakelua_init_flag__ = false;\n";

    // C 运行时类型定义、宏 and 函数（从 c_runtime_header.h 提取，便于独立维护）
    Out() << kCRuntimeHeader;

    // 前置发射所有 spec 类型的 typedef + 特化 get/set 函数（读取 TypeInferencer 预计算的
    // ir.spec_type_metadata）。CGen 不再在 CompileTableconstructor 里懒发射这些样板代码。
    for (const auto &[spec_type, meta]: ir().spec_type_metadata) {
        EmitSpecTypeBoilerplate(spec_type, meta);
    }
}

void CGen::GenerateGlobal(const SyntaxTreeInterfacePtr &chunk) {
    SectionGuard sg(*this, Section::Globals);
    Out() << "// ===== Global Variables =====\n\n";

    // 静态全局的CVar
    // 定义初始化器宏
    Out() << "static const CVar kNil = (CVar){.type_ = VAR_NIL};\n";
    Out() << "static const CVar kTrue = (CVar){.type_ = VAR_BOOL, .data_.b = true};\n";
    Out() << "static const CVar kFalse = (CVar){.type_ = VAR_BOOL, .data_.b = false};\n";

    // 遍历顶层的 local 变量定义，生成全局常量
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);

    for (const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(chunk); const auto &stmt: block->Stmts()) {
        if (stmt->Type() == SyntaxTreeType::LocalVar) {
            const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
            const auto namelist = local_var->Namelist();
            const auto explist = local_var->Explist();

            if (!namelist) {
                continue;
            }

            DEBUG_ASSERT(namelist->Type() == SyntaxTreeType::NameList);

            const auto namelist_ptr = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist);
            const auto &names = namelist_ptr->Names();

            static const std::vector<SyntaxTreeInterfacePtr> empty_exps;
            const auto explist_ptr = explist ? std::dynamic_pointer_cast<SyntaxTreeExplist>(explist) : nullptr;
            const auto &exps = explist_ptr ? explist_ptr->Exps() : empty_exps;

            for (size_t i = 0; i < names.size(); ++i) {
                const auto &name = names[i];
                SyntaxTreeInterfacePtr exp = (i < exps.size()) ? exps[i] : nullptr;

                InferredType global_type = ir().global_const_vars.at(name);
                const auto exp_node = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
                if (global_type == T_INT) {
                    if (!exp_node || exp_node->GetExpKind() == ExpKind::kNil) {
                        Out() << "static const int64_t " << name << " = 0;\n";
                    } else {
                        Out() << "static const int64_t " << name << " = " << CompileNumericExp(exp) << ";\n";
                    }
                } else if (global_type == T_FLOAT) {
                    if (!exp_node || exp_node->GetExpKind() == ExpKind::kNil) {
                        Out() << "static const double " << name << " = 0.0;\n";
                    } else {
                        Out() << "static const double " << name << " = " << CompileNumericExp(exp) << ";\n";
                    }
                } else {
                    // 非数值字面量：保留 static CVar 形式。
                    const std::string cvar_init = exp ? CompileExp(exp) : "(CVar){.type_ = VAR_NIL}";
                    Out() << "static CVar " << name << " = " << cvar_init << ";\n";
                }
            }
        }
    }

    Out() << "\n";
}

void CGen::GenerateDecls(const SyntaxTreeInterfacePtr &chunk, GenResult &gr) {
    SectionGuard sg(*this, Section::Decls);
    Out() << "\n// ===== Function Declarations =====\n\n";

    for (const auto &func : all_funcs_) {
        const std::string &name = func->unique_c_name;
        const auto &params = func->params;

        // 生成带有 VarClosure *_CL 的函数声明
        Out() << "CVar " << name << "(VarClosure *_CL";
        for (size_t i = 0; i < params.size(); ++i) {
            Out() << ", CVar " << params[i];
        }
        Out() << ");\n";

        bool is_vararg = func->is_vararg;
        gr.function_names[name] = JitFunctionInfo{static_cast<int>(params.size()), is_vararg, name};
        if (!cur_package_name_.empty() && func->parent == nullptr && func->unique_c_name != kInitFunctionName && !func->name.empty()) {
            std::string pkg_func_name = cur_package_name_ + "." + func->name;
            gr.function_names[pkg_func_name] = JitFunctionInfo{static_cast<int>(params.size()), is_vararg, name};
        }

        // 如果原始函数含有数学参数，声明其特化变体
        if (const auto math_it = ir().math_param_positions.find(func->name); math_it != ir().math_param_positions.end()) {
            const auto &math_params = math_it->second;
            const int num_specs = 1 << static_cast<int>(math_params.size());
            for (int bitmask = 0; bitmask < num_specs; ++bitmask) {
                const auto spec_name = SpecFuncName(name, math_params, bitmask);
                const auto spec_ret = GetSpecReturnType(func->name, bitmask);
                Out() << SpecReturnCTypeName(spec_ret) << " " << spec_name << "(";
                EmitSpecParamList(params, math_params, bitmask);
                Out() << ");\n";
                // 注册特化函数名，使 CompileFunctioncall 能将其识别为
                // 本地调用（同文件直接调用）。
                gr.function_names[spec_name] = JitFunctionInfo{static_cast<int>(params.size()), is_vararg};
            }
        }
    }

    Out() << "\n";
}

std::string CGen::CompileFuncName(const SyntaxTreeInterfacePtr &ptr) {
    DEBUG_ASSERT(ptr->Type() == SyntaxTreeType::FuncName);

    const auto name = std::dynamic_pointer_cast<SyntaxTreeFuncname>(ptr);
    const auto funcnamelistptr = name->FuncNameList();

    DEBUG_ASSERT(funcnamelistptr->Type() == SyntaxTreeType::FuncNameList);
    const auto funcnamelist = std::dynamic_pointer_cast<SyntaxTreeFuncnamelist>(funcnamelistptr);
    const auto &namelist = funcnamelist->Funcnames();

    DEBUG_ASSERT(namelist.size() == 1);

    DEBUG_ASSERT(name->ColonName().empty());

    return namelist[0];
}

[[noreturn]] void CGen::ThrowError(const std::string &msg, const SyntaxTreeInterfacePtr &ptr) {
    ThrowFakeluaException(std::format("Code generate failed, {} at {}:{}:{}", msg, file_name_, ptr->Loc().begin.line, ptr->Loc().begin.column));
}

bool CGen::BlockEndsWithReturn(const SyntaxTreeInterfacePtr &block) {
    const auto block_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(block);
    const auto &stmts = block_ptr->Stmts();
    if (stmts.empty()) {
        return false;
    }
    return stmts.back()->Type() == SyntaxTreeType::Return;
}

std::vector<std::string> CGen::CompileParList(const SyntaxTreeInterfacePtr &parlist) {
    DEBUG_ASSERT(parlist->Type() == SyntaxTreeType::ParList);
    const auto parlist_ptr = std::dynamic_pointer_cast<SyntaxTreeParlist>(parlist);

    // PreProcessor 已确保不存在变长参数
    DEBUG_ASSERT(!parlist_ptr->VarParams());

    const auto namelist = parlist_ptr->Namelist();
    DEBUG_ASSERT(namelist);
    DEBUG_ASSERT(namelist->Type() == SyntaxTreeType::NameList);
    const auto namelist_ptr = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist);
    auto &param_names = namelist_ptr->Names();

    std::set<std::string> param_names_set;
    for (const auto &key: ar().global_const_names) {
        param_names_set.insert(key);
    }

    for (auto &name: param_names) {
        if (param_names_set.contains(name)) {
            ThrowError("the param name is duplicated: " + name, namelist_ptr);
        }
        param_names_set.insert(name);
    }

    return param_names;
}

void CGen::GenerateImpl(const SyntaxTreeInterfacePtr &chunk, GenResult &gr) {
    SectionGuard sg(*this, Section::Impls);
    DEBUG_ASSERT(chunk->Type() == SyntaxTreeType::Block);

    auto compile_func = [this](FuncInfo *func) {
        const std::string &name = func->unique_c_name;
        SyntaxTreeInterfacePtr funcbody = func->funcbody;

        if (!funcbody) {
            return;
        }

        FuncInfo *prev_func_info = cur_func_info_;
        cur_func_info_ = func;

        DEBUG_ASSERT(funcbody->Type() == SyntaxTreeType::FuncBody);
        const auto funcbody_ptr = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
        const auto parlist = funcbody_ptr->Parlist();
        std::vector<std::string> func_params;
        if (parlist) {
            func_params = CompileParList(parlist);
        }

        const auto func_block = funcbody_ptr->Block();
        if (const auto math_it = ir().math_param_positions.find(func->name); math_it != ir().math_param_positions.end()) {
            const auto &math_params = math_it->second;
            const int num_specs = 1 << static_cast<int>(math_params.size());
            for (int bitmask = 0; bitmask < num_specs; ++bitmask) {
                const auto spec_name = SpecFuncName(name, math_params, bitmask);
                const auto spec_ret = GetSpecReturnType(func->name, bitmask);
                Out() << SpecReturnCTypeName(spec_ret) << " " << spec_name << "(";
                EmitSpecParamList(func_params, math_params, bitmask);
                Out() << ") {\n";
                CompileFuncBody(func->name, func_block, bitmask, sections_[static_cast<size_t>(Section::Impls)]);
                if (!BlockEndsWithReturn(func_block)) {
                    if (spec_ret == T_INT) {
                        Out() << "    return 0;\n";
                    } else if (spec_ret == T_FLOAT) {
                        Out() << "    return 0.0;\n";
                    } else {
                        Out() << "    return kNil;\n";
                    }
                }
                Out() << "}\n";
            }
            GenerateEntryDispatcher(name, func_params, math_params);
        } else {
            Out() << "CVar " << name << "(VarClosure *_CL";
            for (size_t i = 0; i < func_params.size(); ++i) {
                Out() << ", CVar " << func_params[i];
            }
            Out() << ") {\n";
            CompileFuncBody(func->name, func_block, -1, sections_[static_cast<size_t>(Section::Impls)]);
            if (!BlockEndsWithReturn(func_block)) {
                Out() << "    return kNil;\n";
            }
            Out() << "}\n";
        }

        cur_func_info_ = prev_func_info;
    };

    FuncInfo *init_func = nullptr;
    for (const auto &func : all_funcs_) {
        if (func->unique_c_name == kInitFunctionName) {
            init_func = func.get();
            break;
        }
    }
    if (init_func) {
        compile_func(init_func);
    }
    for (const auto &func : all_funcs_) {
        if (func.get() == init_func) continue;
        compile_func(func.get());
    }
}

void CGen::GenerateEntryDispatcher(const std::string &func_name, const std::vector<std::string> &func_params, const std::vector<int> &math_param_indices) {
    const int k = static_cast<int>(math_param_indices.size());
    const int num_specs = 1 << k;

    Out() << "CVar " << func_name << "(VarClosure *_CL";
    for (size_t i = 0; i < func_params.size(); ++i) {
        Out() << ", CVar " << func_params[i];
    }
    Out() << ") {\n";

    for (int i = 0; i < k; ++i) {
        const int param_idx = math_param_indices[i];
        const auto &mp_name = func_params[static_cast<size_t>(param_idx)];
        Out() << std::format("    if (UNLIKELY({0}.type_ != VAR_INT && {0}.type_ != VAR_FLOAT)) {{ FakeluaThrowError(_S, \"bad argument #{1} ({2}): "
                             "attempt to perform arithmetic on non-numeric value\"); }}\n",
                             mp_name, param_idx + 1, mp_name);
    }

    Out() << "    int flua_spec_idx = ";
    for (int i = 0; i < k; ++i) {
        if (i > 0) {
            Out() << " | ";
        }
        const auto &mp_name = func_params[static_cast<size_t>(math_param_indices[i])];
        if (i == 0) {
            Out() << std::format("({}.type_ == VAR_FLOAT ? {} : {})", mp_name, static_cast<int>(kMathParamFloat), static_cast<int>(kMathParamInt));
        } else {
            Out() << std::format("(({}.type_ == VAR_FLOAT ? {} : {}) << {})", mp_name, static_cast<int>(kMathParamFloat), static_cast<int>(kMathParamInt), i);
        }
    }
    Out() << ";\n";

    Out() << "    switch (flua_spec_idx) {\n";
    for (int bitmask = 0; bitmask < num_specs; ++bitmask) {
        const auto spec_name = SpecFuncName(func_name, math_param_indices, bitmask);

        std::string args_str;
        for (size_t i = 0; i < func_params.size(); ++i) {
            if (i > 0) {
                args_str += ", ";
            }
            if (const auto mp_it = std::ranges::find(math_param_indices, static_cast<int>(i)); mp_it != math_param_indices.end()) {
                const int mp_idx = static_cast<int>(mp_it - math_param_indices.begin());
                const auto kind = MathParamKindOf(bitmask, mp_idx);
                args_str += func_params[i] + (kind == kMathParamFloat ? ".data_.f" : ".data_.i");
            } else {
                args_str += func_params[i];
            }
        }

        if (const auto spec_ret = GetSpecReturnType(func_name, bitmask); spec_ret == T_INT || spec_ret == T_FLOAT) {
            const auto native_tmp = std::format("flua_r_{}", bitmask);
            Out() << std::format("        case {}: {{ {} {} = {}({}); return {}; }}\n", bitmask, SpecReturnCTypeName(spec_ret), native_tmp, spec_name, args_str, BoxNativeValue(native_tmp, spec_ret));
        } else {
            Out() << std::format("        case {}: return {}({});\n", bitmask, spec_name, args_str);
        }
    }
    Out() << "    }\n";
    Out() << "    return kNil;\n";
    Out() << "}\n";
}

std::string CGen::GenTab() const {
    const auto tab_size = static_cast<size_t>(cur_tab_) * 4;
    std::string tabs(tab_size, ' ');
    return tabs;
}

InferredType CGen::LookupNodeType(SyntaxTreeInterface *node) const {
    if (cur_spec_ctx_ && cur_spec_ctx_->snapshot) {
        if (const auto it = cur_spec_ctx_->snapshot->find(node); it != cur_spec_ctx_->snapshot->end()) {
            return it->second;
        }
    }
    if (const auto it = ir().main_eval_types.find(node); it != ir().main_eval_types.end()) {
        return it->second;
    }
    return T_UNKNOWN;
}

InferredType CGen::GetSpecReturnType(const std::string &func_name, int bitmask) const {
    const auto it = ir().specialization_return_types.find(func_name);
    DEBUG_ASSERT(it != ir().specialization_return_types.end());
    DEBUG_ASSERT(bitmask >= 0 && bitmask < static_cast<int>(it->second.size()));
    return it->second[static_cast<size_t>(bitmask)];
}

std::string CGen::GetKeyDescriptor(const std::string &key, TableKeyKind kind) {
    if (kind == TableKeyKind::kString) return "S_" + key;
    if (kind == TableKeyKind::kInt) return "I_" + key;
    if (kind == TableKeyKind::kBool) return "B_" + key;
    return "F_" + key;
}



std::string CGen::GetSpecTypeForVar(const SyntaxTreeInterfacePtr &pe) const {
    // 从 TypeInferencer 标注的 ir.var_spec_annotations 中获取流敏感 spec 类型名。
    if (const auto it = ir().var_spec_annotations.find(pe.get()); it != ir().var_spec_annotations.end()) {
        return it->second;
    }
    return "";
}

bool CGen::IsSpecField(const std::string &spec_type, const std::string &key, TableKeyKind kind) const {
    const auto it = ir().spec_type_metadata.find(spec_type);
    if (it == ir().spec_type_metadata.end()) return false;
    return it->second.field_key_descs.contains(GetKeyDescriptor(key, kind));
}

std::string CGen::GetSpecFieldCName(const std::string &spec_type, const std::string &key, TableKeyKind kind) const {
    const auto it = ir().spec_type_metadata.find(spec_type);
    if (it == ir().spec_type_metadata.end()) return "";
    const auto fit = it->second.c_field_names.find(GetKeyDescriptor(key, kind));
    return fit == it->second.c_field_names.end() ? "" : fit->second;
}

int CGen::GetSpecFieldIndex(const std::string &spec_type, const std::string &key, TableKeyKind kind) const {
    const auto it = ir().spec_type_metadata.find(spec_type);
    if (it == ir().spec_type_metadata.end()) return -1;
    const auto fit = it->second.field_indices.find(GetKeyDescriptor(key, kind));
    return fit == it->second.field_indices.end() ? -1 : fit->second;
}

InferredType CGen::GetSpecFieldType(const std::string &spec_type, const std::string &key, TableKeyKind kind) const {
    const auto it = ir().spec_type_metadata.find(spec_type);
    if (it == ir().spec_type_metadata.end()) return T_UNKNOWN;
    const auto fit = it->second.field_types.find(GetKeyDescriptor(key, kind));
    return fit == it->second.field_types.end() ? T_UNKNOWN : fit->second;
}

void CGen::CompileFuncBody(const std::string &func_name, const SyntaxTreeInterfacePtr &func_block, int spec_bitmask, std::ostream &out) {
    SectionGuard section_guard(*this, Section::Body);
    // 初始化特化上下文：从 ir.spec_func_context 查得当前版本的 snapshot/func_name/bitmask。
    const SpecFuncContext *ctx = nullptr;
    if (spec_bitmask >= 0) {
        if (const auto it = ir().spec_func_context.find(func_name); it != ir().spec_func_context.end()) {
            const auto &ctx_vec = it->second;
            if (static_cast<size_t>(spec_bitmask) < ctx_vec.size()) {
                ctx = &ctx_vec[static_cast<size_t>(spec_bitmask)];
            }
        }
    }
    cur_spec_ctx_ = ctx;

    // 将函数体编译到 body 缓冲区。
    func_temp_decls_.str("");
    func_temp_decls_.clear();
    auto &body_ss = sections_[static_cast<size_t>(Section::Body)];
    body_ss.str("");
    body_ss.clear();

    // Box captured parameters at the start of the function
    if (cur_func_info_) {
        const SyntaxTreeInterface *parlist_ptr = nullptr;
        SyntaxTreeInterfacePtr funcbody = cur_func_info_->funcbody;
        if (funcbody) {
            const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
            if (fb->Parlist()) parlist_ptr = fb->Parlist().get();
        }
        
        if (parlist_ptr) {
            for (const auto &pname : cur_func_info_->params) {
                bool is_captured = false;
                if (const auto it = stmt_var_to_def_.find({parlist_ptr, pname}); it != stmt_var_to_def_.end()) {
                    is_captured = it->second->is_captured;
                }
                if (is_captured) {
                    func_temp_decls_ << "    CVar *__box_" << pname << " = (CVar *)FakeluaAlloc(_S, sizeof(CVar), false);\n";
                    if (cur_spec_ctx_) {
                        if (const auto pit = cur_spec_ctx_->param_types.find(pname); pit != cur_spec_ctx_->param_types.end()) {
                            if (pit->second == T_INT) {
                                func_temp_decls_ << std::format("    *__box_{0} = (CVar){{.type_ = VAR_INT, .data_.i = {0}}};\n", pname);
                                continue;
                            } else if (pit->second == T_FLOAT) {
                                func_temp_decls_ << std::format("    *__box_{0} = (CVar){{.type_ = VAR_FLOAT, .data_.f = {0}}};\n", pname);
                                continue;
                            }
                        }
                    }
                    func_temp_decls_ << "    *__box_" << pname << " = " << pname << ";\n";
                }
            }
        }
    }
    cur_tab_++;
    CompileStmtBlock(func_block);
    cur_tab_--;

    // 写入调用方提供的输出流。
    out << func_temp_decls_.str();
    out << body_ss.str();
    if (func_name == kInitFunctionName) {
        out << "    __fakelua_init_flag__ = true;\n";
    }

    // 清除特化上下文。
    cur_spec_ctx_ = nullptr;
    // section_guard 析构时自动恢复 cur_section_。
}

bool CGen::TryInferMathCallBitmask(const std::string &callee_name, const std::vector<SyntaxTreeInterfacePtr> &raw_args, int &bitmask) const {
    if (const auto math_it = ir().math_param_positions.find(callee_name); math_it != ir().math_param_positions.end()) {
        const auto &math_params = math_it->second;
        bitmask = 0;
        for (int i = 0; i < static_cast<int>(math_params.size()); ++i) {
            const int param_pos = math_params[i];
            if (param_pos >= static_cast<int>(raw_args.size())) {
                return false;// fewer args than math params: let slow-path ThrowError handle it
            }
            const auto &arg = raw_args[static_cast<size_t>(param_pos)];
            DEBUG_ASSERT(arg && arg->Type() == SyntaxTreeType::Exp);
            const auto arg_type = GetType(arg);
            if (arg_type == T_DYNAMIC) {
                return false;
            }
            if (arg_type == T_FLOAT) {
                bitmask |= (1 << i);
            }
        }
        return true;
    }
    ThrowFakeluaException("callee_name should be a math function");
}

bool CGen::TryInferMathCallSpec(const std::string &callee_name, const std::vector<SyntaxTreeInterfacePtr> &raw_args, int &bitmask, InferredType &spec_ret) const {
    [[maybe_unused]] bool ok = TryInferMathCallBitmask(callee_name, raw_args, bitmask);
    DEBUG_ASSERT(ok);
    spec_ret = GetSpecReturnType(callee_name, bitmask);
    return true;
}

InferredType CGen::GetType(const SyntaxTreeInterfacePtr &exp) const {
    if (!exp || exp->Type() != SyntaxTreeType::Exp) {
        return T_DYNAMIC;
    }
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    DEBUG_ASSERT(e);
    // For simple variables, the snapshot can be stale: CGen may degrade a
    // variable to CVar after TypeInferencer's single pass (e.g. assigned both
    // an int and a string in different branches). So we rely solely on CGen's
    // local scope here and fall back to T_DYNAMIC, never the snapshot.
    if (e->GetExpKind() == ExpKind::kPrefixExp) {
        const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e->Right());
        if (pe && pe->GetPrefixKind() == PrefixExpKind::kVar) {
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue());
            if (var && var->GetVarKind() == VarKind::kSimple) {
                const auto &name = var->GetName();
                 if (const auto native_type = GetNativeVarType(name, var.get()); native_type == T_INT || native_type == T_FLOAT) {
                     return native_type;
                 }
                if (const auto git = ir().global_const_vars.find(name); git != ir().global_const_vars.end()) {
                    if (git->second == T_INT || git->second == T_FLOAT) {
                        return git->second;
                    }
                }
                return T_DYNAMIC;
            }
        }
    }
    // All other expressions: use the type pre-computed by TypeInferencer.
    if (const auto t = LookupNodeType(exp.get()); t != T_UNKNOWN) {
        return t;
    }
    return T_DYNAMIC;
}

// 尝试将表达式编译为高效的原生 C 数值运算表达式。若中途由于类型不匹配等抛出异常，则优雅捕获并返回空字符串（指示回退到动态分发计算）
std::string CGen::TryCompileNativeExpr(const SyntaxTreeInterfacePtr &exp) {
    try {
        return CompileNumericExp(exp);
    } catch (...) {
        return {};
    }
}

// 将比较运算符映射到对应的 C 操作符。
static const std::unordered_map<BinOpKind, std::string_view> kCmpOpMap = {{BinOpKind::kLess, "<"},       {BinOpKind::kLessEqual, "<="}, {BinOpKind::kMore, ">"},
                                                                          {BinOpKind::kMoreEqual, ">="}, {BinOpKind::kEqual, "=="},     {BinOpKind::kNotEqual, "!="}};

std::string CGen::TryCompileNativeBoolExpr(const SyntaxTreeInterfacePtr &exp) {
    // 只处理 Exp 节点。
    DEBUG_ASSERT(exp && exp->Type() == SyntaxTreeType::Exp);
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);

    // 透明地解包括括号表达式：(expr) → expr。
    if (e->GetExpKind() == ExpKind::kPrefixExp) {
        const auto pexp = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e->Right());
        if (!pexp || pexp->GetPrefixKind() != PrefixExpKind::kExp) {
            return {};
        }
        return TryCompileNativeBoolExpr(pexp->GetValue());
    }

    // 处理 not 一元逻辑取反：将 not <bool_expr> 编译为 !(<bool_expr>)。
    if (e->GetExpKind() == ExpKind::kUnop) {
        if (const auto unop = std::dynamic_pointer_cast<SyntaxTreeUnop>(e->Op()); !unop || unop->GetOpKind() != UnOpKind::kNot) {
            return {};
        }
        const auto inner = TryCompileNativeBoolExpr(e->Right());
        if (inner.empty()) {
            return {};
        }
        return std::format("!({})", inner);
    }

    if (e->GetExpKind() != ExpKind::kBinop) {
        return {};
    }
    const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(e->Op());
    DEBUG_ASSERT(op);
    const auto op_kind = op->GetOpKind();

    // 处理 and/or 逻辑运算符：递归将两侧编译为原生布尔表达式。
    if (op_kind == BinOpKind::kAnd || op_kind == BinOpKind::kOr) {
        const auto left_bool = TryCompileNativeBoolExpr(e->Left());
        const auto right_bool = TryCompileNativeBoolExpr(e->Right());
        if (left_bool.empty() || right_bool.empty()) {
            return {};
        }
        const auto c_op = (op_kind == BinOpKind::kAnd) ? "&&" : "||";
        return std::format("({}) {} ({})", left_bool, c_op, right_bool);
    }

    if (const auto op_it = kCmpOpMap.find(op_kind); op_it != kCmpOpMap.end()) {
        const auto left_type = e->Left() ? GetType(e->Left()) : T_DYNAMIC;
        if (const auto right_type = e->Right() ? GetType(e->Right()) : T_DYNAMIC; (left_type != T_INT && left_type != T_FLOAT) || (right_type != T_INT && right_type != T_FLOAT)) {
            return {};
        }
        const auto left_native = TryCompileNativeExpr(e->Left());
        const auto right_native = TryCompileNativeExpr(e->Right());
        DEBUG_ASSERT(!left_native.empty() && !right_native.empty());
        return std::format("({}) {} ({})", left_native, op_it->second, right_native);
    }
    return {};
}

void CGen::EmitSpecParamList(const std::vector<std::string> &params, const std::vector<int> &math_params, int bitmask) {
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) {
            Out() << ", ";
        }
        if (const auto mp_it = std::ranges::find(math_params, static_cast<int>(i)); mp_it != math_params.end()) {
            const int mp_idx = static_cast<int>(mp_it - math_params.begin());
            Out() << MathParamCTypeName(MathParamKindOf(bitmask, mp_idx)) << " " << params[i];
        } else {
            Out() << "CVar " << params[i];
        }
    }
}

// ===========================================================================
// 第二部分：语句编译
// ===========================================================================

void CGen::CompileStmtBlock(const SyntaxTreeInterfacePtr &block) {
    DEBUG_ASSERT(block->Type() == SyntaxTreeType::Block);
    const auto block_ptr = std::dynamic_pointer_cast<SyntaxTreeBlock>(block);

    for (const auto &stmts = block_ptr->Stmts(); auto &stmt: stmts) {
        CompileStmt(stmt);
    }
}

bool CGen::IsPackageHeaderStmt(const SyntaxTreeInterfacePtr &stmt) const {
    if (cur_package_name_.empty()) return false;
    return IsPackageDeclStmt(stmt);
}

void CGen::CompileStmt(const SyntaxTreeInterfacePtr &stmt) {
    if (IsPackageHeaderStmt(stmt)) {
        return;
    }
    switch (stmt->Type()) {
        case SyntaxTreeType::Return:
            CompileStmtReturn(stmt);
            break;
        case SyntaxTreeType::LocalVar:
            CompileStmtLocalVar(stmt);
            break;
        case SyntaxTreeType::Assign:
            CompileStmtAssign(stmt);
            break;
        case SyntaxTreeType::FunctionCall:
            CompileFunctioncall(stmt);
            break;
        case SyntaxTreeType::Block:
            Out() << GenTab() << "{\n";
            cur_tab_++;
            CompileStmtBlock(stmt);
            cur_tab_--;
            Out() << GenTab() << "}\n";
            break;
        case SyntaxTreeType::While:
            CompileStmtWhile(stmt);
            break;
        case SyntaxTreeType::Repeat:
            CompileStmtRepeat(stmt);
            break;
        case SyntaxTreeType::If:
            CompileStmtIf(stmt);
            break;
        case SyntaxTreeType::Break:
            CompileStmtBreak(stmt);
            break;
        case SyntaxTreeType::ForLoop:
            CompileStmtForLoop(stmt);
            break;
        case SyntaxTreeType::ForIn:
            CompileStmtForIn(stmt);
            break;
        case SyntaxTreeType::LocalFunction:
            CompileStmtLocalFunction(stmt);
            break;
        case SyntaxTreeType::Empty:
            break;
        case SyntaxTreeType::Goto:
            CompileStmtGoto(stmt);
            break;
        case SyntaxTreeType::Label:
            CompileStmtLabel(stmt);
            break;
        default:
            ThrowError(std::format("not support stmt type: {}", SyntaxTreeTypeToString(stmt->Type())), stmt);
    }
}

void CGen::CompileStmtReturn(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::Return);
    const auto return_stmt = std::dynamic_pointer_cast<SyntaxTreeReturn>(stmt);

    auto explist = return_stmt->Explist();
    if (!explist) {
        // 默认返回 nil
        explist = std::make_shared<SyntaxTreeExplist>(return_stmt->Loc());
        const auto exp = std::make_shared<SyntaxTreeExp>(return_stmt->Loc());
        exp->SetExpKind(ExpKind::kNil);
        std::dynamic_pointer_cast<SyntaxTreeExplist>(explist)->AddExp(exp);
    }

    const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
    DEBUG_ASSERT(!explist_ptr->Exps().empty());

    if (explist_ptr->Exps().size() == 1) {
        const auto exp = explist_ptr->Exps()[0];
        // 若当前处于原生返回类型的特化函数中，直接将返回表达式编译为原生数值并返回，
        // 跳过 CompileExp 的装箱步骤，消除一次 CVar 封箱拆箱开销。
        if (cur_spec_ctx_) {
            if (const auto spec_ret = GetSpecReturnType(cur_spec_ctx_->func_name, cur_spec_ctx_->bitmask); spec_ret == T_INT || spec_ret == T_FLOAT) {
                const auto native_ret = CompileNumericExp(exp);
                Out() << GenTab() << "return " << native_ret << ";\n";
                return;
            }
        }
        // 始终通过 CompileExp 编译返回表达式。
        const std::string ret = CompileExp(exp, true);
        Out() << GenTab() << "return " << ret << ";\n";
    } else {
        const auto &exps = explist_ptr->Exps();
        bool last_is_func = ar().function_call_exps.contains(exps.back().get());
        bool last_is_vararg = IsVarargExp(exps.back());
        std::string last_callee = last_is_func ? ar().callee_names.at(exps.back().get()) : "";
        bool is_last_single_return_local = !last_callee.empty() && ar().function_max_returns.contains(last_callee) && ar().function_max_returns.at(last_callee) == 1;

        if ((last_is_func && !is_last_single_return_local) || last_is_vararg) {
            std::vector<std::string> prefix_args;
            for (size_t i = 0; i < exps.size() - 1; ++i) {
                prefix_args.push_back(CompileExp(exps[i], false));
            }
            std::string last_arg = CompileExp(exps.back(), true);
            std::string prefix_arr_name = std::format("flua_ret_prefix_{}", tmp_var_counter_++);
            func_temp_decls_ << "    CVar " << prefix_arr_name << "[" << prefix_args.size() << "];\n";
            for (size_t i = 0; i < prefix_args.size(); ++i) {
                Out() << GenTab() << prefix_arr_name << "[" << i << "] = " << prefix_args[i] << ";\n";
            }
            Out() << GenTab() << "return FlCombineMulti(_S, " << prefix_args.size() << ", " << prefix_arr_name << ", " << last_arg << ");\n";
        } else {
            std::string call = std::format("FlMakeMulti(_S, {}", exps.size());
            for (const auto &exp: exps) {
                call += ", " + CompileExp(exp, false);
            }
            call += ")";
            Out() << GenTab() << "return " << call << ";\n";
        }
    }
}

void CGen::CompileStmtLocalVar(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::LocalVar);
    const auto local_var = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(stmt);
    const auto namelist = local_var->Namelist();

    DEBUG_ASSERT(namelist);
    DEBUG_ASSERT(namelist->Type() == SyntaxTreeType::NameList);
    const auto namelist_ptr = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist);
    const auto &names = namelist_ptr->Names();

    static const std::vector<SyntaxTreeInterfacePtr> empty_exps;
    const auto explist = local_var->Explist();
    const auto &exps = explist ? std::dynamic_pointer_cast<SyntaxTreeExplist>(explist)->Exps() : empty_exps;

    bool last_is_func = !exps.empty() && ar().function_call_exps.contains(exps.back().get());
    bool last_is_vararg = !exps.empty() && IsVarargExp(exps.back());
    std::string callee = last_is_func ? ar().callee_names.at(exps.back().get()) : "";
    bool is_single_return_local = !callee.empty() && ar().function_max_returns.contains(callee) && ar().function_max_returns.at(callee) == 1;

    if (names.size() > exps.size() && ((last_is_func && !is_single_return_local) || last_is_vararg)) {
        // Compile prior expressions first (M - 1 expressions)
        for (size_t i = 0; i < exps.size() - 1; ++i) {
            const auto &name = names[i];
            if (ar().global_const_names.contains(name)) {
                ThrowError("local variable conflicts with global constant: " + name, stmt);
            }
            bool is_captured = false;
            if (const auto it = stmt_var_to_def_.find({stmt.get(), name}); it != stmt_var_to_def_.end()) {
                is_captured = it->second->is_captured;
            }
            if (is_captured) {
                const std::string init = CompileExp(exps[i]);
                Out() << GenTab() << "CVar *__box_" << name << " = (CVar *)FakeluaAlloc(_S, sizeof(CVar), false);\n";
                Out() << GenTab() << "*__box_" << name << " = " << init << ";\n";
                continue;
            }
            // All prior expressions map one-to-one to variables
            const auto type = LookupNodeType(exps[i].get());
            if (type == T_INT || type == T_FLOAT) {
                const auto native_expr = CompileNumericExp(exps[i]);
                const std::string type_str = (type == T_INT) ? "int64_t" : "double";
                if (ir().shadowed_decls.contains({stmt.get(), name})) {
                    const auto tmp = std::format("flua_local_{}", tmp_var_counter_++);
                    func_temp_decls_ << "    " << type_str << " " << tmp << ";\n";
                    Out() << GenTab() << tmp << " = " << native_expr << ";\n";
                    Out() << GenTab() << type_str << " " << name << " = " << tmp << ";\n";
                } else {
                    Out() << GenTab() << type_str << " " << name << " = " << native_expr << ";\n";
                }
            } else {
                const std::string init = CompileExp(exps[i]);
                Out() << GenTab() << "CVar " << name << " = " << init << ";\n";
            }
        }

        // Compile the last function call and store in a temporary variable
        const std::string call_expr = CompileExp(exps.back(), true);
        const auto tmp_res = std::format("flua_call_res_{}", tmp_var_counter_++);
        func_temp_decls_ << "    CVar " << tmp_res << ";\n";
        Out() << GenTab() << tmp_res << " = " << call_expr << ";\n";

        // Assign unboxed values to remaining variables
        for (size_t i = exps.size() - 1; i < names.size(); ++i) {
            const auto &name = names[i];
            if (ar().global_const_names.contains(name)) {
                ThrowError("local variable conflicts with global constant: " + name, stmt);
            }
            bool is_captured = false;
            if (const auto it = stmt_var_to_def_.find({stmt.get(), name}); it != stmt_var_to_def_.end()) {
                is_captured = it->second->is_captured;
            }
            if (is_captured) {
                Out() << GenTab() << "CVar *__box_" << name << " = (CVar *)FakeluaAlloc(_S, sizeof(CVar), false);\n";
                Out() << GenTab() << "*__box_" << name << " = FlUnboxMulti(" << tmp_res << ", " << (i - (exps.size() - 1)) << ");\n";
            } else {
                Out() << GenTab() << "CVar " << name << " = FlUnboxMulti(" << tmp_res << ", " << (i - (exps.size() - 1)) << ");\n";
            }
        }
    } else {
        // Standard one-to-one compilation path (or fallback path where extra variables get nil)
        for (size_t i = 0; i < names.size(); ++i) {
            const auto &name = names[i];

            if (ar().global_const_names.contains(name)) {
                ThrowError("local variable conflicts with global constant: " + name, stmt);
            }

            bool is_captured = false;
            if (const auto it = stmt_var_to_def_.find({stmt.get(), name}); it != stmt_var_to_def_.end()) {
                is_captured = it->second->is_captured;
            }
            if (is_captured) {
                const std::string init = (i < exps.size()) ? CompileExp(exps[i]) : "kNil";
                Out() << GenTab() << "CVar *__box_" << name << " = (CVar *)FakeluaAlloc(_S, sizeof(CVar), false);\n";
                Out() << GenTab() << "*__box_" << name << " = " << init << ";\n";
                continue;
            }

            const auto type = (i < exps.size()) ? LookupNodeType(exps[i].get()) : T_DYNAMIC;
            if (type == T_INT || type == T_FLOAT) {
                const auto native_expr = CompileNumericExp(exps[i]);
                const std::string type_str = (type == T_INT) ? "int64_t" : "double";
                if (ir().shadowed_decls.contains({stmt.get(), name})) {
                    const auto tmp = std::format("flua_local_{}", tmp_var_counter_++);
                    func_temp_decls_ << "    " << type_str << " " << tmp << ";\n";
                    Out() << GenTab() << tmp << " = " << native_expr << ";\n";
                    Out() << GenTab() << type_str << " " << name << " = " << tmp << ";\n";
                } else {
                    Out() << GenTab() << type_str << " " << name << " = " << native_expr << ";\n";
                }
            } else if (i < exps.size()) {
                const auto init_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(exps[i]);
                bool is_degraded_expression = false;
                if (init_exp && LookupNodeType(exps[i].get()) == T_DYNAMIC) {
                    if (const auto kind = init_exp->GetExpKind(); kind == ExpKind::kNumber) {
                        is_degraded_expression = true;
                    } else if (kind == ExpKind::kBinop && init_exp->Left() && init_exp->Right()) {
                        const auto lt = LookupNodeType(init_exp->Left().get());
                        const auto rt = LookupNodeType(init_exp->Right().get());
                        is_degraded_expression = IsNumericInferredType(lt) && IsNumericInferredType(rt);
                    } else if (kind == ExpKind::kUnop && init_exp->Right()) {
                        const auto ot = LookupNodeType(init_exp->Right().get());
                        is_degraded_expression = IsNumericInferredType(ot);
                    }
                }
                if (!is_degraded_expression) {
                    DEBUG_ASSERT(GetType(exps[i]) != T_INT && GetType(exps[i]) != T_FLOAT);
                }
                const std::string init = CompileExp(exps[i]);
                Out() << GenTab() << "CVar " << name << " = " << init << ";\n";
            } else {
                Out() << GenTab() << "CVar " << name << " = kNil;\n";
            }
        }
    }
}

void CGen::CompileStmtAssign(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::Assign);
    const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(stmt);

    const auto varlist = assign->Varlist();
    DEBUG_ASSERT(varlist->Type() == SyntaxTreeType::VarList);
    const auto varlist_ptr = std::dynamic_pointer_cast<SyntaxTreeVarlist>(varlist);
    const auto &vars = varlist_ptr->Vars();

    const auto explist = assign->Explist();
    DEBUG_ASSERT(explist && explist->Type() == SyntaxTreeType::ExpList);
    const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
    const auto &exps = explist_ptr->Exps();

    // PreprocessSplitAssign 保证此时恰好有 1 个变量和 1 个表达式
    DEBUG_ASSERT(vars.size() == 1);
    DEBUG_ASSERT(exps.size() == 1);

    DEBUG_ASSERT(vars[0]->Type() == SyntaxTreeType::Var);
    const auto v_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(vars[0]);

    // PreprocessTableAssign 将方括号/点号赋值重写为 FAKELUA_SET_TABLE 调用，
    // 所以只有简单变量赋值能到达此处。
    DEBUG_ASSERT(v_ptr->GetVarKind() == VarKind::kSimple);
    if (const auto &name = v_ptr->GetName(); IsTypedNativeVar(name, v_ptr.get())) {
        // 被赋值变量是原生类型（int64_t / double）变量：
        const auto var_type = GetNativeVarType(name, v_ptr.get());
        if (const auto native_rhs = TryCompileNativeExpr(exps[0]); !native_rhs.empty()) {
            // RHS 可以直接编译为原生数值表达式——无需临时 CVar。
            // 若 RHS 类型与目标变量类型不同（如 double → int64_t），插入显式强制转换。
            if (const auto rhs_type = GetType(exps[0]); rhs_type == T_FLOAT && var_type == T_INT) {
                Out() << GenTab() << name << " = (int64_t)(" << native_rhs << ");\n";
            } else if (rhs_type == T_INT && var_type == T_FLOAT) {
                Out() << GenTab() << name << " = (double)(" << native_rhs << ");\n";
            } else {
                Out() << GenTab() << name << " = " << native_rhs << ";\n";
            }
        } else {
            // RHS 无法编译为原生数值（如调用返回 CVar 的函数）：
            //   (1) 将 RHS 编译为 CVar 临时变量；
            //   (2) 运行时检查 CVar 类型，拆包为原生类型赋值；非数值则抛出运行时错误。
            const std::string rhs = CompileExp(exps[0]);
            const auto tmp = std::format("flua_assign_tmp_{}", tmp_var_counter_++);
            func_temp_decls_ << "    CVar " << tmp << ";\n";
            Out() << GenTab() << tmp << " = " << rhs << ";\n";
            if (var_type == T_FLOAT) {
                // 运行时检查：CVar 必须是数值类型，否则报错。
                Out() << GenTab() << "if (LIKELY(" << tmp << ".type_ == VAR_FLOAT)) {\n";
                cur_tab_++;
                Out() << GenTab() << name << " = " << tmp << ".data_.f;\n";
                cur_tab_--;
                Out() << GenTab() << "} else if (" << tmp << ".type_ == VAR_INT) {\n";
                cur_tab_++;
                Out() << GenTab() << name << " = (double)" << tmp << ".data_.i;\n";
                cur_tab_--;
                Out() << GenTab() << "} else {\n";
                cur_tab_++;
                Out() << GenTab() << "FakeluaThrowError(_S, \"attempt to assign non-numeric value to typed float variable\");\n";
                cur_tab_--;
                Out() << GenTab() << "}\n";
            } else {
                // 运行时检查：CVar 必须是数值类型，否则报错。
                Out() << GenTab() << "if (LIKELY(" << tmp << ".type_ == VAR_INT)) {\n";
                cur_tab_++;
                Out() << GenTab() << name << " = " << tmp << ".data_.i;\n";
                cur_tab_--;
                Out() << GenTab() << "} else if (" << tmp << ".type_ == VAR_FLOAT) {\n";
                cur_tab_++;
                Out() << GenTab() << name << " = (int64_t)" << tmp << ".data_.f;\n";
                cur_tab_--;
                Out() << GenTab() << "} else {\n";
                cur_tab_++;
                Out() << GenTab() << "FakeluaThrowError(_S, \"attempt to assign non-numeric value to typed int variable\");\n";
                cur_tab_--;
                Out() << GenTab() << "}\n";
            }
        }
    } else {
        const std::string rhs = CompileExp(exps[0]);
        Out() << GenTab() << CompileVar(v_ptr) << " = " << rhs << ";\n";
    }
}


std::string CGen::CompileCondBoolExpr(const SyntaxTreeInterfacePtr &exp, const std::string &tmp_prefix) {
    if (auto native_cond = TryCompileNativeBoolExpr(exp); !native_cond.empty()) {
        return native_cond;
    }
    const auto tmp_bool = std::format("{}_{}", tmp_prefix, tmp_var_counter_++);
    func_temp_decls_ << "    bool " << tmp_bool << ";\n";
    const auto cond = CompileExp(exp);
    Out() << GenTab() << std::format("IsTrue(({}), {});\n", cond, tmp_bool);
    return tmp_bool;
}

void CGen::CompileStmtWhile(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::While);
    const auto while_stmt = std::dynamic_pointer_cast<SyntaxTreeWhile>(stmt);

    if (const auto native_cond = TryCompileNativeBoolExpr(while_stmt->Exp()); !native_cond.empty()) {
        Out() << GenTab() << "while (" << native_cond << ") {\n";
        cur_tab_++;
        CompileStmtBlock(while_stmt->Block());
        cur_tab_--;
        Out() << GenTab() << "}\n";
        return;
    }

    const auto tmp_bool = std::format("flua_wbt_{}", tmp_var_counter_++);
    func_temp_decls_ << "    bool " << tmp_bool << ";\n";
    Out() << GenTab() << "while (1) {\n";
    cur_tab_++;
    const auto cond = CompileExp(while_stmt->Exp());
    Out() << GenTab() << std::format("IsTrue(({}), {});\n", cond, tmp_bool);
    Out() << GenTab() << std::format("if (!{}) break;\n", tmp_bool);
    CompileStmtBlock(while_stmt->Block());
    cur_tab_--;
    Out() << GenTab() << "}\n";
}

void CGen::CompileStmtRepeat(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::Repeat);
    const auto repeat_stmt = std::dynamic_pointer_cast<SyntaxTreeRepeat>(stmt);

    Out() << GenTab() << "do {\n";
    // Lua 语义：until 条件可访问块内声明的 local 变量 —— 先编译块，再编译条件。
    cur_tab_++;

    CompileStmtBlock(repeat_stmt->Block());
    const auto cond_bool = CompileCondBoolExpr(repeat_stmt->Exp(), "flua_rbt");
    Out() << GenTab() << std::format("if ({}) break;\n", cond_bool);

    cur_tab_--;
    Out() << GenTab() << "} while (1);\n";
}

void CGen::CompileStmtIf(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::If);
    const auto if_stmt = std::dynamic_pointer_cast<SyntaxTreeIf>(stmt);

    // 流敏感 spec 类型已由 TypeInferencer 的前向分析标注到 ir.var_spec_annotations，
    // 各分支直接顺序编译即可，不再需要快照 / 汇合。

    const auto cond_bool = CompileCondBoolExpr(if_stmt->Exp(), "flua_ibt");
    Out() << GenTab() << std::format("if ({}) {{\n", cond_bool);
    cur_tab_++;
    CompileStmtBlock(if_stmt->Block());
    cur_tab_--;
    Out() << GenTab() << "}";

    int elseif_depth = 0;
    if (const auto elseifs_node = if_stmt->ElseIfs()) {
        const auto elseif_list = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(elseifs_node);
        for (size_t i = 0; i < elseif_list->ElseifSize(); ++i) {
            Out() << " else {\n";
            cur_tab_++;
            elseif_depth++;

            const auto econd_bool = CompileCondBoolExpr(elseif_list->ElseifExp(i), "flua_ibt");
            Out() << GenTab() << std::format("if ({}) {{\n", econd_bool);
            cur_tab_++;
            CompileStmtBlock(elseif_list->ElseifBlock(i));
            cur_tab_--;
            Out() << GenTab() << "}";
        }
    }

    if (const auto else_block = if_stmt->ElseBlock()) {
        Out() << " else {\n";
        cur_tab_++;
        CompileStmtBlock(else_block);
        cur_tab_--;
        Out() << GenTab() << "}";
    }

    for (int i = 0; i < elseif_depth; ++i) {
        Out() << "\n";
        cur_tab_--;
        Out() << GenTab() << "}";
    }

    Out() << "\n";
}

void CGen::CompileStmtBreak(const SyntaxTreeInterfacePtr &stmt) {
    Out() << GenTab() << "break;\n";
}

void CGen::CompileStmtGoto(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::Goto);
    const auto goto_stmt = std::dynamic_pointer_cast<SyntaxTreeGoto>(stmt);
    Out() << GenTab() << "goto flua_L_" << goto_stmt->GetLabel() << ";\n";
}

void CGen::CompileStmtLabel(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::Label);
    const auto label_stmt = std::dynamic_pointer_cast<SyntaxTreeLabel>(stmt);
    Out() << "flua_L_" << label_stmt->GetName() << ": ;\n";
}

void CGen::CompileStmtForLoop(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::ForLoop);
    const auto for_stmt = std::dynamic_pointer_cast<SyntaxTreeForLoop>(stmt);

    const bool typed_int_for = for_stmt->ExpBegin() && for_stmt->ExpEnd() && LookupNodeType(for_stmt->ExpBegin().get()) == T_INT && LookupNodeType(for_stmt->ExpEnd().get()) == T_INT &&
                               (!for_stmt->ExpStep() || LookupNodeType(for_stmt->ExpStep().get()) == T_INT);
    if (typed_int_for) {
        CompileTypedNumericForLoop(for_stmt, T_INT);
        return;
    }

    // T_FLOAT 快路径：所有边界为数值（T_INT 或 T_FLOAT）但并非全为 T_INT。
    const bool step_is_numeric = !for_stmt->ExpStep() || LookupNodeType(for_stmt->ExpStep().get()) == T_INT || LookupNodeType(for_stmt->ExpStep().get()) == T_FLOAT;
    const bool typed_float_for = !typed_int_for && for_stmt->ExpBegin() && for_stmt->ExpEnd() &&
                                 (LookupNodeType(for_stmt->ExpBegin().get()) == T_INT || LookupNodeType(for_stmt->ExpBegin().get()) == T_FLOAT) &&
                                 (LookupNodeType(for_stmt->ExpEnd().get()) == T_INT || LookupNodeType(for_stmt->ExpEnd().get()) == T_FLOAT) && step_is_numeric;
    if (typed_float_for) {
        CompileTypedNumericForLoop(for_stmt, T_FLOAT);
        return;
    }

    CompileDynamicForLoop(for_stmt);
}

void CGen::CompileTypedNumericForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_stmt, InferredType loop_type) {
    const auto ctrl_var = std::format("flua_for_ctrl_{}", tmp_var_counter_++);
    const auto end_var = std::format("flua_for_end_{}", tmp_var_counter_++);
    const auto step_var = std::format("flua_for_step_{}", tmp_var_counter_++);

    const std::string type_str = (loop_type == T_INT) ? "int64_t" : "double";
    const std::string zero_str = (loop_type == T_INT) ? "0" : "0.0";
    const std::string step_default = (loop_type == T_INT) ? "1" : "1.0";
    const std::string cast_prefix = (loop_type == T_INT) ? "" : "(double)";

    bool is_constant_step = false;
    double step_double_val = 1.0;
    int64_t step_int_val = 1;
    if (!for_stmt->ExpStep()) {
        is_constant_step = true;
        step_double_val = 1.0;
        step_int_val = 1;
    } else if (const auto step_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(for_stmt->ExpStep()); step_exp && step_exp->GetExpKind() == ExpKind::kNumber) {
        is_constant_step = true;
        if (loop_type == T_INT) {
            step_int_val = ToInteger(step_exp->ExpValue());
            if (step_int_val == 0) {
                ThrowError("'for' step is zero", for_stmt->ExpStep());
            }
        } else {
            step_double_val = (LookupNodeType(step_exp.get()) == T_INT) ? static_cast<double>(ToInteger(step_exp->ExpValue())) : ToFloat(step_exp->ExpValue());
            if (step_double_val == 0.0) {
                ThrowError("'for' step is zero", for_stmt->ExpStep());
            }
        }
    }

    func_temp_decls_ << "    " << type_str << " " << ctrl_var << ";\n";
    func_temp_decls_ << "    " << type_str << " " << end_var << ";\n";
    if (!is_constant_step) {
        func_temp_decls_ << "    " << type_str << " " << step_var << ";\n";
    }

    const auto native_begin = CompileNumericExp(for_stmt->ExpBegin());
    Out() << GenTab() << ctrl_var << " = " << cast_prefix << "(" << native_begin << ");\n";
    const auto native_end = CompileNumericExp(for_stmt->ExpEnd());
    Out() << GenTab() << end_var << " = " << cast_prefix << "(" << native_end << ");\n";

    if (is_constant_step) {
        if (loop_type == T_INT) {
            if (step_int_val > 0) {
                if (step_int_val == 1) {
                    Out() << GenTab() << "for (; " << ctrl_var << " <= " << end_var << "; " << ctrl_var << "++) {\n";
                } else {
                    Out() << GenTab() << "for (; " << ctrl_var << " <= " << end_var << "; " << ctrl_var << " += " << step_int_val << ") {\n";
                }
            } else {
                if (step_int_val == -1) {
                    Out() << GenTab() << "for (; " << ctrl_var << " >= " << end_var << "; " << ctrl_var << "--) {\n";
                } else {
                    Out() << GenTab() << "for (; " << ctrl_var << " >= " << end_var << "; " << ctrl_var << " += " << step_int_val << ") {\n";
                }
            }
        } else {
            if (step_double_val > 0.0) {
                if (step_double_val == 1.0) {
                    Out() << GenTab() << "for (; " << ctrl_var << " <= " << end_var << "; " << ctrl_var << "++) {\n";
                } else {
                    Out() << GenTab() << "for (; " << ctrl_var << " <= " << end_var << "; " << ctrl_var << " += " << step_double_val << ") {\n";
                }
            } else {
                if (step_double_val == -1.0) {
                    Out() << GenTab() << "for (; " << ctrl_var << " >= " << end_var << "; " << ctrl_var << "--) {\n";
                } else {
                    Out() << GenTab() << "for (; " << ctrl_var << " >= " << end_var << "; " << ctrl_var << " += " << step_double_val << ") {\n";
                }
            }
        }
    } else {
        const auto native_step = CompileNumericExp(for_stmt->ExpStep());
        Out() << GenTab() << step_var << " = " << cast_prefix << "(" << native_step << ");\n";
        Out() << GenTab() << "if (UNLIKELY(" << step_var << " == " << zero_str << ")) { FakeluaThrowError(_S, \"'for' step is zero\"); }\n";
        Out() << GenTab() << "for (; (" << step_var << " > " << zero_str << ") ? (" << ctrl_var << " <= " << end_var << ") : (" << ctrl_var << " >= " << end_var << "); " << ctrl_var
              << " += " << step_var << ") {\n";
    }
    cur_tab_++;
    bool is_captured = false;
    if (const auto it = stmt_var_to_def_.find({for_stmt.get(), for_stmt->Name()}); it != stmt_var_to_def_.end()) {
        is_captured = it->second->is_captured;
    }
    if (is_captured) {
        Out() << GenTab() << "CVar *__box_" << for_stmt->Name() << " = (CVar *)FakeluaAlloc(_S, sizeof(CVar), false);\n";
        Out() << GenTab() << "*__box_" << for_stmt->Name() << " = " << BoxNativeValue(ctrl_var, loop_type) << ";\n";
    } else {
        if (LookupNodeType(for_stmt.get()) == loop_type) {
            Out() << GenTab() << type_str << " " << for_stmt->Name() << " = " << ctrl_var << ";\n";
        } else {
            Out() << GenTab() << "CVar " << for_stmt->Name() << " = " << BoxNativeValue(ctrl_var, loop_type) << ";\n";
        }
    }
    // 用内层作用域包裹循环体，避免 local 同名变量与循环变量在同一 C 作用域中重复声明。
    Out() << GenTab() << "{\n";
    cur_tab_++;
    CompileStmtBlock(for_stmt->Block());
    cur_tab_--;
    Out() << GenTab() << "}\n";
    cur_tab_--;
    Out() << GenTab() << "}\n";
}

void CGen::CompileDynamicForLoop(const std::shared_ptr<SyntaxTreeForLoop> &for_stmt) {
    const auto ctrl_var = std::format("flua_for_ctrl_{}", tmp_var_counter_++);
    const auto end_var = std::format("flua_for_end_{}", tmp_var_counter_++);
    const auto step_var = std::format("flua_for_step_{}", tmp_var_counter_++);
    const auto step_pos_var = std::format("flua_for_step_pos_{}", tmp_var_counter_++);
    const auto cond_var = std::format("flua_for_cond_{}", tmp_var_counter_++);
    const auto cmp_var = std::format("flua_for_cmp_{}", tmp_var_counter_++);

    func_temp_decls_ << "    CVar " << ctrl_var << ";\n";
    func_temp_decls_ << "    CVar " << end_var << ";\n";
    func_temp_decls_ << "    CVar " << step_var << ";\n";
    func_temp_decls_ << "    bool " << step_pos_var << ";\n";
    func_temp_decls_ << "    bool " << cond_var << ";\n";
    func_temp_decls_ << "    CVar " << cmp_var << ";\n";

    const auto begin_expr = CompileExp(for_stmt->ExpBegin());
    Out() << GenTab() << ctrl_var << " = " << begin_expr << ";\n";

    const auto end_expr = CompileExp(for_stmt->ExpEnd());
    Out() << GenTab() << end_var << " = " << end_expr << ";\n";

    if (for_stmt->ExpStep()) {
        const auto step_expr = CompileExp(for_stmt->ExpStep());
        Out() << GenTab() << step_var << " = " << step_expr << ";\n";
    } else {
        Out() << GenTab() << "SET_INT(" << step_var << ", 1);\n";
    }

    Out() << GenTab() << "if (LIKELY(" << step_var << ".type_ == VAR_INT)) {\n";
    Out() << GenTab() << "    if (UNLIKELY(" << step_var << ".data_.i == 0)) { FakeluaThrowError(_S, \"'for' step is zero\"); }\n";
    Out() << GenTab() << "    " << step_pos_var << " = (" << step_var << ".data_.i > 0);\n";
    Out() << GenTab() << "} else if (" << step_var << ".type_ == VAR_FLOAT) {\n";
    Out() << GenTab() << "    if (UNLIKELY(" << step_var << ".data_.f == 0.0)) { FakeluaThrowError(_S, \"'for' step is zero\"); }\n";
    Out() << GenTab() << "    " << step_pos_var << " = (" << step_var << ".data_.f > 0.0);\n";
    Out() << GenTab() << "} else { FakeluaThrowError(_S, \"'for' step must be a number\"); " << step_pos_var << " = 1; }\n";

    Out() << GenTab() << "while (1) {\n";
    cur_tab_++;

    Out() << GenTab() << "if (" << step_pos_var << ") {\n";
    cur_tab_++;
    Out() << GenTab() << std::format("OpLe(({0}), ({1}), {2});\n", ctrl_var, end_var, cmp_var);
    cur_tab_--;
    Out() << GenTab() << "} else {\n";
    cur_tab_++;
    Out() << GenTab() << std::format("OpGe(({0}), ({1}), {2});\n", ctrl_var, end_var, cmp_var);
    cur_tab_--;
    Out() << GenTab() << "}\n";

    Out() << GenTab() << std::format("IsTrue(({0}), {1});\n", cmp_var, cond_var);
    Out() << GenTab() << std::format("if (!{}) break;\n", cond_var);

    const auto &loop_var_name = for_stmt->Name();
    bool is_captured = false;
    if (const auto it = stmt_var_to_def_.find({for_stmt.get(), loop_var_name}); it != stmt_var_to_def_.end()) {
        is_captured = it->second->is_captured;
    }
    if (is_captured) {
        Out() << GenTab() << "CVar *__box_" << loop_var_name << " = (CVar *)FakeluaAlloc(_S, sizeof(CVar), false);\n";
        Out() << GenTab() << "*__box_" << loop_var_name << " = " << ctrl_var << ";\n";
    } else {
        Out() << GenTab() << "CVar " << loop_var_name << " = " << ctrl_var << ";\n";
    }

    Out() << GenTab() << "{\n";
    cur_tab_++;
    CompileStmtBlock(for_stmt->Block());
    cur_tab_--;
    Out() << GenTab() << "}\n";

    Out() << GenTab() << std::format("OpAdd(({0}), ({1}), {2});\n", ctrl_var, step_var, ctrl_var);

    cur_tab_--;
    Out() << GenTab() << "}\n";
}

void CGen::CompileStmtForIn(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::ForIn);
    const auto for_in = std::dynamic_pointer_cast<SyntaxTreeForIn>(stmt);

    const auto namelist = for_in->Namelist();
    DEBUG_ASSERT(namelist->Type() == SyntaxTreeType::NameList);
    const auto namelist_ptr = std::dynamic_pointer_cast<SyntaxTreeNamelist>(namelist);
    const auto &names = namelist_ptr->Names();
    DEBUG_ASSERT(!names.empty());

    const auto explist = for_in->Explist();
    DEBUG_ASSERT(explist->Type() == SyntaxTreeType::ExpList);
    const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);

    auto handle_loop_var = [&](const std::string &vname, const std::string &src_tmp) {
        bool is_captured = false;
        if (const auto it = stmt_var_to_def_.find({for_in.get(), vname}); it != stmt_var_to_def_.end()) {
            is_captured = it->second->is_captured;
        }
        if (is_captured) {
            Out() << GenTab() << "CVar *__box_" << vname << " = (CVar *)FakeluaAlloc(_S, sizeof(CVar), false);\n";
            Out() << GenTab() << "*__box_" << vname << " = " << src_tmp << ";\n";
        } else {
            Out() << GenTab() << "CVar " << vname << " = " << src_tmp << ";\n";
        }
    };

    bool is_pairs_or_ipairs = false;
    SyntaxTreeInterfacePtr tbl_exp_node;

    // Detect pairs/ipairs fast path with early returns to flatten nesting
    if (explist_ptr->Exps().size() == 1 && names.size() <= 2) {
        const auto exp = explist_ptr->Exps()[0];
        if (exp && exp->Type() == SyntaxTreeType::Exp) {
            const auto exp_ptr = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
            if (exp_ptr && exp_ptr->GetExpKind() == ExpKind::kPrefixExp && exp_ptr->Right()) {
                const auto pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(exp_ptr->Right());
                if (pe_ptr && pe_ptr->GetPrefixKind() == PrefixExpKind::kFunctionCall) {
                    const auto fc_ptr = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(pe_ptr->GetValue());
                    if (fc_ptr && fc_ptr->Name().empty()) {
                        const auto func_pe = fc_ptr->prefixexp();
                        if (func_pe && func_pe->Type() == SyntaxTreeType::PrefixExp) {
                            const auto func_pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(func_pe);
                            if (func_pe_ptr && func_pe_ptr->GetPrefixKind() == PrefixExpKind::kVar) {
                                const auto func_var = std::dynamic_pointer_cast<SyntaxTreeVar>(func_pe_ptr->GetValue());
                                if (func_var && (func_var->GetName() == "pairs" || func_var->GetName() == "ipairs")) {
                                    const auto args_node = fc_ptr->Args();
                                    if (args_node && args_node->Type() == SyntaxTreeType::Args) {
                                        const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(args_node);
                                        if (args_ptr && args_ptr->GetArgsKind() == ArgsKind::kExpList) {
                                            const auto args_explist = std::dynamic_pointer_cast<SyntaxTreeExplist>(args_ptr->Explist());
                                            if (args_explist && args_explist->Exps().size() == 1) {
                                                is_pairs_or_ipairs = true;
                                                tbl_exp_node = args_explist->Exps()[0];
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (is_pairs_or_ipairs) {
        const auto tbl_expr = CompileExp(tbl_exp_node);

        const auto tbl_var = std::format("flua_fi_tbl_{}", tmp_var_counter_++);
        const auto sz_var = std::format("flua_fi_sz_{}", tmp_var_counter_++);
        const auto idx_var = std::format("flua_fi_idx_{}", tmp_var_counter_++);

        func_temp_decls_ << "    CVar " << tbl_var << ";\n";
        func_temp_decls_ << "    uint32_t " << sz_var << ";\n";
        func_temp_decls_ << "    uint32_t " << idx_var << ";\n";

        Out() << GenTab() << tbl_var << " = " << tbl_expr << ";\n";
        Out() << GenTab() << "if (UNLIKELY(" << tbl_var << ".type_ != VAR_TABLE)) { FakeluaThrowError(_S, \"for in: not a table\"); }\n";
        Out() << GenTab() << sz_var << " = " << tbl_var << ".data_.t->count_ + " << tbl_var << ".data_.t->spec_count;\n";

        Out() << GenTab() << "for (" << idx_var << " = 0; " << idx_var << " < " << sz_var << "; " << idx_var << "++) {\n";
        cur_tab_++;

        const auto &key_name = names[0];
        const auto tmp_k = std::format("flua_fi_k_{}", tmp_var_counter_++);
        Out() << GenTab() << "CVar " << tmp_k << ";\n";
        std::string tmp_v = "";
        if (names.size() >= 2) {
            tmp_v = std::format("flua_fi_v_{}", tmp_var_counter_++);
            Out() << GenTab() << "CVar " << tmp_v << ";\n";
            Out() << GenTab() << std::format("GET_TABLE_ENTRY({}, {}, {}, {});\n", tbl_var, idx_var, tmp_k, tmp_v);
            Out() << GenTab() << std::format("if ({} == VAR_NIL) {{ continue; }}\n", tmp_v + ".type_");
        } else {
            const auto dummy_val = std::format("flua_fi_dummy_val_{}", tmp_var_counter_++);
            func_temp_decls_ << "    CVar " << dummy_val << ";\n";
            Out() << GenTab() << std::format("GET_TABLE_ENTRY({}, {}, {}, {});\n", tbl_var, idx_var, tmp_k, dummy_val);
            Out() << GenTab() << std::format("if ({} == VAR_NIL) {{ continue; }}\n", dummy_val + ".type_");
        }

        handle_loop_var(key_name, tmp_k);
        if (names.size() >= 2) {
            handle_loop_var(names[1], tmp_v);
        }

        Out() << GenTab() << "{\n";
        cur_tab_++;
        CompileStmtBlock(for_in->Block());
        cur_tab_--;
        Out() << GenTab() << "}\n";

        cur_tab_--;
        Out() << GenTab() << "}\n";
    } else {
        std::string iter_f = std::format("flua_fi_f_{}", tmp_var_counter_++);
        std::string iter_s = std::format("flua_fi_s_{}", tmp_var_counter_++);
        std::string iter_var = std::format("flua_fi_var_{}", tmp_var_counter_++);

        func_temp_decls_ << "    CVar " << iter_f << ";\n";
        func_temp_decls_ << "    CVar " << iter_s << ";\n";
        func_temp_decls_ << "    CVar " << iter_var << ";\n";

        const auto &exps = explist_ptr->Exps();
        if (exps.size() == 1) {
            std::string exp0_compiled = CompileExp(exps[0]);
            std::string tmp_exp0 = std::format("flua_fi_exp0_{}", tmp_var_counter_++);
            func_temp_decls_ << "    CVar " << tmp_exp0 << ";\n";
            Out() << GenTab() << tmp_exp0 << " = " << exp0_compiled << ";\n";

            Out() << GenTab() << "if (" << tmp_exp0 << ".type_ == VAR_MULTI) {\n";
            Out() << GenTab() << "    " << iter_f << " = FlUnboxMulti(" << tmp_exp0 << ", 0);\n";
            Out() << GenTab() << "    " << iter_s << " = FlUnboxMulti(" << tmp_exp0 << ", 1);\n";
            Out() << GenTab() << "    " << iter_var << " = FlUnboxMulti(" << tmp_exp0 << ", 2);\n";
            Out() << GenTab() << "} else {\n";
            Out() << GenTab() << "    " << iter_f << " = " << tmp_exp0 << ";\n";
            Out() << GenTab() << "    " << iter_s << " = kNil;\n";
            Out() << GenTab() << "    " << iter_var << " = kNil;\n";
            Out() << GenTab() << "}\n";
        } else {
            std::string e0 = CompileExp(exps[0]);
            std::string e1 = (exps.size() >= 2) ? CompileExp(exps[1]) : "kNil";
            std::string e2 = (exps.size() >= 3) ? CompileExp(exps[2]) : "kNil";
            Out() << GenTab() << iter_f << " = FlUnboxMulti(" << e0 << ", 0);\n";
            Out() << GenTab() << iter_s << " = FlUnboxMulti(" << e1 << ", 0);\n";
            Out() << GenTab() << iter_var << " = FlUnboxMulti(" << e2 << ", 0);\n";
        }

        Out() << GenTab() << "while (true) {\n";
        cur_tab_++;

        std::string iter_res = std::format("flua_fi_res_{}", tmp_var_counter_++);
        Out() << GenTab() << "CVar " << iter_res << " = FlCallClosure(_S, " << iter_f << ", 2, " << iter_s << ", " << iter_var << ");\n";

        std::vector<std::string> loop_var_tmps;
        for (size_t i = 0; i < names.size(); ++i) {
            std::string vtmp = std::format("flua_fi_v_{}_{}", i, tmp_var_counter_++);
            Out() << GenTab() << "CVar " << vtmp << " = FlUnboxMulti(" << iter_res << ", " << i << ");\n";
            loop_var_tmps.push_back(vtmp);
        }

        Out() << GenTab() << "if (" << loop_var_tmps[0] << ".type_ == VAR_NIL) { break; }\n";
        Out() << GenTab() << iter_var << " = " << loop_var_tmps[0] << ";\n";

        for (size_t i = 0; i < names.size(); ++i) {
            handle_loop_var(names[i], loop_var_tmps[i]);
        }

        Out() << GenTab() << "{\n";
        cur_tab_++;
        CompileStmtBlock(for_in->Block());
        cur_tab_--;
        Out() << GenTab() << "}\n";

        cur_tab_--;
        Out() << GenTab() << "}\n";
    }
}

// ===========================================================================
// 第三部分：表达式编译
// ===========================================================================

// 编译表达式：将 AST 表达式节点转换为相应的 C 语言表达式字符串（如字面量、前缀表达式、二元/一元运算等）
std::string CGen::CompileExp(const SyntaxTreeInterfacePtr &exp, bool preserve_multi) {
    DEBUG_ASSERT(exp->Type() == SyntaxTreeType::Exp);
    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto exp_kind = e->GetExpKind();
    const auto &value = e->ExpValue();

    DEBUG_ASSERT(exp_kind != ExpKind::kVarParams && "VarParams should have been caught by PreProcessor");

    switch (exp_kind) {
        case ExpKind::kNil:
            return (cur_section_ == Section::Globals) ? "(CVar){.type_ = VAR_NIL}" : "kNil";
        case ExpKind::kFalse:
            return (cur_section_ == Section::Globals) ? "(CVar){.type_ = VAR_BOOL, .data_.b = false}" : "kFalse";
        case ExpKind::kTrue:
            return (cur_section_ == Section::Globals) ? "(CVar){.type_ = VAR_BOOL, .data_.b = true}" : "kTrue";
        case ExpKind::kNumber:
            // Number literal type is a purely syntactic property of the source text
            // (integer vs float format). CompileExp is also called from global-init
            // contexts where the snapshot may not cover this node, so we derive it
            // directly from the literal rather than consulting the snapshot.
            if (IsInteger(value)) {
                return std::format("(CVar){{.type_ = VAR_INT, .data_.i = {}}}", ToInteger(value));
            } else {
                return std::format("(CVar){{.type_ = VAR_FLOAT, .data_.f = {}}}", ToFloat(value));
            }
        case ExpKind::kString:
            return std::format("(CVar){{.type_ = VAR_STRINGID, .data_.i = {}}}", s_->GetConstString().Alloc(value));
        case ExpKind::kPrefixExp:
            return CompilePrefixexp(e->Right(), preserve_multi);
        case ExpKind::kTableConstructor:
            return CompileTableconstructor(e->Right());
        case ExpKind::kFunctionDef: {
            const auto func_def = std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(e->Right());
            FuncInfo *func = func_map_[func_def.get()];
            std::string closure_expr = std::format("FlMakeClosure(_S, (void*){}, {}, {}, {}",
                                                   func->unique_c_name,
                                                   func->captured_vars.size(),
                                                   func->params.size(),
                                                   func->is_vararg ? "true" : "false");
            for (VarDef *up : func->captured_vars) {
                closure_expr += ", " + CompileUpvaluePointer(up);
            }
            closure_expr += ")";
            return closure_expr;
        }
        case ExpKind::kBinop:
            return CompileBinop(e, e->Op());
        case ExpKind::kUnop:
            return CompileUnop(e, e->Op());
        default:
            ThrowError("unsupported expression kind", e);
    }
}

std::string CGen::CompilePrefixexp(const SyntaxTreeInterfacePtr &pe, bool preserve_multi) {
    DEBUG_ASSERT(pe->Type() == SyntaxTreeType::PrefixExp);
    const auto pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(pe);

    const auto pe_kind = pe_ptr->GetPrefixKind();
    const auto value = pe_ptr->GetValue();

    DEBUG_ASSERT(pe_kind == PrefixExpKind::kVar || pe_kind == PrefixExpKind::kFunctionCall || pe_kind == PrefixExpKind::kExp);

    if (pe_kind == PrefixExpKind::kVar) {
        const auto var_str = CompileVar(value);
        // 若变量是 vararg 隐式参数（类型可能为 VarType::Multi），且调用方不需要保留 Multi，
        // 则取第一个值，等同于函数调用在非末尾位置的处理（FlUnboxMulti(x, 0)）。
        if (!preserve_multi) {
            const auto var_node = std::dynamic_pointer_cast<SyntaxTreeVar>(value);
            if (var_node && var_node->GetVarKind() == VarKind::kSimple &&
                var_node->GetName().rfind("__fakelua_vararg_", 0) == 0) {
                return std::format("FlUnboxMulti({}, 0)", var_str);
            }
        }
        return var_str;
    } else if (pe_kind == PrefixExpKind::kFunctionCall) {
        std::string call = CompileFunctioncall(value);
        if (preserve_multi) {
            return call;
        }
        std::string callee = ar().callee_names.contains(value.get()) ? ar().callee_names.at(value.get()) : "";
        bool is_single_return_local = !callee.empty() && ar().function_max_returns.contains(callee) && ar().function_max_returns.at(callee) == 1;
        if (is_single_return_local) {
            return call;
        }
        return std::format("FlUnboxMulti({}, 0)", call);
    } else /*if (pe_kind == PrefixExpKind::kExp)*/ {
        return CompileExp(value, preserve_multi);
    }
}

std::string CGen::CompileTableconstructor(const SyntaxTreeInterfacePtr &tc) {
    DEBUG_ASSERT(cur_section_ != Section::Globals);

    DEBUG_ASSERT(tc->Type() == SyntaxTreeType::TableConstructor);
    const auto tc_ptr = std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(tc);

    // Duplicate-key detection and key classification are done by TypeInferencer
    // (BuildCtorFields throws on duplicate keys; table_spec_infos stores the
    // classified layout). CGen just consumes the result.


    const auto var_name = std::format("flua_tbl_{}", tmp_var_counter_++);

    func_temp_decls_ << "    "
                     << "CVar " << var_name << ";\n";

    // table 特化：TypeInferencer 已在 table_spec_infos 中预分类所有静态 key 的构造器
    // （BuildCtorFields 对重复 key 抛出异常；can_specialize 表示可特化）。
    // CGen 直接消费，无需再次遍历 AST。
    if (const auto spec_it = ir().table_spec_infos.find(tc.get());
        spec_it != ir().table_spec_infos.end() && spec_it->second.can_specialize && !spec_it->second.fields.empty()) {
        const auto &fields = spec_it->second.fields;
        {
            const auto spec_type = ComputeTableSpecName(fields);
            const auto get_fn = std::format("FlGetTableStrId_{}", spec_type);
            const auto set_fn = std::format("FlSetTableStrId_{}", spec_type);

            // 发射 table 分配宏（对应的 typedef 与 get/set 函数由 GenerateHeader 发射）。
            Out() << GenTab() << "SET_TABLE_SPEC(" << var_name << ", " << spec_type << ", " << get_fn << ", " << set_fn << ", " << fields.size() << ");\n";

            // 填充 spec_keys/spec_vals (以正确的左到右语法顺序)
            int cur_array_idx = 1;
            const auto fieldlist = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(tc_ptr->Fieldlist());
            for (const auto &field : fieldlist->Fields()) {
                const auto fp = std::dynamic_pointer_cast<SyntaxTreeField>(field);
                if (!fp) continue;

                const auto value_str = CompileExp(fp->Value());

                if (fp->GetFieldKind() == FieldKind::kObject) {
                    const auto key_name = fp->Name();
                    const auto id = s_->GetConstString().Alloc(key_name);
                    Out() << GenTab() << std::format("FlSetTableStrId({}, {}, {});\n", var_name, id, value_str);
                } else {
                    if (fp->Key() == nullptr) {
                        Out() << GenTab() << std::format("FlSetTableInt({}, {}, {});\n", var_name, cur_array_idx, value_str);
                        cur_array_idx++;
                    } else {
                        const auto key_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(fp->Key());
                        if (key_exp) {
                            if (key_exp->GetExpKind() == ExpKind::kString) {
                                const auto key_name = key_exp->ExpValue();
                                const auto id = s_->GetConstString().Alloc(key_name);
                                Out() << GenTab() << std::format("FlSetTableStrId({}, {}, {});\n", var_name, id, value_str);
                            } else if (key_exp->GetExpKind() == ExpKind::kNumber) {
                                std::string num_str = key_exp->ExpValue();
                                if (num_str.find('.') == std::string::npos && num_str.find('e') == std::string::npos && num_str.find('E') == std::string::npos) {
                                    int64_t int_val = std::stoll(num_str);
                                    Out() << GenTab() << std::format("FlSetTableInt({}, {}, {});\n", var_name, int_val, value_str);
                                    cur_array_idx = std::max(cur_array_idx, static_cast<int>(int_val + 1));
                                } else {
                                    double float_val = std::stod(num_str);
                                    Out() << GenTab() << std::format("FlSetTable({}, (CVar){{.type_ = VAR_FLOAT, .data_.f = {}}}, {});\n", var_name, float_val, value_str);
                                }
                            } else if (key_exp->GetExpKind() == ExpKind::kTrue) {
                                Out() << GenTab() << std::format("FlSetTable({}, kTrue, {});\n", var_name, value_str);
                            } else if (key_exp->GetExpKind() == ExpKind::kFalse) {
                                Out() << GenTab() << std::format("FlSetTable({}, kFalse, {});\n", var_name, value_str);
                            }
                        }
                    }
                }
            }

            // Phase 2: 对合并布局中本字面量不包含的 optional 字段显式 nil 初始化。
            // temp allocator 不清零，若漏初始化则 spec_keys/spec_vals/struct 字段为垃圾，
            // 导致 pairs 迭代读到未定义内存。nil 值会被 for-in 的 `if (val == VAR_NIL) continue` 跳过。
            for (size_t f_idx = 0; f_idx < fields.size(); ++f_idx) {
                if (fields[f_idx].optional) {
                    Out() << GenTab() << std::format("FL_SET_SPEC({}, {}, {}, {}, kNil);\n", spec_type, var_name, fields[f_idx].c_field_name, f_idx);
                    Out() << GenTab() << std::format("{}.data_.t->spec_keys[{}] = kNil;\n", var_name, f_idx);
                }
            }

            return var_name;
        }
    }

    // 普通路径：无特化，使用 hash 操作
    Out() << GenTab() << "SET_TABLE(" << var_name << ");\n";
    if (const auto fieldlist = tc_ptr->Fieldlist()) {
        DEBUG_ASSERT(fieldlist->Type() == SyntaxTreeType::FieldList);
        const auto fieldlist_ptr = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(fieldlist);

        std::shared_ptr<SyntaxTreeField> last_array_field = nullptr;
        for (const auto &field: fieldlist_ptr->Fields()) {
            const auto field_ptr = std::dynamic_pointer_cast<SyntaxTreeField>(field);
            if (field_ptr->GetFieldKind() == FieldKind::kArray && !field_ptr->Key()) {
                last_array_field = field_ptr;
            }
        }

        int array_idx = 1;
        for (const auto &field: fieldlist_ptr->Fields()) {
            DEBUG_ASSERT(field->Type() == SyntaxTreeType::Field);
            const auto field_ptr = std::dynamic_pointer_cast<SyntaxTreeField>(field);
            const auto fkind = field_ptr->GetFieldKind();

            const auto value_exp = field_ptr->Value();
            bool is_func = ar().function_call_exps.contains(value_exp.get());
            bool is_vararg = IsVarargExp(value_exp);
            bool is_expand = (field_ptr == last_array_field) && (is_func || is_vararg);
            const auto value_str = CompileExp(value_exp, is_expand);

            if (fkind == FieldKind::kObject) {
                const auto name = field_ptr->Name();
                const auto id = s_->GetConstString().Alloc(name);
                Out() << GenTab() << std::format("FlSetTableStrId({}, {}, {});\n", var_name, id, value_str);
            } else {
                DEBUG_ASSERT(fkind == FieldKind::kArray);
                if (const auto key = field_ptr->Key()) {
                    if (const auto key_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(key); key_exp && key_exp->GetExpKind() == ExpKind::kString) {
                        const auto id = s_->GetConstString().Alloc(key_exp->ExpValue());
                        Out() << GenTab() << std::format("FlSetTableStrId({}, {}, {});\n", var_name, id, value_str);
                    } else {
                        if (const auto key_type = GetType(key); key_type == T_INT) {
                            if (const auto native_key = TryCompileNativeExpr(key); !native_key.empty()) {
                                Out() << GenTab() << std::format("FlSetTableInt({}, {}, {});\n", var_name, native_key, value_str);
                            } else {
                                const auto key_str = CompileExp(key);
                                Out() << GenTab() << std::format("FlSetTable({}, {}, {});\n", var_name, key_str, value_str);
                            }
                        } else {
                            const auto key_str = CompileExp(key);
                            Out() << GenTab() << std::format("FlSetTable({}, {}, {});\n", var_name, key_str, value_str);
                        }
                    }
                } else {
                    if (is_expand) {
                        Out() << GenTab() << std::format("FlTableExpandMulti({}, {}, {});\n", var_name, array_idx, value_str);
                    } else {
                        Out() << GenTab() << std::format("FlSetTableInt({}, {}, {});\n", var_name, array_idx, value_str);
                        ++array_idx;
                    }
                }
            }
        }
    }

    return var_name;
}

// ---------------------------------------------------------------------------
// CompileBinop —— 二元运算符的代码生成
//
// 生成策略：
//
//   1. and / or（短路运算符）：
//      Lua 的 and/or 不返回布尔值，而是返回某一操作数本身。
//      必须先求左操作数并保存，再通过 IsTrue 判断真假，
//      仅在必要时才求右操作数（保证短路语义）。
//
//   2. 原生算术快路径（native fast path）：
//      若两侧操作数类型均已知（T_INT/T_FLOAT，通过 GetType 推断），
//      则将两侧直接编译为原生数值（CompileNumericExp），
//      生成 C 表达式（如 (a) + (b)），并通过 BoxNativeValue 装箱为 CVar 后返回。
//      这条路径消除了 OpAdd/OpSub 等宏的运行时类型分支开销。
//      注意：// 和 % 需要处理除零及 Lua 向下取整语义（FlFloorDivInt / FlModInt / FlModFloat）。
//
//   3. 通用慢速路径（slow path）：
//      两侧均编译为 CVar，调用 OpXxx 宏（处理运行时类型判断和装拆箱）。
//      适用于操作数类型未知或运算符不支持原生路径（如字符串连接 ..）的情形。
// ---------------------------------------------------------------------------
std::string CGen::CompileBinop(const SyntaxTreeInterfacePtr &exp, const SyntaxTreeInterfacePtr &op) {
    DEBUG_ASSERT(cur_section_ != Section::Globals);

    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);
    const auto left = e->Left();
    const auto right = e->Right();
    const auto op_ptr = std::dynamic_pointer_cast<SyntaxTreeBinop>(op);
    const auto op_kind = op_ptr->GetOpKind();
    // Derive the result type from the operand types (consulted via GetType, which
    // respects CGen's local-scope knowledge such as degraded CVar variables — the
    // snapshot can stale for those). This applies the canonical type rule, not a
    // fresh AST walk.
    // Operand types are obtained from the snapshot via GetType, which also respects
    // CGen's local-scope knowledge (e.g. degraded CVar variables). They are only used
    // for the native-path guard and boxing — never to walk the AST.
    const auto lt = GetType(left);
    const auto rt = GetType(right);

    // --- and / or：短路运算，Lua 语义为返回决定性操作数的值 ---
    if (op_kind == BinOpKind::kAnd || op_kind == BinOpKind::kOr) {
        const auto left_str = CompileExp(left);

        const auto tmp = std::format("flua_op_{}", tmp_var_counter_++);
        func_temp_decls_ << "    "
                         << "CVar " << tmp << ";\n";
        const auto tmp_bool = std::format("flua_bt_{}", tmp_var_counter_++);
        func_temp_decls_ << "    "
                         << "bool " << tmp_bool << ";\n";

        Out() << GenTab() << std::format("IsTrue(({}), {});\n", left_str, tmp_bool);

        const bool is_and = (op_kind == BinOpKind::kAnd);
        Out() << GenTab() << std::format("if ({}{}) {{\n", is_and ? "!" : "", tmp_bool);
        cur_tab_++;
        Out() << GenTab() << std::format("{} = {};\n", tmp, left_str);
        cur_tab_--;
        Out() << GenTab() << "} else {\n";
        cur_tab_++;
        const auto right_str = CompileExp(right);
        Out() << GenTab() << std::format("{} = {};\n", tmp, right_str);
        cur_tab_--;
        Out() << GenTab() << "}\n";

        return tmp;
    }

    // Native arithmetic fast path: both operands must be numeric. The result type
    // (T_INT/T_FLOAT) is derived from the operand types via the canonical type rule.
    if (lt != T_DYNAMIC && rt != T_DYNAMIC) {
        const auto result_type = (op_kind == BinOpKind::kSlash || op_kind == BinOpKind::kPow)                                              ? T_FLOAT
                                : (op_kind == BinOpKind::kBitAnd || op_kind == BinOpKind::kXor || op_kind == BinOpKind::kBitOr || op_kind == BinOpKind::kLeftShift || op_kind == BinOpKind::kRightShift)
                                              ? T_INT
                                              : (lt == T_INT && rt == T_INT) ? T_INT : T_FLOAT;
        if (auto native_str = CompileNativeArithBinop(left, right, op_kind, result_type); !native_str.empty()) {
            return native_str;
        }
    }

    // Native comparison fast path: ==, ~=, <, <=, >, >= in expression context.
    if (auto native_str = CompileNativeCmpBinop(left, right, op_kind); !native_str.empty()) {
        return native_str;
    }

    const auto left_str = CompileExp(left);
    const auto right_str = CompileExp(right);

    const auto tmp = std::format("flua_op_{}", tmp_var_counter_++);
    func_temp_decls_ << "    "
                     << "CVar " << tmp << ";\n";

    const auto l = std::format("({})", left_str);
    const auto r = std::format("({})", right_str);

    // Lookup table for binary operators that use the OpXxx macro pattern.
    static const std::unordered_map<BinOpKind, std::string_view> kBinOpMacros = {
            {BinOpKind::kPlus, "OpAdd"},
            {BinOpKind::kMinus, "OpSub"},
            {BinOpKind::kStar, "OpMul"},
            {BinOpKind::kSlash, "OpDiv"},
            {BinOpKind::kDoubleSlash, "OpFloorDiv"},
            {BinOpKind::kPow, "OpPow"},
            {BinOpKind::kMod, "OpMod"},
            {BinOpKind::kBitAnd, "OpBitAnd"},
            {BinOpKind::kXor, "OpBitXor"},
            {BinOpKind::kBitOr, "OpBitOr"},
            {BinOpKind::kRightShift, "OpRightShift"},
            {BinOpKind::kLeftShift, "OpLeftShift"},
            {BinOpKind::kConcat, "OpConcat"},
            {BinOpKind::kLess, "OpLt"},
            {BinOpKind::kLessEqual, "OpLe"},
            {BinOpKind::kMore, "OpGt"},
            {BinOpKind::kMoreEqual, "OpGe"},
            {BinOpKind::kEqual, "OpEq"},
            {BinOpKind::kNotEqual, "OpNe"},
    };

    if (const auto it = kBinOpMacros.find(op_kind); it != kBinOpMacros.end()) {
        Out() << GenTab() << std::format("{}({}, {}, {});\n", it->second, l, r, tmp);
    } else {
        ThrowError("binary operator not supported", op);
    }

    return tmp;
}

std::string CGen::CompileUnop(const SyntaxTreeInterfacePtr &exp, const SyntaxTreeInterfacePtr &op) {
    DEBUG_ASSERT(cur_section_ != Section::Globals);

    const auto op_ptr = std::dynamic_pointer_cast<SyntaxTreeUnop>(op);
    const auto op_kind = op_ptr->GetOpKind();
    const auto right = std::dynamic_pointer_cast<SyntaxTreeExp>(exp)->Right();
    // Derive result type from the operand type via GetType (respects local scope,
    // unlike the snapshot which can be stale for degraded variables).
    const auto rt = GetType(right);
    InferredType result_type;
    if (op_kind == UnOpKind::kMinus) {
        result_type = (rt == T_INT || rt == T_FLOAT) ? rt : T_DYNAMIC;
    } else if (op_kind == UnOpKind::kBitNot) {
        result_type = (rt == T_INT || rt == T_FLOAT) ? T_INT : T_DYNAMIC;
    } else {
        result_type = T_DYNAMIC;  // kNumberSign / kNot handled by slow path
    }

    // Native fast path for unary minus and bitwise not when operand is numeric.
    if (auto native_str = CompileNativeUnop(right, op_kind, result_type); !native_str.empty()) {
        return native_str;
    }

    const auto right_str = CompileExp(right);

    const auto tmp = std::format("flua_op_{}", tmp_var_counter_++);
    func_temp_decls_ << "    "
                     << "CVar " << tmp << ";\n";

    const auto r = std::format("({})", right_str);

    if (op_kind == UnOpKind::kNot) {
        Out() << GenTab() << std::format("OpNot({}, {});\n", r, tmp);
    } else if (op_kind == UnOpKind::kMinus) {
        Out() << GenTab() << std::format("OpUnaryMinus({}, {});\n", r, tmp);
    } else if (op_kind == UnOpKind::kBitNot) {
        Out() << GenTab() << std::format("OpBitNot({}, {});\n", r, tmp);
    } else if (op_kind == UnOpKind::kNumberSign) {
        Out() << GenTab() << std::format("OpLen({}, {});\n", r, tmp);
    } else {
        ThrowError("unary operator not supported", op);
    }

    return tmp;
}

std::string CGen::CompileNativeArithBinop(const SyntaxTreeInterfacePtr &left, const SyntaxTreeInterfacePtr &right, BinOpKind op_kind, InferredType result_type) {
    static const std::unordered_set<BinOpKind> kNativeArithOps = {BinOpKind::kPlus, BinOpKind::kMinus,  BinOpKind::kStar, BinOpKind::kSlash, BinOpKind::kDoubleSlash, BinOpKind::kPow,
                                                                  BinOpKind::kMod,  BinOpKind::kBitAnd, BinOpKind::kXor,  BinOpKind::kBitOr, BinOpKind::kLeftShift,   BinOpKind::kRightShift};

    if (!kNativeArithOps.contains(op_kind)) {
        return "";
    }

    DEBUG_ASSERT(result_type == T_INT || result_type == T_FLOAT);

    const auto native_expr = CompileRawNativeArithBinop(left, right, op_kind, result_type);
    DEBUG_ASSERT(!native_expr.empty());
    const auto tmp = std::format("flua_op_{}", tmp_var_counter_++);
    func_temp_decls_ << "    CVar " << tmp << ";\n";
    Out() << GenTab() << tmp << " = " << BoxNativeValue(native_expr, result_type) << ";\n";
    return tmp;
}

std::string CGen::CompileRawNativeArithBinop(const SyntaxTreeInterfacePtr &left, const SyntaxTreeInterfacePtr &right, BinOpKind op_kind, InferredType result_type) {
    const auto left_native = CompileNumericExp(left);
    const auto right_native = CompileNumericExp(right);

    const auto to_int_operand = [&](const SyntaxTreeInterfacePtr &operand_node, const std::string &native_operand) -> std::string {
        const auto operand_type = GetType(operand_node);
        if (operand_type == T_INT) {
            return std::format("(int64_t)({})", native_operand);
        }
        if (operand_type == T_FLOAT) {
            const auto itmp = std::format("flua_native_{}", tmp_var_counter_++);
            func_temp_decls_ << "    int64_t " << itmp << ";\n";
            Out() << GenTab() << std::format("FlToIntChecked(({}), {});\n", native_operand, itmp);
            return itmp;
        }
        ThrowError("bitwise operand is not numeric", operand_node);
        return {};
    };

    if (op_kind == BinOpKind::kPlus) {
        return std::format("(({}) + ({}))", left_native, right_native);
    }
    if (op_kind == BinOpKind::kMinus) {
        return std::format("(({}) - ({}))", left_native, right_native);
    }
    if (op_kind == BinOpKind::kStar) {
        return std::format("(({}) * ({}))", left_native, right_native);
    }
    if (op_kind == BinOpKind::kSlash) {
        return std::format("((double)({}) / (double)({}))", left_native, right_native);
    }
    if (op_kind == BinOpKind::kPow) {
        return std::format("pow((double)({}), (double)({}))", left_native, right_native);
    }
    if (op_kind == BinOpKind::kDoubleSlash) {
        if (result_type == T_INT) {
            const auto ntmp = std::format("flua_native_{}", tmp_var_counter_++);
            func_temp_decls_ << "    int64_t " << ntmp << ";\n";
            Out() << GenTab() << std::format("FlFloorDivInt(({}), ({}), {});\n", left_native, right_native, ntmp);
            return ntmp;
        }
        return std::format("floor((double)({}) / (double)({}))", left_native, right_native);
    }
    if (op_kind == BinOpKind::kMod) {
        if (result_type == T_INT) {
            const auto ntmp = std::format("flua_native_{}", tmp_var_counter_++);
            func_temp_decls_ << "    int64_t " << ntmp << ";\n";
            Out() << GenTab() << std::format("FlModInt(({}), ({}), {});\n", left_native, right_native, ntmp);
            return ntmp;
        }
        const auto ntmp = std::format("flua_native_{}", tmp_var_counter_++);
        func_temp_decls_ << "    double " << ntmp << ";\n";
        Out() << GenTab() << std::format("FlModFloat((double)({}), (double)({}), {});\n", left_native, right_native, ntmp);
        return ntmp;
    }
    if (op_kind == BinOpKind::kBitAnd || op_kind == BinOpKind::kBitOr || op_kind == BinOpKind::kXor || op_kind == BinOpKind::kLeftShift || op_kind == BinOpKind::kRightShift) {
        const auto left_int = to_int_operand(left, left_native);
        const auto right_int = to_int_operand(right, right_native);
        if (op_kind == BinOpKind::kBitAnd) {
            return std::format("(({}) & ({}))", left_int, right_int);
        }
        if (op_kind == BinOpKind::kBitOr) {
            return std::format("(({}) | ({}))", left_int, right_int);
        }
        if (op_kind == BinOpKind::kXor) {
            return std::format("(({}) ^ ({}))", left_int, right_int);
        }
        if (op_kind == BinOpKind::kLeftShift) {
            const auto ntmp = std::format("flua_native_{}", tmp_var_counter_++);
            func_temp_decls_ << "    int64_t " << ntmp << ";\n";
            Out() << GenTab() << std::format("FlLShiftInt(({}), ({}), {});\n", left_int, right_int, ntmp);
            return ntmp;
        }
        if (op_kind == BinOpKind::kRightShift) {
            const auto ntmp = std::format("flua_native_{}", tmp_var_counter_++);
            func_temp_decls_ << "    int64_t " << ntmp << ";\n";
            Out() << GenTab() << std::format("FlRShiftInt(({}), ({}), {});\n", left_int, right_int, ntmp);
            return ntmp;
        }
    }
    return "";
}

std::string CGen::CompileNativeCmpBinop(const SyntaxTreeInterfacePtr &left, const SyntaxTreeInterfacePtr &right, BinOpKind op_kind) {
    if (const auto cmp_it = kCmpOpMap.find(op_kind); cmp_it != kCmpOpMap.end()) {
        // Comparison requires numeric operands — consult the snapshot.
        const auto lt = GetType(left);
        if (lt == T_INT || lt == T_FLOAT) {
            const auto rt = GetType(right);
            if (rt == T_INT || rt == T_FLOAT) {
                const auto left_native = TryCompileNativeExpr(left);
                if (const auto right_native = TryCompileNativeExpr(right); !left_native.empty() && !right_native.empty()) {
                    const auto native_bool = std::format("({}) {} ({})", left_native, cmp_it->second, right_native);
                    const auto tmp = std::format("flua_op_{}", tmp_var_counter_++);
                    func_temp_decls_ << "    CVar " << tmp << ";\n";
                    Out() << GenTab() << std::format("SET_BOOL({}, {});\n", tmp, native_bool);
                    return tmp;
                }
            }
        }
    }
    return "";
}

std::string CGen::CompileNativeUnop(const SyntaxTreeInterfacePtr &right, UnOpKind op_kind, InferredType result_type) {
    if (op_kind == UnOpKind::kMinus || op_kind == UnOpKind::kBitNot) {
        if (result_type == T_INT || result_type == T_FLOAT) {
            if (const auto native_operand = TryCompileNativeExpr(right); !native_operand.empty()) {
                const auto native_expr = CompileRawNativeUnop(right, op_kind, result_type);
                const auto tmp = std::format("flua_op_{}", tmp_var_counter_++);
                func_temp_decls_ << "    CVar " << tmp << ";\n";
                Out() << GenTab() << tmp << " = " << BoxNativeValue(native_expr, result_type) << ";\n";
                return tmp;
            }
        }
    }
    return "";
}

std::string CGen::CompileRawNativeUnop(const SyntaxTreeInterfacePtr &right, UnOpKind op_kind, InferredType result_type) {
    if (op_kind == UnOpKind::kMinus || op_kind == UnOpKind::kBitNot) {
        if (result_type == T_INT || result_type == T_FLOAT) {
            const auto native_operand = CompileNumericExp(right);
            if (op_kind == UnOpKind::kMinus) {
                return std::format("(-({}))", native_operand);
            }
            if (op_kind == UnOpKind::kBitNot) {
                if (result_type == T_FLOAT) {
                    const auto itmp = std::format("flua_native_{}", tmp_var_counter_++);
                    func_temp_decls_ << "    int64_t " << itmp << ";\n";
                    Out() << GenTab() << std::format("FlToIntChecked(({}), {});\n", native_operand, itmp);
                    return std::format("(~({}))", itmp);
                }
                return std::format("(~((int64_t)({})))", native_operand);
            }
        }
    }
    return "";
}

// ---------------------------------------------------------------------------
// CompileVar —— 变量引用的代码生成
//
// 生成策略（kSimple 变量）：
// 生成策略（kSimple 变量）：
//   优先级：原生局部变量/参数作用域（RuntimeTypeTracker / GetNativeVarType）
//           > 文件级数值常量（ir().global_const_vars）
//           > 普通 CVar 变量名
//
//   前两种情形均已知为原生类型（int64_t / double），需装箱为 CVar 字面量后返回，
//   以保证所有调用方获得统一 the CVar 接口；普通 CVar 变量则直接返回变量名。
//
// kSquare（table[key]）：生成 FlGetTable(table, key) 调用。
// kDot（table.key）：将 key 字符串化后同样生成 FlGetTable 调用。
// ---------------------------------------------------------------------------
std::string CGen::CompileVar(const SyntaxTreeInterfacePtr &v) {
    DEBUG_ASSERT(v->Type() == SyntaxTreeType::Var);
    auto v_ptr = std::dynamic_pointer_cast<SyntaxTreeVar>(v);

    DEBUG_ASSERT(v_ptr->GetVarKind() == VarKind::kSimple || v_ptr->GetVarKind() == VarKind::kSquare || v_ptr->GetVarKind() == VarKind::kDot);

    if (const auto var_kind = v_ptr->GetVarKind(); var_kind == VarKind::kSimple) {
        const auto &name = v_ptr->GetName();
        DEBUG_ASSERT(cur_section_ != Section::Globals);

        // 1. Check if captured (local variable, parameter, or loop variable)
        if (const auto it = var_to_def_map_.find(v_ptr.get()); it != var_to_def_map_.end()) {
            VarDef *def = it->second;
            if (def->is_captured) {
                if (def->defining_func == cur_func_info_) {
                    return "(*__box_" + name + ")";
                } else {
                    if (cur_func_info_) {
                        const auto vit = std::ranges::find(cur_func_info_->captured_vars, def);
                        if (vit != cur_func_info_->captured_vars.end()) {
                            int idx = static_cast<int>(vit - cur_func_info_->captured_vars.begin());
                            return std::format("(*_CL->upvalues[{}])", idx);
                        }
                    }
                }
            }
        }

        // 2. Check if function referenced as value (non-direct call)
        if (local_func_names_.contains(name)) {
            const auto &info = local_func_names_.at(name);
            return std::format("FlMakeClosure(_S, (void*){}, 0, {}, {})", name, info.params_count, info.is_vararg ? "true" : "false");
        }

        // 3. Regular native variable checking
        if (const auto native_type = GetNativeVarType(name, v_ptr.get()); native_type != T_DYNAMIC) {
            return BoxNativeValue(name, native_type);
        }
        // 文件级数值常量（static const int64_t / double）：装箱为 CVar 后返回。
        if (const auto git = ir().global_const_vars.find(name); git != ir().global_const_vars.end()) {
            if (git->second == T_INT || git->second == T_FLOAT) {
                return BoxNativeValue(name, git->second);
            }
        }
        return name;
    } else if (var_kind == VarKind::kSquare) {
        DEBUG_ASSERT(cur_section_ != Section::Globals);
        const auto pe = v_ptr->GetPrefixexp();
        const auto exp = v_ptr->GetExp();
        auto pe_ret = CompilePrefixexp(pe);
        const auto spec_type = GetSpecTypeForVar(pe);

        if (const auto exp_node = std::dynamic_pointer_cast<SyntaxTreeExp>(exp)) {
            // String literal key fast path: t["key"] → FlGetTableStrId or FL_SPEC.
            if (exp_node->GetExpKind() == ExpKind::kString) {
                const auto key_name = exp_node->ExpValue();
                if (!spec_type.empty() && IsSpecField(spec_type, key_name, TableKeyKind::kString)) {
                    const auto c_name = GetSpecFieldCName(spec_type, key_name, TableKeyKind::kString);
                    return std::format("FL_SPEC({}, {}, {})", spec_type, pe_ret, c_name);
                }
                const auto id = s_->GetConstString().Alloc(key_name);
                return std::format("FlGetTableStrId({}, {})", pe_ret, id);
            }
            // Integer literal key fast path: t[1] → FlGetTableInt or FL_SPEC.
            if (exp_node->GetExpKind() == ExpKind::kNumber) {
                std::string num_str = exp_node->ExpValue();
                if (num_str.find('.') == std::string::npos && num_str.find('e') == std::string::npos && num_str.find('E') == std::string::npos) {
                    if (!spec_type.empty() && IsSpecField(spec_type, num_str, TableKeyKind::kInt)) {
                        const auto c_name = GetSpecFieldCName(spec_type, num_str, TableKeyKind::kInt);
                        return std::format("FL_SPEC({}, {}, {})", spec_type, pe_ret, c_name);
                    }
                    return std::format("FlGetTableInt({}, {})", pe_ret, num_str);
                } else {
                    if (!spec_type.empty() && IsSpecField(spec_type, num_str, TableKeyKind::kFloat)) {
                        const auto c_name = GetSpecFieldCName(spec_type, num_str, TableKeyKind::kFloat);
                        return std::format("FL_SPEC({}, {}, {})", spec_type, pe_ret, c_name);
                    }
                    return std::format("FlGetTable({}, (CVar){{.type_ = VAR_FLOAT, .data_.f = {}}})", pe_ret, num_str);
                }
            }
            // Boolean literal key fast path: t[true] → FlGetTable or FL_SPEC.
            if (exp_node->GetExpKind() == ExpKind::kTrue || exp_node->GetExpKind() == ExpKind::kFalse) {
                std::string bool_str = (exp_node->GetExpKind() == ExpKind::kTrue) ? "true" : "false";
                if (!spec_type.empty() && IsSpecField(spec_type, bool_str, TableKeyKind::kBool)) {
                    const auto c_name = GetSpecFieldCName(spec_type, bool_str, TableKeyKind::kBool);
                    return std::format("FL_SPEC({}, {}, {})", spec_type, pe_ret, c_name);
                }
                return std::format("FlGetTable({}, {})", pe_ret, bool_str == "true" ? "kTrue" : "kFalse");
            }
        }

        // Integer key fast path: use FlGetTableInt when key is known T_INT.
        if (const auto key_type = GetType(exp); key_type == T_INT) {
            if (const auto native_key = TryCompileNativeExpr(exp); !native_key.empty()) {
                return std::format("FlGetTableInt({}, {})", pe_ret, native_key);
            }
        }
        auto exp_ret = CompileExp(exp);
        return std::format("FlGetTable({}, {})", pe_ret, exp_ret);
    } else /*if (var_kind == VarKind::kDot)*/ {
        DEBUG_ASSERT(cur_section_ != Section::Globals);
        const auto pe = v_ptr->GetPrefixexp();
        const auto name = v_ptr->GetName();
        auto pe_ret = CompilePrefixexp(pe);

        const auto spec_type = GetSpecTypeForVar(pe);
        if (!spec_type.empty() && IsSpecField(spec_type, name, TableKeyKind::kString)) {
            const auto c_name = GetSpecFieldCName(spec_type, name, TableKeyKind::kString);
            return std::format("FL_SPEC({}, {}, {})", spec_type, pe_ret, c_name);
        }

        // String constant key fast path: use FlGetTableStrId directly.
        const auto id = s_->GetConstString().Alloc(name);
        return std::format("FlGetTableStrId({}, {})", pe_ret, id);
    }
}

// ===========================================================================
// 第四部分：类型推断与原生优化辅助
// ===========================================================================

// ---------------------------------------------------------------------------
// CompileNumericExp —— 将表达式编译为原生 C 数值字符串
//
// 与 CompileExp 的区别：
//   CompileExp 始终返回 CVar 类型的表达式（装箱值），
//   CompileNumericExp 返回 int64_t / double 的原生表达式，用于：
//     1. 特化函数体内的算术运算，消除 CVar 装拆箱开销；
//     2. 原生类型 for-loop 的边界/步长计算；
//     3. TryCompileNativeBoolExpr 生成原生 C 比较运算的操作数。
//
// 失败策略：若无法将表达式编译为原生数值（例如操作数为 T_DYNAMIC 的 CVar），
// 则直接抛出异常；调用方应通过 TryCompileNativeExpr 捕获并回退到 CompileExp。
// ---------------------------------------------------------------------------
std::string CGen::CompileNumericExp(const SyntaxTreeInterfacePtr &exp) {
    DEBUG_ASSERT(exp && exp->Type() == SyntaxTreeType::Exp);

    const auto e = std::dynamic_pointer_cast<SyntaxTreeExp>(exp);

    if (const auto exp_kind = e->GetExpKind(); exp_kind == ExpKind::kNumber) {
        if (LookupNodeType(e.get()) == T_INT) {
            return std::to_string(ToInteger(e->ExpValue()));
        }
        if (LookupNodeType(e.get()) == T_FLOAT) {
            return std::format("{}", ToFloat(e->ExpValue()));
        }
        ThrowError("number node is not inferred as numeric", exp);
    } else if (exp_kind == ExpKind::kPrefixExp) {
        const auto pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(e->Right());
        DEBUG_ASSERT(pe);
        if (pe->GetPrefixKind() == PrefixExpKind::kVar) {
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe->GetValue());
            DEBUG_ASSERT(var && var->GetVarKind() == VarKind::kSimple);
            const auto &vname = var->GetName();
            if (IsTypedNativeVar(vname, var.get())) {
                return vname;
            }
            // 文件级数值常量（static const int64_t / double）：直接用名称。
            if (const auto git = ir().global_const_vars.find(vname); git != ir().global_const_vars.end()) {
                if (git->second == T_INT || git->second == T_FLOAT) {
                    return vname;
                }
            }
            if (LookupNodeType(e.get()) == T_FLOAT) {
                return std::format("{}.data_.f", vname);
            }
            return std::format("{}.data_.i", vname);
        }
        if (pe->GetPrefixKind() == PrefixExpKind::kExp) {
            return CompileNumericExp(pe->GetValue());
        }
        if (pe->GetPrefixKind() == PrefixExpKind::kFunctionCall) {
            const auto inferred = GetType(exp);
            if (inferred == T_DYNAMIC) {
                ThrowError("function call cannot be specialized as numeric", exp);
            }
            if (const auto native_result = TryCompileNativeSpecCallExpr(pe->GetValue()); !native_result.empty()) {
                return native_result;
            }
            ThrowError("function call cannot be specialized as numeric", exp);
        }
    } else if (exp_kind == ExpKind::kBinop) {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeBinop>(e->Op());
        DEBUG_ASSERT(op);
        const auto op_kind = op->GetOpKind();

        if (op_kind == BinOpKind::kAnd) {
            // Numeric values are always truthy in Lua, so `a and b` = b.
            // Evaluate a for side effects only (e.g. function-call statements
            // already emitted by CompileNumericExp); the result is discarded.
            CompileNumericExp(e->Left());
            return CompileNumericExp(e->Right());
        }
        if (op_kind == BinOpKind::kOr) {
            // Pattern match Lua ternary: (cond and val1) or val2
            if (e->Left()->Type() == SyntaxTreeType::Exp) {
                if (const auto left_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(e->Left()); left_exp && left_exp->GetExpKind() == ExpKind::kBinop) {
                    if (const auto left_op = std::dynamic_pointer_cast<SyntaxTreeBinop>(left_exp->Op()); left_op && left_op->GetOpKind() == BinOpKind::kAnd) {
                        const auto cond = left_exp->Left();
                        const auto val1 = left_exp->Right();
                        const auto val2 = e->Right();

                        const auto cond_bool = TryCompileNativeBoolExpr(cond);
                        const auto cond_str = !cond_bool.empty() ? std::format("({})", cond_bool) : std::format("FlIsTrue({})", CompileExp(cond));

                        const auto val1_str = CompileNumericExp(val1);
                        const auto val2_str = CompileNumericExp(val2);
                        return std::format("({} ? {} : {})", cond_str, val1_str, val2_str);
                    }
                }
            }
            return CompileNumericExp(e->Left());
        }

        const auto result_type = LookupNodeType(e.get());
        const auto res = CompileRawNativeArithBinop(e->Left(), e->Right(), op_kind, result_type);
        if (!res.empty()) {
            return res;
        }
        ThrowError("operator is not supported in numeric specialization", exp);
    } else if (exp_kind == ExpKind::kUnop) {
        const auto op = std::dynamic_pointer_cast<SyntaxTreeUnop>(e->Op());
        DEBUG_ASSERT(op);
        const auto op_kind = op->GetOpKind();
        if (op_kind == UnOpKind::kNumberSign) {
            const auto operand_cvar = CompileExp(e->Right());
            const auto ntmp = std::format("flua_native_{}", tmp_var_counter_++);
            func_temp_decls_ << "    int64_t " << ntmp << ";\n";
            Out() << GenTab() << std::format("FlLenInt(({}), {});\n", operand_cvar, ntmp);
            return ntmp;
        }
        const auto rt = GetType(e->Right());
        const auto res = CompileRawNativeUnop(e->Right(), op_kind, rt);
        if (!res.empty()) {
            return res;
        }
        if (op_kind == UnOpKind::kBitNot) {
            ThrowError("bitwise operand is not numeric", e->Right());
        }
        ThrowError("unary operator is not supported in numeric specialization", exp);
    }

    ThrowError("unsupported numeric-specialized expression", exp);
}

// ---------------------------------------------------------------------------
// TryCompileNativeSpecCallExpr —— 将调用特化函数的结果编译为原生数值临时变量
//
// 用途：在 CompileNumericExp 遇到函数调用（kPrefixExp → kFunctionCall）时，
// 若被调函数有数学参数且该特化版本返回原生数值类型（T_INT/T_FLOAT），则：
//   1. 计算 bitmask（通过 TryInferMathCallSpec）；
//   2. 将每个数学参数实参编译为原生表达式；
//   3. 发出对应特化函数的直接调用（避免 CVar 装拆箱），
//      结果存入原生类型临时变量并返回其名称。
//
// 与 CompileFunctioncall 的区别：
//   - CompileFunctioncall 返回 CVar 类型的结果（已装箱）；
//   - TryCompileNativeSpecCallExpr 返回 int64_t/double 的原生结果，
//     可直接参与后续原生算术运算。
// ---------------------------------------------------------------------------
std::string CGen::TryCompileNativeSpecCallExpr(const SyntaxTreeInterfacePtr &functioncall_node) {
    const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(functioncall_node);
    DEBUG_ASSERT(fc);
    const auto args_node = fc->Args();
    DEBUG_ASSERT(args_node);
    const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(args_node);
    DEBUG_ASSERT(args_ptr);
    if (args_ptr->GetArgsKind() != ArgsKind::kExpList) {
        return {};
    }
    const auto callee_pe = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(fc->prefixexp());
    DEBUG_ASSERT(callee_pe && callee_pe->GetPrefixKind() == PrefixExpKind::kVar);
    const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(callee_pe->GetValue());
    DEBUG_ASSERT(callee_var && callee_var->GetVarKind() == VarKind::kSimple);
    const auto &callee_name = callee_var->GetName();
    const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(args_ptr->Explist());
    DEBUG_ASSERT(explist_ptr);
    const auto &raw_args = explist_ptr->Exps();

    int bitmask = 0;
    InferredType spec_ret = T_DYNAMIC;
    [[maybe_unused]] bool ok = TryInferMathCallSpec(callee_name, raw_args, bitmask, spec_ret);
    DEBUG_ASSERT(ok);
    DEBUG_ASSERT(spec_ret == T_INT || spec_ret == T_FLOAT);

    const auto &math_params = ir().math_param_positions.at(callee_name);

    std::unordered_map<int, std::string> native_exprs;
    for (int param_pos: math_params) {
        const auto native_expr = TryCompileNativeExpr(raw_args[param_pos]);
        DEBUG_ASSERT(!native_expr.empty());
        native_exprs[param_pos] = native_expr;
    }

    const auto spec_name = SpecFuncName(callee_name, math_params, bitmask);
    std::string call = spec_name + "(";
    for (int i = 0; i < static_cast<int>(raw_args.size()); ++i) {
        if (i > 0) {
            call += ", ";
        }
        if (const auto ne_it = native_exprs.find(i); ne_it != native_exprs.end()) {
            call += ne_it->second;
        } else {
            call += CompileExp(raw_args[i]);
        }
    }
    call += ")";

    const auto ntmp = std::format("flua_native_{}", tmp_var_counter_++);
    func_temp_decls_ << "    " << SpecReturnCTypeName(spec_ret) << " " << ntmp << ";\n";
    Out() << GenTab() << ntmp << " = " << call << ";\n";
    return ntmp;
}

// ---------------------------------------------------------------------------
// CompileFunctioncall —— 函数调用的代码生成
//
// 生成策略（按优先级）：
//
//   1. 特化直接调用（fast path）：
//      若被调函数是同文件的数学函数（math_param_positions_ 中存在），
//      且所有数学参数实参的类型均已知（TryInferMathCallBitmask 成功），
//      则尝试将数学参数编译为原生表达式（TryCompileNativeExpr），
//      并发出对应特化函数（SpecFuncName）的直接调用，避免走 CVar 入口分发器，
//      消除运行时类型检查和分发开销。若特化函数返回原生类型，则装箱后存入 CVar tmp。
//
//   2. 普通路径（slow path）：
//      将所有参数编译为 CVar，然后发出函数名(arg0, arg1, ...) 调用。
//      - 同文件的普通函数：直接调用（local_func_names_ 中存在）。
//      - 跨文件/内置函数：通过 FakeluaCallByName 动态分发（带字符串函数名）。
//      - 特殊内置宏（FAKELUA_SET_TABLE）：生成 FlSetTable 调用。
//
// 返回值：CVar 类型的临时变量名，供调用方（CompileExp/CompileNumericExp）使用。
// ---------------------------------------------------------------------------
std::string CGen::CompileFunctioncall(const SyntaxTreeInterfacePtr &functioncall) {
    DEBUG_ASSERT(cur_section_ != Section::Globals);

    DEBUG_ASSERT(functioncall->Type() == SyntaxTreeType::FunctionCall);
    const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(functioncall);

    const auto args_node = fc->Args();
    DEBUG_ASSERT(args_node->Type() == SyntaxTreeType::Args);
    const auto args_ptr = std::dynamic_pointer_cast<SyntaxTreeArgs>(args_node);
    const auto args_kind = args_ptr->GetArgsKind();

    const auto pe_pre = fc->prefixexp();
    DEBUG_ASSERT(pe_pre->Type() == SyntaxTreeType::PrefixExp);
    const auto pe_pre_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(pe_pre);

    // 尝试直接调用优化：若被调函数是含有数学参数的本地函数，
    // 且所有数学参数的实参类型均已知，则直接发出特化调用。
    if (pe_pre_ptr->GetPrefixKind() == PrefixExpKind::kVar && args_kind == ArgsKind::kExpList) {
        if (const auto callee_var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe_pre_ptr->GetValue()); callee_var && callee_var->GetVarKind() == VarKind::kSimple) {
            const auto &callee_name = callee_var->GetName();
            if (const auto math_it = ir().math_param_positions.find(callee_name); math_it != ir().math_param_positions.end()) {
                const auto &math_params = math_it->second;
                const auto explist_arg = args_ptr->Explist();
                DEBUG_ASSERT(explist_arg->Type() == SyntaxTreeType::ExpList);
                const auto explist_arg_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist_arg);
                const auto &raw_args = explist_arg_ptr->Exps();

                if (int bitmask = 0; TryInferMathCallBitmask(callee_name, raw_args, bitmask)) {
                    std::unordered_map<int, std::string> native_exprs;
                    for (int param_pos: math_params) {
                        const auto native_expr = TryCompileNativeExpr(raw_args[param_pos]);
                        DEBUG_ASSERT(!native_expr.empty());
                        native_exprs[param_pos] = native_expr;
                    }

                    const auto spec_name = SpecFuncName(callee_name, math_params, bitmask);
                    std::string call = spec_name + "(";
                    for (int i = 0; i < static_cast<int>(raw_args.size()); ++i) {
                        if (i > 0) {
                            call += ", ";
                        }
                        if (const auto ne_it = native_exprs.find(i); ne_it != native_exprs.end()) {
                            call += ne_it->second;
                        } else {
                            call += CompileExp(raw_args[i]);
                        }
                    }
                    call += ")";
                    const auto tmp = std::format("flua_call_{}", tmp_var_counter_++);
                    func_temp_decls_ << "    CVar " << tmp << ";\n";
                    if (const auto spec_ret = GetSpecReturnType(callee_name, bitmask); spec_ret == T_INT || spec_ret == T_FLOAT) {
                        const auto ntmp = std::format("flua_native_{}", tmp_var_counter_++);
                        func_temp_decls_ << "    " << SpecReturnCTypeName(spec_ret) << " " << ntmp << ";\n";
                        Out() << GenTab() << ntmp << " = " << call << ";\n";
                        Out() << GenTab() << tmp << " = " << BoxNativeValue(ntmp, spec_ret) << ";\n";
                    } else {
                        Out() << GenTab() << tmp << " = " << call << ";\n";
                    }
                    return tmp;
                }
            }
        }
    }

    // FAKELUA_SET_TABLE fast path: detect early before compiling all args to CVar,
    // so we can use FlSetTableInt/FlSetTableStrId when the key type is known.
    if (pe_pre_ptr->GetPrefixKind() == PrefixExpKind::kVar && args_kind == ArgsKind::kExpList) {
        if (const auto early_var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe_pre_ptr->GetValue());
            early_var && early_var->GetVarKind() == VarKind::kSimple && early_var->GetName() == "FAKELUA_SET_TABLE") {
            const auto explist_node = args_ptr->Explist();
            DEBUG_ASSERT(explist_node->Type() == SyntaxTreeType::ExpList);
            const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist_node);
            const auto &raw_args = explist_ptr->Exps();
            if (raw_args.size() != 3) {
                ThrowError("FAKELUA_SET_TABLE expects exactly 3 arguments", functioncall);
            }
            const auto tmp = std::format("flua_call_{}", tmp_var_counter_++);
            func_temp_decls_ << "    CVar " << tmp << ";\n";

            // table 特化快速路径：通过 spec 指针直接设置成员
            if (const auto key_exp = std::dynamic_pointer_cast<SyntaxTreeExp>(raw_args[1])) {
                const auto tbl_str = CompileExp(raw_args[0]);
                const auto val_str = CompileExp(raw_args[2]);
                const auto spec_type = GetSpecTypeForVar(raw_args[0]);

                bool is_spec = false;
                std::string c_field_name;
                int index = -1;

                if (key_exp->GetExpKind() == ExpKind::kString) {
                    const auto key_name = key_exp->ExpValue();
                    if (!spec_type.empty() && IsSpecField(spec_type, key_name, TableKeyKind::kString)) {
                        is_spec = true;
                        c_field_name = GetSpecFieldCName(spec_type, key_name, TableKeyKind::kString);
                        index = GetSpecFieldIndex(spec_type, key_name, TableKeyKind::kString);
                    }
                    if (is_spec) {
                        const auto tmp_val = std::format("flua_spec_val_{}", tmp_var_counter_++);
                        func_temp_decls_ << "    CVar " << tmp_val << ";\n";
                        Out() << GenTab() << tmp_val << " = " << val_str << ";\n";
                        Out() << GenTab() << std::format("FL_SET_SPEC({}, {}, {}, {}, {});\n", spec_type, tbl_str, c_field_name, index, tmp_val);
                    } else {
                        const auto id = s_->GetConstString().Alloc(key_name);
                        Out() << GenTab() << std::format("FlSetTableStrId({}, {}, {});\n", tbl_str, id, val_str);
                    }
                    Out() << GenTab() << std::format("SET_NIL({});\n", tmp);
                    return tmp;
                } else if (key_exp->GetExpKind() == ExpKind::kNumber) {
                    std::string num_str = key_exp->ExpValue();
                    if (num_str.find('.') == std::string::npos && num_str.find('e') == std::string::npos && num_str.find('E') == std::string::npos) {
                        if (!spec_type.empty() && IsSpecField(spec_type, num_str, TableKeyKind::kInt)) {
                            is_spec = true;
                            c_field_name = GetSpecFieldCName(spec_type, num_str, TableKeyKind::kInt);
                            index = GetSpecFieldIndex(spec_type, num_str, TableKeyKind::kInt);
                        }
                        if (is_spec) {
                            const auto tmp_val = std::format("flua_spec_val_{}", tmp_var_counter_++);
                            func_temp_decls_ << "    CVar " << tmp_val << ";\n";
                            Out() << GenTab() << tmp_val << " = " << val_str << ";\n";
                            Out() << GenTab() << std::format("FL_SET_SPEC({}, {}, {}, {}, {});\n", spec_type, tbl_str, c_field_name, index, tmp_val);
                        } else {
                            Out() << GenTab() << std::format("FlSetTableInt({}, {}, {});\n", tbl_str, num_str, val_str);
                        }
                    } else {
                        if (!spec_type.empty() && IsSpecField(spec_type, num_str, TableKeyKind::kFloat)) {
                            is_spec = true;
                            c_field_name = GetSpecFieldCName(spec_type, num_str, TableKeyKind::kFloat);
                            index = GetSpecFieldIndex(spec_type, num_str, TableKeyKind::kFloat);
                        }
                        if (is_spec) {
                            const auto tmp_val = std::format("flua_spec_val_{}", tmp_var_counter_++);
                            func_temp_decls_ << "    CVar " << tmp_val << ";\n";
                            Out() << GenTab() << tmp_val << " = " << val_str << ";\n";
                            Out() << GenTab() << std::format("FL_SET_SPEC({}, {}, {}, {}, {});\n", spec_type, tbl_str, c_field_name, index, tmp_val);
                        } else {
                            Out() << GenTab() << std::format("FlSetTable({}, (CVar){{.type_ = VAR_FLOAT, .data_.f = {}}}, {});\n", tbl_str, num_str, val_str);
                        }
                    }
                    Out() << GenTab() << std::format("SET_NIL({});\n", tmp);
                    return tmp;
                } else if (key_exp->GetExpKind() == ExpKind::kTrue || key_exp->GetExpKind() == ExpKind::kFalse) {
                    std::string bool_str = (key_exp->GetExpKind() == ExpKind::kTrue) ? "true" : "false";
                    if (!spec_type.empty() && IsSpecField(spec_type, bool_str, TableKeyKind::kBool)) {
                        is_spec = true;
                        c_field_name = GetSpecFieldCName(spec_type, bool_str, TableKeyKind::kBool);
                        index = GetSpecFieldIndex(spec_type, bool_str, TableKeyKind::kBool);
                    }
                    if (is_spec) {
                        const auto tmp_val = std::format("flua_spec_val_{}", tmp_var_counter_++);
                        func_temp_decls_ << "    CVar " << tmp_val << ";\n";
                        Out() << GenTab() << tmp_val << " = " << val_str << ";\n";
                        Out() << GenTab() << std::format("FL_SET_SPEC({}, {}, {}, {}, {});\n", spec_type, tbl_str, c_field_name, index, tmp_val);
                    } else {
                        Out() << GenTab() << std::format("FlSetTable({}, (CVar){{.type_ = VAR_BOOL, .data_.b = {}}}, {});\n", tbl_str, bool_str == "true" ? "true" : "false", val_str);
                    }
                    Out() << GenTab() << std::format("SET_NIL({});\n", tmp);
                    return tmp;
                }
            }
            // Check if key is a known integer type → use FlSetTableInt.
            if (const auto key_type = GetType(raw_args[1]); key_type == T_INT) {
                if (const auto native_key = TryCompileNativeExpr(raw_args[1]); !native_key.empty()) {
                    const auto tbl_str = CompileExp(raw_args[0]);
                    const auto val_str = CompileExp(raw_args[2]);
                    Out() << GenTab() << std::format("FlSetTableInt({}, {}, {});\n", tbl_str, native_key, val_str);
                    Out() << GenTab() << std::format("SET_NIL({});\n", tmp);
                    return tmp;
                }
            }
            // Fallback: compile all args as CVar and use generic FlSetTable.
            const auto tbl_str = CompileExp(raw_args[0]);
            const auto key_str = CompileExp(raw_args[1]);
            const auto val_str = CompileExp(raw_args[2]);
            Out() << GenTab() << std::format("FlSetTable({}, {}, {});\n", tbl_str, key_str, val_str);
            Out() << GenTab() << std::format("SET_NIL({});\n", tmp);
            return tmp;
        }
    }

    // 普通路径：将所有参数编译为 CVar。
    std::vector<std::string> compiled_args;
    bool has_expansion = false;
    std::string expansion_tmp;
    int expansion_start_idx = 0;

    if (args_kind == ArgsKind::kExpList) {
        const auto explist = args_ptr->Explist();
        DEBUG_ASSERT(explist->Type() == SyntaxTreeType::ExpList);
        const auto explist_ptr = std::dynamic_pointer_cast<SyntaxTreeExplist>(explist);
        const auto &raw_args = explist_ptr->Exps();

        bool last_is_func = !raw_args.empty() && ar().function_call_exps.contains(raw_args.back().get());
        bool last_is_vararg = !raw_args.empty() && IsVarargExp(raw_args.back());
        std::string last_callee = last_is_func ? ar().callee_names.at(raw_args.back().get()) : "";
        bool is_last_single_return_local = !last_callee.empty() && ar().function_max_returns.contains(last_callee) && ar().function_max_returns.at(last_callee) == 1;

        if ((last_is_func && !is_last_single_return_local) || last_is_vararg) {
            has_expansion = true;
            // Compile prior arguments first
            for (size_t i = 0; i < raw_args.size() - 1; ++i) {
                compiled_args.push_back(CompileExp(raw_args[i]));
            }
            // Compile the last function call/vararg into a temporary variable
            expansion_tmp = std::format("flua_call_res_{}", tmp_var_counter_++);
            func_temp_decls_ << "    CVar " << expansion_tmp << ";\n";
            Out() << GenTab() << expansion_tmp << " = " << CompileExp(raw_args.back(), true) << ";\n";
            expansion_start_idx = raw_args.size() - 1;
        } else {
            for (const auto &exp: raw_args) {
                compiled_args.push_back(CompileExp(exp));
            }
        }
    } else if (args_kind == ArgsKind::kTableConstructor) {
        compiled_args.push_back(CompileTableconstructor(args_ptr->Tableconstructor()));
    } else if (args_kind == ArgsKind::kString) {
        compiled_args.push_back(CompileExp(args_ptr->String()));
    }
    std::string func_name;
    const SyntaxTreeVar *var_ptr = nullptr;
    std::shared_ptr<SyntaxTreeVar> var;
    if (pe_pre_ptr->GetPrefixKind() == PrefixExpKind::kVar) {
        var = std::dynamic_pointer_cast<SyntaxTreeVar>(pe_pre_ptr->GetValue());
        if (var && var->GetVarKind() == VarKind::kSimple) {
            func_name = var->GetName();
            var_ptr = var.get();
        } else if (var && var->GetVarKind() == VarKind::kDot) {
            const auto base_pe = var->GetPrefixexp();
            if (base_pe && base_pe->Type() == SyntaxTreeType::PrefixExp) {
                const auto base_pe_ptr = std::dynamic_pointer_cast<SyntaxTreePrefixexp>(base_pe);
                if (base_pe_ptr && base_pe_ptr->GetPrefixKind() == PrefixExpKind::kVar && base_pe_ptr->GetValue()) {
                    const auto base_var = std::dynamic_pointer_cast<SyntaxTreeVar>(base_pe_ptr->GetValue());
                    if (base_var && base_var->GetVarKind() == VarKind::kSimple) {
                        bool is_base_local = false;
                        const std::string &bname = base_var->GetName();
                        for (const auto &[vptr, def] : var_to_def_map_) {
                            if (def && def->name == bname) {
                                is_base_local = true;
                                break;
                            }
                        }
                        if (!is_base_local) {
                            func_name = base_var->GetName() + "." + var->GetName();
                        }
                    }
                }
            }
        }
    }

    if (!func_name.empty() && func_name == "FAKELUA_SET_TABLE") {
        if (compiled_args.size() != 3) {
            ThrowError("FAKELUA_SET_TABLE expects exactly 3 arguments", functioncall);
        }
        ThrowError("FAKELUA_SET_TABLE should have been handled by early fast path", functioncall);
        return "";
    }

    std::string call_expr;
    bool is_local_callee = false;
    if (var_ptr) {
        if (const auto it = var_to_def_map_.find(var_ptr); it != var_to_def_map_.end()) {
            is_local_callee = true;
        }
    }

    if (local_func_names_.contains(func_name)) {
        const auto &info = local_func_names_.at(func_name);
        if (!info.is_vararg && !has_expansion) {
            if (static_cast<int>(compiled_args.size()) != info.params_count) {
                ThrowError(std::format("wrong number of arguments to '{}': expected {}, got {}", func_name, info.params_count, compiled_args.size()), functioncall);
            }
        }
    }

    if (local_func_names_.contains(func_name) && !is_local_callee) {
        const auto &info = local_func_names_.at(func_name);
        if (info.is_vararg) {
            int N = info.params_count;
            int fixed_param_count = N - 1;
            if (has_expansion) {
                if (expansion_start_idx < fixed_param_count) {
                    for (int i = expansion_start_idx; i < fixed_param_count; ++i) {
                        compiled_args.push_back(std::format("FlUnboxMulti({}, {})", expansion_tmp, i - expansion_start_idx));
                    }
                    std::string slice_expr = std::format("FlSliceMulti(_S, {}, {})", expansion_tmp, fixed_param_count - expansion_start_idx);
                    compiled_args.push_back(slice_expr);
                } else if (expansion_start_idx == fixed_param_count) {
                    compiled_args.push_back(expansion_tmp);
                } else { // expansion_start_idx > fixed_param_count
                    std::vector<std::string> prefix_args;
                    for (int i = fixed_param_count; i < expansion_start_idx; ++i) {
                        prefix_args.push_back(compiled_args[i]);
                    }
                    compiled_args.resize(fixed_param_count);
                    std::string prefix_arr_name = std::format("flua_vararg_prefix_{}", tmp_var_counter_++);
                    func_temp_decls_ << "    CVar " << prefix_arr_name << "[" << prefix_args.size() << "];\n";
                    for (size_t i = 0; i < prefix_args.size(); ++i) {
                        Out() << GenTab() << prefix_arr_name << "[" << i << "] = " << prefix_args[i] << ";\n";
                    }
                    std::string combine_expr = std::format("FlCombineMulti(_S, {}, {}, {})", prefix_args.size(), prefix_arr_name, expansion_tmp);
                    compiled_args.push_back(combine_expr);
                }
            } else {
                int num_varargs = static_cast<int>(compiled_args.size()) > fixed_param_count ? static_cast<int>(compiled_args.size()) - fixed_param_count : 0;
                if (num_varargs == 0) {
                    while (static_cast<int>(compiled_args.size()) < fixed_param_count) {
                        compiled_args.push_back("kNil");
                    }
                    compiled_args.push_back("kNil");
                } else {
                    std::string pack = std::format("FlMakeMulti(_S, {}", num_varargs);
                    for (int i = fixed_param_count; i < static_cast<int>(compiled_args.size()); ++i) {
                        pack += ", " + compiled_args[i];
                    }
                    pack += ")";
                    compiled_args.resize(fixed_param_count);
                    compiled_args.push_back(pack);
                }
            }
        } else {
            const int expected_params = info.params_count;
            if (has_expansion) {
                for (int i = expansion_start_idx; i < expected_params; ++i) {
                    compiled_args.push_back(std::format("FlUnboxMulti({}, {})", expansion_tmp, i - expansion_start_idx));
                }
            }
            if (static_cast<int>(compiled_args.size()) != expected_params) {
                ThrowError(std::format("wrong number of arguments to '{}': expected {}, got {}", func_name, expected_params, compiled_args.size()), functioncall);
            }
        }
        call_expr = func_name + "(NULL";
        for (size_t i = 0; i < compiled_args.size(); ++i) {
            call_expr += ", " + compiled_args[i];
        }
        call_expr += ")";
    } else if (!fc->Name().empty()) {
        if (has_expansion) {
            compiled_args.push_back(expansion_tmp);
        }
        const std::string &method_name = fc->Name();
        std::string obj_expr;
        if (var) {
            obj_expr = CompileVar(var);
        } else {
            obj_expr = CompilePrefixexp(pe_pre_ptr);
        }
        std::string obj_tmp = std::format("flua_obj_{}", tmp_var_counter_++);
        func_temp_decls_ << "    CVar " << obj_tmp << ";\n";
        Out() << GenTab() << obj_tmp << " = " << obj_expr << ";\n";

        std::string callee_expr;
        const auto spec_type = GetSpecTypeForVar(pe_pre_ptr);
        if (!spec_type.empty() && IsSpecField(spec_type, method_name, TableKeyKind::kString)) {
            const auto c_name = GetSpecFieldCName(spec_type, method_name, TableKeyKind::kString);
            callee_expr = std::format("FL_SPEC({}, {}, {})", spec_type, obj_tmp, c_name);
        } else {
            const auto id = s_->GetConstString().Alloc(method_name);
            callee_expr = std::format("FlGetTableStrId({}, {})", obj_tmp, id);
        }

        std::string callee_tmp = std::format("flua_method_{}", tmp_var_counter_++);
        func_temp_decls_ << "    CVar " << callee_tmp << ";\n";
        Out() << GenTab() << callee_tmp << " = " << callee_expr << ";\n";

        std::vector<std::string> final_args;
        final_args.push_back(obj_tmp);
        for (const auto &arg : compiled_args) {
            final_args.push_back(arg);
        }

        call_expr = std::format("FlCallClosure(_S, {}, {}", callee_tmp, final_args.size());
        for (const auto &arg : final_args) {
            call_expr += ", " + arg;
        }
        call_expr += ")";
    } else {
        if (has_expansion) {
            compiled_args.push_back(expansion_tmp);
        }
        if (!func_name.empty() && !is_local_callee) {
            call_expr = std::format("FakeluaCallByName(_S, FAKELUA_JIT_TYPE, \"{}\", {}", func_name, compiled_args.size());
            for (const auto &arg: compiled_args) {
                call_expr += ", " + arg;
            }
            call_expr += ")";
        } else {
            std::string callee_expr;
            if (var) {
                callee_expr = CompileVar(var);
            } else {
                callee_expr = CompilePrefixexp(pe_pre);
            }
            call_expr = std::format("FlCallClosure(_S, {}, {}", callee_expr, compiled_args.size());
            for (const auto &arg: compiled_args) {
                call_expr += ", " + arg;
            }
            call_expr += ")";
        }
    }
    const auto tmp = std::format("flua_call_{}", tmp_var_counter_++);
    func_temp_decls_ << "    "
                     << "CVar " << tmp << ";\n";
    Out() << GenTab() << tmp << " = " << call_expr << ";\n";

    return tmp;
}


void CGen::ResolveScopes(const SyntaxTreeInterfacePtr &node,
                         std::vector<Scope> &scopes,
                         std::vector<FuncInfo *> &func_stack,
                         FuncInfo *cur_func) {
    if (!node) return;

    auto enter_scope = [&]() {
        Scope s;
        s.func = cur_func;
        scopes.push_back(s);
    };
    auto exit_scope = [&]() {
        scopes.pop_back();
    };
    auto define_var = [&](const std::string &name, const SyntaxTreeInterface *def_node) {
        auto def = std::make_unique<VarDef>();
        def->name = name;
        def->def_node = def_node;
        def->defining_func = cur_func;
        def->is_captured = false;
        VarDef *ptr = def.get();
        scopes.back().vars[name] = ptr;
        all_defs_.push_back(std::move(def));
        stmt_var_to_def_[{def_node, name}] = ptr;
    };

    switch (node->Type()) {
        case SyntaxTreeType::Block: {
            enter_scope();
            const auto block = std::dynamic_pointer_cast<SyntaxTreeBlock>(node);
            for (const auto &stmt : block->Stmts()) {
                ResolveScopes(stmt, scopes, func_stack, cur_func);
            }
            exit_scope();
            break;
        }
        case SyntaxTreeType::LocalVar: {
            const auto lv = std::dynamic_pointer_cast<SyntaxTreeLocalVar>(node);
            ResolveScopes(lv->Explist(), scopes, func_stack, cur_func);
            if (const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(lv->Namelist())) {
                for (const auto &name : nl->Names()) {
                    define_var(name, lv.get());
                }
            }
            break;
        }
        case SyntaxTreeType::ForLoop: {
            const auto fl = std::dynamic_pointer_cast<SyntaxTreeForLoop>(node);
            ResolveScopes(fl->ExpBegin(), scopes, func_stack, cur_func);
            ResolveScopes(fl->ExpEnd(), scopes, func_stack, cur_func);
            ResolveScopes(fl->ExpStep(), scopes, func_stack, cur_func);
            enter_scope();
            define_var(fl->Name(), fl.get());
            ResolveScopes(fl->Block(), scopes, func_stack, cur_func);
            exit_scope();
            break;
        }
        case SyntaxTreeType::ForIn: {
            const auto fi = std::dynamic_pointer_cast<SyntaxTreeForIn>(node);
            ResolveScopes(fi->Explist(), scopes, func_stack, cur_func);
            enter_scope();
            if (const auto nl = std::dynamic_pointer_cast<SyntaxTreeNamelist>(fi->Namelist())) {
                for (const auto &name : nl->Names()) {
                    define_var(name, fi.get());
                }
            }
            ResolveScopes(fi->Block(), scopes, func_stack, cur_func);
            exit_scope();
            break;
        }
        case SyntaxTreeType::Function:
        case SyntaxTreeType::LocalFunction:
        case SyntaxTreeType::FunctionDef: {
            auto new_func = std::make_unique<FuncInfo>();
            new_func->node = node.get();
            new_func->parent = cur_func;
            
            std::string orig_name = "";
            if (node->Type() == SyntaxTreeType::Function) {
                orig_name = CompileFuncName(std::dynamic_pointer_cast<SyntaxTreeFunction>(node)->Funcname());
            } else if (node->Type() == SyntaxTreeType::LocalFunction) {
                orig_name = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node)->Name();
            } else {
                orig_name = std::format("__fakelua_lambda_{}", all_funcs_.size());
            }
            new_func->name = orig_name;
            
            if (cur_func != nullptr) {
                new_func->unique_c_name = std::format("__fl_func_{}", all_funcs_.size());
            } else {
                new_func->unique_c_name = orig_name;
            }

            SyntaxTreeInterfacePtr funcbody;
            if (node->Type() == SyntaxTreeType::Function) {
                funcbody = std::dynamic_pointer_cast<SyntaxTreeFunction>(node)->Funcbody();
            } else if (node->Type() == SyntaxTreeType::LocalFunction) {
                funcbody = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(node)->Funcbody();
            } else {
                funcbody = std::dynamic_pointer_cast<SyntaxTreeFunctiondef>(node)->Funcbody();
            }
            new_func->funcbody = funcbody;
            if (funcbody) {
                const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
                if (const auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(fb->Parlist())) {
                    if (const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(parlist->Namelist())) {
                        new_func->params = namelist->Names();
                    }
                    new_func->is_vararg = !new_func->params.empty() && (new_func->params.back().rfind("__fakelua_vararg_", 0) == 0);
                }
            }

            FuncInfo *pf = new_func.get();
            func_map_[node.get()] = pf;
            all_funcs_.push_back(std::move(new_func));

            if (node->Type() == SyntaxTreeType::LocalFunction) {
                define_var(orig_name, node.get());
            }

            func_stack.push_back(pf);
            enter_scope();
            if (funcbody) {
                const auto fb = std::dynamic_pointer_cast<SyntaxTreeFuncbody>(funcbody);
                if (const auto parlist = std::dynamic_pointer_cast<SyntaxTreeParlist>(fb->Parlist())) {
                    if (const auto namelist = std::dynamic_pointer_cast<SyntaxTreeNamelist>(parlist->Namelist())) {
                        for (const auto &pname : namelist->Names()) {
                            auto def = std::make_unique<VarDef>();
                            def->name = pname;
                            def->def_node = parlist.get();
                            def->defining_func = pf;
                            VarDef *ptr = def.get();
                            all_defs_.push_back(std::move(def));
                            scopes.back().vars[pname] = ptr;
                            stmt_var_to_def_[{parlist.get(), pname}] = ptr;
                        }
                    }
                }
                ResolveScopes(fb->Block(), scopes, func_stack, pf);
            }
            exit_scope();
            func_stack.pop_back();
            break;
        }
        case SyntaxTreeType::Var: {
            const auto var = std::dynamic_pointer_cast<SyntaxTreeVar>(node);
            if (var->GetVarKind() == VarKind::kSimple) {
                const std::string &name = var->GetName();
                VarDef *found_def = nullptr;
                for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i) {
                    if (const auto it = scopes[i].vars.find(name); it != scopes[i].vars.end()) {
                        found_def = it->second;
                        break;
                    }
                }
                if (found_def) {
                    var_to_def_map_[var.get()] = found_def;
                    FuncInfo *f_def = found_def->defining_func;
                    if (f_def != nullptr && f_def != cur_func) {
                        found_def->is_captured = true;
                        FuncInfo *p = cur_func;
                        while (p && p != f_def) {
                            if (p->captured_set.insert(found_def).second) {
                                p->captured_vars.push_back(found_def);
                            }
                            p = p->parent;
                        }
                    }
                }
            }
            ResolveScopes(var->GetPrefixexp(), scopes, func_stack, cur_func);
            ResolveScopes(var->GetExp(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::Return: {
            ResolveScopes(std::dynamic_pointer_cast<SyntaxTreeReturn>(node)->Explist(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::VarList: {
            const auto vl = std::dynamic_pointer_cast<SyntaxTreeVarlist>(node);
            for (const auto &v : vl->Vars()) {
                ResolveScopes(v, scopes, func_stack, cur_func);
            }
            break;
        }
        case SyntaxTreeType::ExpList: {
            const auto el = std::dynamic_pointer_cast<SyntaxTreeExplist>(node);
            for (const auto &exp : el->Exps()) {
                ResolveScopes(exp, scopes, func_stack, cur_func);
            }
            break;
        }
        case SyntaxTreeType::Assign: {
            const auto assign = std::dynamic_pointer_cast<SyntaxTreeAssign>(node);
            ResolveScopes(assign->Varlist(), scopes, func_stack, cur_func);
            ResolveScopes(assign->Explist(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::FunctionCall: {
            const auto fc = std::dynamic_pointer_cast<SyntaxTreeFunctioncall>(node);
            ResolveScopes(fc->prefixexp(), scopes, func_stack, cur_func);
            ResolveScopes(fc->Args(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::Args: {
            const auto args = std::dynamic_pointer_cast<SyntaxTreeArgs>(node);
            ResolveScopes(args->Explist(), scopes, func_stack, cur_func);
            ResolveScopes(args->Tableconstructor(), scopes, func_stack, cur_func);
            ResolveScopes(args->String(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::TableConstructor: {
            ResolveScopes(std::dynamic_pointer_cast<SyntaxTreeTableconstructor>(node)->Fieldlist(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::FieldList: {
            const auto fl = std::dynamic_pointer_cast<SyntaxTreeFieldlist>(node);
            for (const auto &field : fl->Fields()) {
                ResolveScopes(field, scopes, func_stack, cur_func);
            }
            break;
        }
        case SyntaxTreeType::Field: {
            const auto field = std::dynamic_pointer_cast<SyntaxTreeField>(node);
            ResolveScopes(field->Key(), scopes, func_stack, cur_func);
            ResolveScopes(field->Value(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::While: {
            const auto while_node = std::dynamic_pointer_cast<SyntaxTreeWhile>(node);
            ResolveScopes(while_node->Exp(), scopes, func_stack, cur_func);
            ResolveScopes(while_node->Block(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::Repeat: {
            const auto rep = std::dynamic_pointer_cast<SyntaxTreeRepeat>(node);
            ResolveScopes(rep->Block(), scopes, func_stack, cur_func);
            ResolveScopes(rep->Exp(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::If: {
            const auto if_node = std::dynamic_pointer_cast<SyntaxTreeIf>(node);
            ResolveScopes(if_node->Exp(), scopes, func_stack, cur_func);
            ResolveScopes(if_node->Block(), scopes, func_stack, cur_func);
            ResolveScopes(if_node->ElseIfs(), scopes, func_stack, cur_func);
            ResolveScopes(if_node->ElseBlock(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::ElseIfList: {
            const auto eil = std::dynamic_pointer_cast<SyntaxTreeElseiflist>(node);
            for (size_t i = 0; i < eil->ElseifSize(); ++i) {
                ResolveScopes(eil->ElseifExp(i), scopes, func_stack, cur_func);
                ResolveScopes(eil->ElseifBlock(i), scopes, func_stack, cur_func);
            }
            break;
        }
        case SyntaxTreeType::Exp: {
            const auto exp = std::dynamic_pointer_cast<SyntaxTreeExp>(node);
            ResolveScopes(exp->Left(), scopes, func_stack, cur_func);
            ResolveScopes(exp->Right(), scopes, func_stack, cur_func);
            break;
        }
        case SyntaxTreeType::PrefixExp: {
            ResolveScopes(std::dynamic_pointer_cast<SyntaxTreePrefixexp>(node)->GetValue(), scopes, func_stack, cur_func);
            break;
        }
        default:
            break;
    }
}

std::string CGen::CompileUpvaluePointer(VarDef *def) {
    if (def->defining_func == cur_func_info_) {
        return "__box_" + def->name;
    } else {
        if (cur_func_info_) {
            const auto vit = std::ranges::find(cur_func_info_->captured_vars, def);
            if (vit != cur_func_info_->captured_vars.end()) {
                int idx = static_cast<int>(vit - cur_func_info_->captured_vars.begin());
                return std::format("_CL->upvalues[{}]", idx);
            }
        }
    }
    return "NULL";
}

void CGen::CompileStmtLocalFunction(const SyntaxTreeInterfacePtr &stmt) {
    DEBUG_ASSERT(stmt->Type() == SyntaxTreeType::LocalFunction);
    const auto lf = std::dynamic_pointer_cast<SyntaxTreeLocalFunction>(stmt);
    const auto &name = lf->Name();
    
    FuncInfo *func = func_map_[lf.get()];
    
    bool is_captured = false;
    if (const auto it = stmt_var_to_def_.find({lf.get(), name}); it != stmt_var_to_def_.end()) {
        is_captured = it->second->is_captured;
    }
    
    std::string closure_expr = std::format("FlMakeClosure(_S, (void*){}, {}, {}, {}",
                                           func->unique_c_name,
                                           func->captured_vars.size(),
                                           func->params.size(),
                                           func->is_vararg ? "true" : "false");
    for (VarDef *up : func->captured_vars) {
        closure_expr += ", " + CompileUpvaluePointer(up);
    }
    closure_expr += ")";
    
    if (is_captured) {
        Out() << GenTab() << "CVar *__box_" << name << " = (CVar *)FakeluaAlloc(_S, sizeof(CVar), false);\n";
        Out() << GenTab() << "*__box_" << name << " = " << closure_expr << ";\n";
    } else {
        Out() << GenTab() << "CVar " << name << " = " << closure_expr << ";\n";
    }
}


}// namespace fakelua
