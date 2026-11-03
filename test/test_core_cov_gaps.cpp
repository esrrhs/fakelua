#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
#include "fakelua.h"
#include "var/var_type.h"
#include "test_jit.h"
#include "util/logging.h"
#include "util/number_util.h"
#include "util/os.h"
#include "util/string_util.h"
#include "gtest/gtest.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <vector>

using namespace fakelua;

extern "C" void FakeluaLogLua(State *s, int level, CVar msg, const char *file, int line, const char *func);

namespace {

yy::location Loc() {
    return yy::location();
}

SyntaxTreeInterfacePtr MakeNumberExp(const std::string &v) {
    auto e = std::make_shared<SyntaxTreeExp>(Loc());
    e->SetExpKind(ExpKind::kNumber);
    e->SetValue(v);
    return e;
}

SyntaxTreeInterfacePtr MakeStringExp(const std::string &v) {
    auto e = std::make_shared<SyntaxTreeExp>(Loc());
    e->SetExpKind(ExpKind::kString);
    e->SetValue(v);
    return e;
}

SyntaxTreeInterfacePtr MakeSimpleVar(const std::string &name) {
    auto v = std::make_shared<SyntaxTreeVar>(Loc());
    v->SetVarKind(VarKind::kSimple);
    v->SetName(name);
    return v;
}

SyntaxTreeInterfacePtr MakePrefixVar(const std::string &name) {
    auto pe = std::make_shared<SyntaxTreePrefixexp>(Loc());
    pe->SetPrefixKind(PrefixExpKind::kVar);
    pe->SetValue(MakeSimpleVar(name));
    return pe;
}

SyntaxTreeInterfacePtr MakePrefixCall(const std::string &name) {
    auto pe = std::make_shared<SyntaxTreePrefixexp>(Loc());
    pe->SetPrefixKind(PrefixExpKind::kFunctionCall);
    pe->SetValue(MakeSimpleVar(name));
    return pe;
}

SyntaxTreeInterfacePtr MakeVarPrefixExp(const std::string &name) {
    auto e = std::make_shared<SyntaxTreeExp>(Loc());
    e->SetExpKind(ExpKind::kPrefixExp);
    e->SetRight(MakePrefixVar(name));
    return e;
}

SyntaxTreeInterfacePtr MakeCallPrefixExp(const std::string &name) {
    auto e = std::make_shared<SyntaxTreeExp>(Loc());
    e->SetExpKind(ExpKind::kPrefixExp);
    e->SetRight(MakePrefixCall(name));
    return e;
}

SyntaxTreeInterfacePtr MakeUnaryMinus(const SyntaxTreeInterfacePtr &right) {
    auto op = std::make_shared<SyntaxTreeUnop>(Loc());
    op->SetOpKind(UnOpKind::kMinus);
    auto e = std::make_shared<SyntaxTreeExp>(Loc());
    e->SetExpKind(ExpKind::kUnop);
    e->SetOp(op);
    e->SetRight(right);
    return e;
}

}// namespace

TEST(core_cov_gaps, compile_common_math_param_helpers) {
    EXPECT_EQ(MathParamKindOf(0, 0), kMathParamInt);
    EXPECT_EQ(MathParamKindOf(1, 0), kMathParamFloat);
    EXPECT_EQ(MathParamKindOf(2, 1), kMathParamFloat);
    EXPECT_STREQ(MathParamCTypeName(kMathParamInt), "int64_t");
    EXPECT_STREQ(MathParamCTypeName(kMathParamFloat), "double");
    EXPECT_EQ(MathParamSuffix(kMathParamInt), '0');
    EXPECT_EQ(MathParamSuffix(kMathParamFloat), '1');

    EXPECT_EQ(InferredTypeToString(T_UNKNOWN), "T_UNKNOWN");
    EXPECT_EQ(InferredTypeToString(T_INT), "T_INT");
    EXPECT_EQ(InferredTypeToString(T_FLOAT), "T_FLOAT");
    EXPECT_EQ(InferredTypeToString(T_DYNAMIC), "T_DYNAMIC");
    EXPECT_EQ(InferredTypeToString(static_cast<InferredType>(99)), "T_UNKNOWN");

    EXPECT_TRUE(IsNumericInferredType(T_INT));
    EXPECT_TRUE(IsNumericInferredType(T_FLOAT));
    EXPECT_FALSE(IsNumericInferredType(T_UNKNOWN));
    EXPECT_FALSE(IsNumericInferredType(T_DYNAMIC));

    EXPECT_EQ(InferNumericBinopResultType(BinOpKind::kSlash, T_INT, T_INT), T_FLOAT);
    EXPECT_EQ(InferNumericBinopResultType(BinOpKind::kPow, T_INT, T_FLOAT), T_FLOAT);
    EXPECT_EQ(InferNumericBinopResultType(BinOpKind::kPlus, T_INT, T_INT), T_INT);
    EXPECT_EQ(InferNumericBinopResultType(BinOpKind::kPlus, T_INT, T_FLOAT), T_FLOAT);
    EXPECT_EQ(InferNumericBinopResultType(BinOpKind::kBitAnd, T_INT, T_INT), T_INT);
    EXPECT_EQ(InferNumericBinopResultType(BinOpKind::kAnd, T_INT, T_FLOAT), T_FLOAT);
    EXPECT_EQ(InferNumericBinopResultType(BinOpKind::kOr, T_INT, T_FLOAT), T_INT);
    EXPECT_EQ(InferNumericBinopResultType(BinOpKind::kConcat, T_INT, T_INT), T_DYNAMIC);
    EXPECT_EQ(InferNumericBinopResultType(BinOpKind::kPlus, T_DYNAMIC, T_INT), T_DYNAMIC);

    EXPECT_STREQ(SpecReturnCTypeName(T_INT), "int64_t");
    EXPECT_STREQ(SpecReturnCTypeName(T_FLOAT), "double");
    EXPECT_STREQ(SpecReturnCTypeName(T_DYNAMIC), "CVar");

    EXPECT_EQ(SpecFuncName("fib", {0}, 0), "fib_0");
    EXPECT_EQ(SpecFuncName("test", {1, 4}, 2), "test_0_1");
    EXPECT_NE(BoxNativeValue("x", T_INT).find("VAR_INT"), std::string::npos);
    EXPECT_NE(BoxNativeValue("y", T_FLOAT).find("VAR_FLOAT"), std::string::npos);
}

TEST(core_cov_gaps, compile_common_extract_args_and_vararg) {
    {
        auto args = std::make_shared<SyntaxTreeArgs>(Loc());
        args->SetArgsKind(ArgsKind::kExpList);
        auto el = std::make_shared<SyntaxTreeExplist>(Loc());
        el->AddExp(MakeNumberExp("1"));
        el->AddExp(MakeNumberExp("2"));
        args->SetExplist(el);
        auto raw = ExtractCallRawArgs(args);
        ASSERT_EQ(raw.size(), 2u);
    }
    {
        auto args = std::make_shared<SyntaxTreeArgs>(Loc());
        args->SetArgsKind(ArgsKind::kString);
        args->SetString(MakeStringExp("\"hi\""));
        auto raw = ExtractCallRawArgs(args);
        ASSERT_EQ(raw.size(), 1u);
    }
    {
        auto args = std::make_shared<SyntaxTreeArgs>(Loc());
        args->SetArgsKind(ArgsKind::kTableConstructor);
        auto tc = std::make_shared<SyntaxTreeTableconstructor>(Loc());
        args->SetTableconstructor(tc);
        auto raw = ExtractCallRawArgs(args);
        ASSERT_EQ(raw.size(), 1u);
    }

    EXPECT_FALSE(IsVarargExp(nullptr));
    EXPECT_FALSE(IsVarargExp(MakeNumberExp("1")));
    EXPECT_FALSE(IsVarargExp(MakeCallPrefixExp("f")));
    EXPECT_TRUE(IsVarargExp(MakeVarPrefixExp("__fakelua_vararg_0")));
    EXPECT_FALSE(IsVarargExp(MakeVarPrefixExp("x")));

    EXPECT_FALSE(IsFunctionCallExp(nullptr));
    EXPECT_FALSE(IsFunctionCallExp(MakeNumberExp("1")));
    EXPECT_TRUE(IsFunctionCallExp(MakeCallPrefixExp("f")));
    EXPECT_FALSE(IsFunctionCallExp(MakeVarPrefixExp("x")));
}

TEST(core_cov_gaps, compile_common_package_and_strings) {
    EXPECT_EQ(StripLuaStringQuotes("\"abc\""), "abc");
    EXPECT_EQ(StripLuaStringQuotes("'xyz'"), "xyz");
    EXPECT_EQ(StripLuaStringQuotes("bare"), "bare");
    EXPECT_EQ(StripLuaStringQuotes("\""), "\"");

    {
        // package "mypkg"
        auto fc = std::make_shared<SyntaxTreeFunctioncall>(Loc());
        fc->SetPrefixexp(MakePrefixVar("package"));
        auto args = std::make_shared<SyntaxTreeArgs>(Loc());
        args->SetArgsKind(ArgsKind::kString);
        args->SetString(MakeStringExp("\"mypkg\""));
        fc->SetArgs(args);
        std::string out;
        EXPECT_TRUE(ExtractPackageName(fc, out));
        EXPECT_EQ(out, "mypkg");
    }
    {
        // package("mypkg") via explist
        auto fc = std::make_shared<SyntaxTreeFunctioncall>(Loc());
        fc->SetPrefixexp(MakePrefixVar("package"));
        auto args = std::make_shared<SyntaxTreeArgs>(Loc());
        args->SetArgsKind(ArgsKind::kExpList);
        auto el = std::make_shared<SyntaxTreeExplist>(Loc());
        el->AddExp(MakeStringExp("\"pkg2\""));
        args->SetExplist(el);
        fc->SetArgs(args);
        std::string out;
        EXPECT_TRUE(ExtractPackageName(fc, out));
        EXPECT_EQ(out, "pkg2");
    }
    {
        // package = "pkg3"
        auto assign = std::make_shared<SyntaxTreeAssign>(Loc());
        auto vl = std::make_shared<SyntaxTreeVarlist>(Loc());
        vl->AddVar(MakeSimpleVar("package"));
        auto el = std::make_shared<SyntaxTreeExplist>(Loc());
        el->AddExp(MakeStringExp("\"pkg3\""));
        assign->SetVarlist(vl);
        assign->SetExplist(el);
        std::string out;
        EXPECT_TRUE(ExtractPackageName(assign, out));
        EXPECT_EQ(out, "pkg3");
    }
    {
        auto empty = std::make_shared<SyntaxTreeEmpty>(Loc());
        std::string out;
        EXPECT_FALSE(ExtractPackageName(empty, out));
    }

    auto empty = std::make_shared<SyntaxTreeEmpty>(Loc());
    EXPECT_FALSE(SyntaxTreeLocationStr("f.lua", empty).empty());
}

TEST(core_cov_gaps, compile_common_number_key_classify) {
    TableKeyKind kind = TableKeyKind::kString;
    std::string canon;
    int64_t iv = 0;
    double fv = 0;

    EXPECT_TRUE(ClassifyLuaNumberKey("42", kind, canon, iv, fv));
    EXPECT_EQ(kind, TableKeyKind::kInt);
    EXPECT_EQ(iv, 42);

    EXPECT_TRUE(ClassifyLuaNumberKey("0x10", kind, canon, iv, fv));
    EXPECT_EQ(kind, TableKeyKind::kInt);
    EXPECT_EQ(iv, 16);

    EXPECT_TRUE(ClassifyLuaNumberKey("1.5", kind, canon, iv, fv));
    EXPECT_EQ(kind, TableKeyKind::kFloat);

    EXPECT_TRUE(ClassifyLuaNumberKey("1.0", kind, canon, iv, fv));
    EXPECT_EQ(kind, TableKeyKind::kInt);
    EXPECT_EQ(iv, 1);

    EXPECT_FALSE(ClassifyLuaNumberKey("", kind, canon, iv, fv));
    EXPECT_FALSE(ClassifyLuaNumberKey("abc", kind, canon, iv, fv));
    EXPECT_FALSE(ClassifyLuaNumberKey("inf", kind, canon, iv, fv));

    auto n = MakeNumberExp("7");
    EXPECT_TRUE(ClassifyConstNumberExp(n, kind, canon, iv, fv));
    EXPECT_EQ(iv, 7);

    auto neg = MakeUnaryMinus(MakeNumberExp("3"));
    EXPECT_TRUE(ClassifyConstNumberExp(neg, kind, canon, iv, fv));
    EXPECT_EQ(iv, -3);

    auto min_neg = MakeUnaryMinus(MakeNumberExp(std::to_string(std::numeric_limits<int64_t>::min())));
    // May classify as float overflow path; just exercise the branch.
    (void)ClassifyConstNumberExp(min_neg, kind, canon, iv, fv);

    auto two63 = MakeUnaryMinus(MakeNumberExp("9223372036854775808.0"));
    EXPECT_TRUE(ClassifyConstNumberExp(two63, kind, canon, iv, fv));

    EXPECT_FALSE(ClassifyConstNumberExp(MakeStringExp("\"x\""), kind, canon, iv, fv));
    EXPECT_FALSE(ConstNumberExpHasFloatSyntax(nullptr));
    EXPECT_TRUE(ConstNumberExpHasFloatSyntax(MakeNumberExp("1.0")));
    EXPECT_FALSE(ConstNumberExpHasFloatSyntax(MakeNumberExp("1")));
    EXPECT_TRUE(ConstNumberExpHasFloatSyntax(MakeUnaryMinus(MakeNumberExp("0.0"))));

    EXPECT_TRUE(ConstNumberExpIsIntValue(TableKeyKind::kInt, MakeNumberExp("1")));
    EXPECT_FALSE(ConstNumberExpIsIntValue(TableKeyKind::kInt, MakeNumberExp("1.0")));

    const double z = ConstNumberExpToDouble(TableKeyKind::kInt, 0, 0.0, MakeNumberExp("0.0"));
    EXPECT_TRUE(z == 0.0);
    const double nz = ConstNumberExpToDouble(TableKeyKind::kInt, 0, 0.0, MakeUnaryMinus(MakeNumberExp("0.0")));
    EXPECT_TRUE(std::signbit(nz));

    EXPECT_NE(FormatCDoubleLiteral(1.0).find('.'), std::string::npos);
    EXPECT_FALSE(FormatCDoubleLiteral(1.5).empty());

    EXPECT_EQ(EscapeCStringLiteral("a\"b\\c\n\r\td"), "a\\\"b\\\\c\\n\\r\\td");
    EXPECT_NE(EscapeCStringLiteral(std::string("\x01\x7f", 2)).find('\\'), std::string::npos);
}

TEST(core_cov_gaps, compile_common_table_spec_helpers) {
    TableFieldInfo fs{.key = "a", .type = T_INT, .key_kind = TableKeyKind::kString};
    TableFieldInfo fi{.key = "1", .type = T_INT, .key_kind = TableKeyKind::kInt, .int_value = 1};
    TableFieldInfo fb{.key = "true", .type = T_INT, .key_kind = TableKeyKind::kBool, .bool_value = true};
    TableFieldInfo ff{.key = "1.5", .type = T_FLOAT, .key_kind = TableKeyKind::kFloat, .float_value = 1.5};
    EXPECT_EQ(TableFieldDescriptor(fs), "S_a");
    EXPECT_EQ(TableFieldDescriptor(fi), "I_1");
    EXPECT_EQ(TableFieldDescriptor(fb), "B_true");
    EXPECT_EQ(TableFieldDescriptor(ff), "F_1.5");

    std::vector<TableFieldInfo> fields{fs, fi};
    const auto name = ComputeTableSpecName(fields);
    EXPECT_EQ(name.rfind("flua_spec_", 0), 0u);
    EXPECT_EQ(ComputeTableSpecName({fi, fs}), name);

    EXPECT_EQ(GetSimpleVarName(nullptr), "");
    EXPECT_EQ(GetSimpleVarName(MakeSimpleVar("x")), "x");
    EXPECT_EQ(GetSimpleVarName(MakePrefixVar("y")), "y");
    EXPECT_EQ(GetSimpleVarName(MakeVarPrefixExp("z")), "z");
    EXPECT_EQ(GetSimpleVarName(MakeNumberExp("1")), "");
}

TEST(core_cov_gaps, syntax_tree_getters_setters) {
    auto empty = std::make_shared<SyntaxTreeEmpty>(Loc());
    EXPECT_EQ(empty->Type(), SyntaxTreeType::Empty);
    EXPECT_FALSE(empty->Dump(1).empty());
    empty->SetLoc(Loc());
    (void)empty->Loc();

    auto label = std::make_shared<SyntaxTreeLabel>(Loc());
    label->SetName("L");
    EXPECT_EQ(label->GetName(), "L");
    EXPECT_EQ(label->Type(), SyntaxTreeType::Label);

    auto ret = std::make_shared<SyntaxTreeReturn>(Loc());
    auto el = std::make_shared<SyntaxTreeExplist>(Loc());
    ret->SetExplist(el);
    EXPECT_EQ(ret->Explist(), el);

    auto assign = std::make_shared<SyntaxTreeAssign>(Loc());
    auto vl = std::make_shared<SyntaxTreeVarlist>(Loc());
    assign->SetVarlist(vl);
    assign->SetExplist(el);
    EXPECT_EQ(assign->Varlist(), vl);
    EXPECT_EQ(assign->Explist(), el);
    vl->AddVar(MakeSimpleVar("a"));
    EXPECT_EQ(vl->Vars().size(), 1u);
    el->AddExp(MakeNumberExp("1"));
    EXPECT_EQ(el->Exps().size(), 1u);

    auto var = std::make_shared<SyntaxTreeVar>(Loc());
    var->SetName("t");
    var->SetVarKind(VarKind::kDot);
    auto pe = MakePrefixVar("obj");
    var->SetPrefixexp(pe);
    var->SetExp(MakeNumberExp("1"));
    EXPECT_EQ(var->GetName(), "t");
    EXPECT_EQ(var->GetPrefixexp(), pe);
    EXPECT_NE(var->GetExp(), nullptr);
    EXPECT_EQ(var->GetVarKind(), VarKind::kDot);
    // const overloads
    const SyntaxTreeVar &cvar = *var;
    EXPECT_EQ(cvar.GetName(), "t");
    EXPECT_EQ(cvar.GetPrefixexp(), pe);
    EXPECT_NE(cvar.GetExp(), nullptr);

    auto fc = std::make_shared<SyntaxTreeFunctioncall>(Loc());
    fc->SetPrefixexp(pe);
    auto args = std::make_shared<SyntaxTreeArgs>(Loc());
    args->SetArgsKind(ArgsKind::kExpList);
    args->SetExplist(el);
    fc->SetArgs(args);
    fc->SetName("m");
    EXPECT_EQ(fc->prefixexp(), pe);
    EXPECT_EQ(fc->Args(), args);
    EXPECT_EQ(fc->Name(), "m");

    auto tc = std::make_shared<SyntaxTreeTableconstructor>(Loc());
    auto fl = std::make_shared<SyntaxTreeFieldlist>(Loc());
    auto field = std::make_shared<SyntaxTreeField>(Loc());
    field->SetKey(MakeNumberExp("1"));
    field->SetValue(MakeNumberExp("2"));
    field->SetName("k");
    field->SetFieldKind(FieldKind::kObject);
    EXPECT_NE(field->Key(), nullptr);
    EXPECT_NE(field->Value(), nullptr);
    EXPECT_EQ(field->Name(), "k");
    EXPECT_EQ(field->GetFieldKind(), FieldKind::kObject);
    fl->AddField(field);
    EXPECT_EQ(fl->Fields().size(), 1u);
    tc->SetFieldlist(fl);
    EXPECT_EQ(tc->Fieldlist(), fl);

    auto brk = std::make_shared<SyntaxTreeBreak>(Loc());
    EXPECT_EQ(brk->Type(), SyntaxTreeType::Break);
    auto gt = std::make_shared<SyntaxTreeGoto>(Loc());
    gt->SetLabel("L");
    EXPECT_EQ(gt->GetLabel(), "L");

    auto wh = std::make_shared<SyntaxTreeWhile>(Loc());
    wh->SetExp(MakeNumberExp("1"));
    wh->SetBlock(std::make_shared<SyntaxTreeBlock>(Loc()));
    EXPECT_NE(wh->Exp(), nullptr);
    EXPECT_NE(wh->Block(), nullptr);

    auto rp = std::make_shared<SyntaxTreeRepeat>(Loc());
    rp->SetBlock(std::make_shared<SyntaxTreeBlock>(Loc()));
    rp->SetExp(MakeNumberExp("1"));
    EXPECT_NE(rp->Block(), nullptr);
    EXPECT_NE(rp->Exp(), nullptr);

    auto iff = std::make_shared<SyntaxTreeIf>(Loc());
    iff->SetExp(MakeNumberExp("1"));
    iff->SetBlock(std::make_shared<SyntaxTreeBlock>(Loc()));
    iff->SetElseiflist(std::make_shared<SyntaxTreeElseiflist>(Loc()));
    iff->SetElseBlock(std::make_shared<SyntaxTreeBlock>(Loc()));
    EXPECT_NE(iff->Exp(), nullptr);
    EXPECT_NE(iff->Block(), nullptr);
    EXPECT_NE(iff->ElseIfs(), nullptr);
    EXPECT_NE(iff->ElseBlock(), nullptr);

    EXPECT_EQ(SyntaxTreeTypeToString(static_cast<SyntaxTreeType>(999)), "UNKNOWN");
}

TEST(core_cov_gaps, string_escapes_and_math_inline) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./cov/test_core_gaps.lua", config);
        double res = 0;
        Call(s, jit_type, "test_string_escapes", res);
        EXPECT_EQ(res, 0.0) << "jit=" << JitTypeName(jit_type);
        Call(s, jit_type, "test_math_inline_extra", res);
        EXPECT_EQ(res, 0.0) << "jit=" << JitTypeName(jit_type);
        Call(s, jit_type, "test_local_multi_assign", res);
        EXPECT_EQ(res, 0.0) << "jit=" << JitTypeName(jit_type);
        Call(s, jit_type, "test_table_seq_and_move", res);
        EXPECT_EQ(res, 0.0) << "jit=" << JitTypeName(jit_type);
        Call(s, jit_type, "test_string_ops_extra", res);
        EXPECT_EQ(res, 0.0) << "jit=" << JitTypeName(jit_type);
        Call(s, jit_type, "test_select_and_closure_args", res);
        EXPECT_EQ(res, 0.0) << "jit=" << JitTypeName(jit_type);
    }
    FakeluaDeleteState(s);
}

TEST(core_cov_gaps, fakelua_log_lua_cvar_types) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    SetLogLevel(s, LogLevel::Trace);

    CVar nil{};
    FakeluaLogLua(s, static_cast<int>(LogLevel::Info), nil, "f.lua", 1, "fn");

    CVar b{};
    b.type_ = static_cast<int>(VarType::Bool);
    b.data_.b = true;
    FakeluaLogLua(s, static_cast<int>(LogLevel::Info), b, "f.lua", 2, "fn");
    b.data_.b = false;
    FakeluaLogLua(s, static_cast<int>(LogLevel::Warn), b, "f.lua", 3, "fn");

    CVar i{};
    i.type_ = static_cast<int>(VarType::Int);
    i.data_.i = 42;
    FakeluaLogLua(s, static_cast<int>(LogLevel::Error), i, "f.lua", 4, "fn");

    CVar f{};
    f.type_ = static_cast<int>(VarType::Float);
    f.data_.f = 3.14;
    FakeluaLogLua(s, static_cast<int>(LogLevel::Critical), f, "f.lua", 5, "fn");

    CVar bad{};
    bad.type_ = static_cast<int>(VarType::Table);
    FakeluaLogLua(s, static_cast<int>(LogLevel::Info), bad, nullptr, 6, nullptr);

    // Level filter early-return in FakeluaLogLua / LogLua
    SetLogLevel(s, LogLevel::Error);
    FakeluaLogLua(s, static_cast<int>(LogLevel::Info), i, "f.lua", 7, "fn");
    LogLua(s, LogLevel::Debug, "t", "m", "f.lua", 8, "fn");

    // Off level name path via explicit Off log when level is Off
    SetLogLevel(s, LogLevel::Off);
    Log(s, LogLevel::Off, "t", "off-msg");

    // empty path configure + reconfigure existing sink
    SetLogFile(s, "", 0, 0);
    const auto path = (std::filesystem::temp_directory_path() / "fakelua_cov_log2.log").string();
    std::error_code ec;
    std::filesystem::remove(path, ec);
    // Create oversized file so Configure rotates on open
    {
        std::ofstream ofs(path);
        ofs << std::string(200, 'z');
    }
    SetLogFile(s, path, /*max_size=*/50, /*max_files=*/2);
    SetLogFile(s, path, /*max_size=*/50, /*max_files=*/2);// close existing
    SetLogLevel(s, LogLevel::Info);
    LOG_INFO(s, "engine", "after-rotate");
    SetLogFile(s, "", 0, 0);
    FakeluaDeleteState(s);
    std::filesystem::remove(path, ec);
    std::filesystem::remove(path + ".1", ec);
}

TEST(core_cov_gaps, gcc_jit_env_and_paths) {
    StateConfig cfg;
    cfg.gcc_config.include_paths.push_back("/usr/include");
    cfg.gcc_config.library_paths.push_back("/usr/lib");
    cfg.gcc_config.libraries.push_back("m");
    State *s = FakeluaNewState(cfg);
    ASSERT_NE(s, nullptr);

    setenv("FAKELUA_CC", "gcc", 1);
    CompileConfig ccfg;
    ccfg.debug_mode = true;
    EXPECT_NO_THROW(CompileString(s, "function gcc_cov_fn()\n  return 1\nend\n", ccfg));
    double res = 0;
    Call(s, JIT_GCC, "gcc_cov_fn", res);
    EXPECT_EQ(res, 1.0);

    // Force GCC failure to hit JoinCommand / CleanupOnError / log-read paths.
    setenv("FAKELUA_CC", "false", 1);
    EXPECT_THROW(CompileString(s, "function gcc_fail()\n  return 2\nend\n", ccfg), std::exception);
    unsetenv("FAKELUA_CC");
    FakeluaDeleteState(s);
}

TEST(core_cov_gaps, number_util_and_os_exec) {
    EXPECT_TRUE(IsInteger("42"));
    EXPECT_FALSE(IsInteger("1.0"));
    EXPECT_FALSE(IsInteger(""));

    EXPECT_EQ(FormatLuaFloat(1.0), "1.0");
    EXPECT_FALSE(FormatLuaFloat(1.5).empty());
    EXPECT_FALSE(FormatLuaFloat(-42.0).empty());

    EXPECT_EQ(*TryConvertDoubleToInt64(3.0), 3);
    EXPECT_FALSE(TryConvertDoubleToInt64(1.5).has_value());
    EXPECT_FALSE(TryConvertDoubleToInt64(std::numeric_limits<double>::infinity()).has_value());
    EXPECT_FALSE(TryConvertDoubleToInt64(1e300).has_value());
    EXPECT_EQ(*TryConvertDoubleToInt64(static_cast<double>(std::numeric_limits<int64_t>::min())), std::numeric_limits<int64_t>::min());

    EXPECT_EQ(CompareInt64Double(1, std::numeric_limits<double>::quiet_NaN()), 0);
    EXPECT_EQ(CompareInt64Double(1, 1e300), -1);
    EXPECT_EQ(CompareInt64Double(1, -1e300), 1);
    EXPECT_EQ(CompareInt64Double(2, 2.5), -1);
    EXPECT_EQ(CompareInt64Double(3, 2.5), 1);
    EXPECT_EQ(CompareInt64Double(2, 2.0), 0);
    EXPECT_TRUE(Int64DoubleEqual(2, 2.0));
    EXPECT_FALSE(Int64DoubleEqual(2, std::numeric_limits<double>::quiet_NaN()));

    const std::string out = Exec("echo hello_cov");
    EXPECT_NE(out.find("hello_cov"), std::string::npos);
}

TEST(core_cov_gaps, register_native_var_and_func_addr) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    RegisterNativeVarFunction(s, "cov_make_tbl", 0, false, [](State *state, const std::vector<VarInterface *> &) -> VarInterface * {
        auto *t = new SimpleVarImpl();
        std::vector<std::pair<VarInterface *, VarInterface *>> kvs;
        auto *k = new SimpleVarImpl();
        k->ViSetString("a");
        auto *v = new SimpleVarImpl();
        v->ViSetInt(1);
        kvs.emplace_back(k, v);
        t->ViSetTable(kvs);
        delete k;
        delete v;
        return t;
    });
    // null callback path
    RegisterNativeVarFunction(s, "cov_nil_cb", 0, false, NativeVarFuncCallback{});

    CompileConfig config;
    CompileString(s, R"(
function test_native_var()
    local t = cov_make_tbl()
    if t.a ~= 1 then return 1 end
    local n = cov_nil_cb()
    if n ~= nil then return 2 end
    return 0
end
)",
                  config);
    double res = 0;
    Call(s, JIT_GCC, "test_native_var", res);
    EXPECT_EQ(res, 0.0);

    int argc = 0;
    bool is_va = false;
    void *addr = inter::GetFuncAddr(s, JIT_GCC, "test_native_var", argc, is_va);
    EXPECT_NE(addr, nullptr);
    EXPECT_EQ(inter::GetFuncAddr(s, JIT_GCC, "no_such_fn_xyz", argc, is_va), nullptr);

    SetDebugLogLevel(s, 1);
    SetDebugLogLevel(s, LogLevel::Warn);
    FakeluaDeleteState(s);
}
