#include "fakelua.h"
#include "util/common.h"
#include "util/macro.h"
#include "util/string_util.h"
#include "gtest/gtest.h"
#include <vector>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace {
struct UnbufferedStdio {
    UnbufferedStdio() {
        setvbuf(stdout, nullptr, _IONBF, 0);
        setvbuf(stderr, nullptr, _IONBF, 0);
    }
} g_unbuffered_stdio;
}// namespace

using namespace fakelua;

TEST(common, escapse_string) {
    std::string src = "a\\ab\\bc\\fd\\ne\\rf\\tg\\vh\\\"i\\'j\\\\k";
    std::string dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "a\ab\bc\fd\ne\rf\tg\vh\"i\'j\\k");

    src = "\\a\\b\\f\\n\\r\\t\\v\\\"\\'";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "\a\b\f\n\r\t\v\"\'");

    src = "a\\z    b";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "ab");

    src = "a\\z    ";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "a");

    src = "\\z    \\z    ";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "");

    src = "\\z\\z    ";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "");

    src = "\\z\\z";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "");

    // \z should skip all whitespace characters (tabs, newlines, carriage returns, etc.)
    src = "a\\z\t\n\r b";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "ab");

    src = "hello\\z\n\t\t  world";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "helloworld");

    src = "x\\z\r\n\v\fy";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "xy");

    src = "a\\z\n";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "a");

    src = "a\\97b";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "aab");

    src = "a\\97";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "aa");

    src = "\\097b";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "ab");

    src = "\\097\\97";
    dst = ReplaceEscapeChars(src);
    ASSERT_EQ(dst, "aa");
}

TEST(common, logging) {
    FakeluaStateGuard sg;
    auto s = sg.GetState();
    SetLogLevel(s, LogLevel::Info);
    LOG_INFO(s, "test", "Hello, World!");
    LOG_ERROR(s, "test", "Hello, World!");
}

TEST(common, vi_sort) {
    SimpleVarImpl t;
    std::vector<std::pair<VarInterface *, VarInterface *>> kv;

    SimpleVarImpl k1;
    k1.ViSetNil();
    SimpleVarImpl v1;
    v1.ViSetInt(1);
    kv.push_back(std::make_pair(&k1, &v1));

    SimpleVarImpl k2;
    k2.ViSetNil();
    SimpleVarImpl v2;
    v2.ViSetInt(1);
    kv.push_back(std::make_pair(&k2, &v2));

    SimpleVarImpl k3;
    k3.ViSetBool(true);
    SimpleVarImpl v3;
    v3.ViSetInt(3);
    kv.push_back(std::make_pair(&k3, &v3));

    SimpleVarImpl k4;
    k4.ViSetBool(false);
    SimpleVarImpl v4;
    v4.ViSetInt(4);
    kv.push_back(std::make_pair(&k4, &v4));

    SimpleVarImpl k5;
    k5.ViSetFloat(1.0);
    SimpleVarImpl v5;
    v5.ViSetInt(5);
    kv.push_back(std::make_pair(&k5, &v5));

    SimpleVarImpl k6;
    k6.ViSetFloat(2.0);
    SimpleVarImpl v6;
    v6.ViSetInt(6);
    kv.push_back(std::make_pair(&k6, &v6));

    SimpleVarImpl k7;
    k7.ViSetTable({});
    SimpleVarImpl v7;
    v7.ViSetInt(7);
    kv.push_back(std::make_pair(&k7, &v7));

    SimpleVarImpl k8;
    k8.ViSetTable({});
    SimpleVarImpl v8;
    v8.ViSetInt(7);
    kv.push_back(std::make_pair(&k8, &v8));

    std::ranges::shuffle(kv, std::mt19937(std::random_device()()));

    t.ViSetTable(kv);
    t.ViSortTable();

    ASSERT_EQ(t.ViToString(0), "table:\n\t[nil] = 1\n\t[nil] = 1\n\t[false] = 4\n\t[true] = 3\n\t[1.000000] = 5\n\t[2.000000] = "
                               "6\n\t[table:] = 7\n\t[table:] = 7");
}

TEST(common, IsNumber) {
    ASSERT_TRUE(IsNumber("123"));
    ASSERT_TRUE(IsNumber("-123"));
    ASSERT_TRUE(IsNumber("+123"));

    ASSERT_TRUE(IsNumber("123.456"));
    ASSERT_TRUE(IsNumber("-123.456"));
    ASSERT_TRUE(IsNumber("+123.456"));

    ASSERT_TRUE(IsNumber("123e456"));
    ASSERT_TRUE(IsNumber("-123e-456"));
    ASSERT_TRUE(IsNumber("+123E+456"));

    ASSERT_TRUE(IsNumber("0x123"));
    ASSERT_TRUE(IsNumber("-0X123"));
    ASSERT_TRUE(IsNumber("+0x123.456"));
    ASSERT_TRUE(IsNumber("-0X123.456P+789"));

    ASSERT_FALSE(IsNumber("abc"));
    ASSERT_FALSE(IsNumber("123abc"));
    ASSERT_FALSE(IsNumber("123.456.789"));
}

TEST(common, IsInteger) {
    ASSERT_TRUE(IsInteger("123"));
    ASSERT_TRUE(IsInteger("-123"));
    ASSERT_TRUE(IsInteger("+123"));

    ASSERT_TRUE(IsInteger("0x123"));
    ASSERT_TRUE(IsInteger("-0X123"));

    ASSERT_FALSE(IsInteger("123.456"));
    ASSERT_FALSE(IsInteger("-123.456"));
    ASSERT_FALSE(IsInteger("+123.456"));
    ASSERT_FALSE(IsInteger("123e456"));
    ASSERT_FALSE(IsInteger("-123e-456"));
    ASSERT_FALSE(IsInteger("+123E+456"));
    ASSERT_FALSE(IsInteger("+0x123.456"));
    ASSERT_FALSE(IsInteger("-0X123.456P+789"));
    ASSERT_FALSE(IsInteger("abc"));
    ASSERT_FALSE(IsInteger("123abc"));
    ASSERT_FALSE(IsInteger("123.456.789"));
}

TEST(common, ToInteger) {
    ASSERT_EQ(ToInteger("123"), 123);
    ASSERT_EQ(ToInteger("-123"), -123);
    ASSERT_EQ(ToInteger("+123"), 123);
    ASSERT_EQ(ToInteger("0x123"), 0x123);
    ASSERT_EQ(ToInteger("0X123"), 0x123);
    ASSERT_EQ(ToInteger("+0X123"), 0x123);
    ASSERT_EQ(ToInteger("-0X123"), -0x123);

    ASSERT_EQ(ToInteger("0"), 0);
    ASSERT_EQ(ToInteger("-0"), 0);
    ASSERT_EQ(ToInteger("0x0"), 0);
    ASSERT_EQ(ToInteger("0X0"), 0);

    ASSERT_EQ(ToInteger("9223372036854775807"), INT64_MAX);
    ASSERT_EQ(ToInteger("-9223372036854775808"), INT64_MIN);

    ASSERT_EQ(ToInteger("0xff"), 255);
    ASSERT_EQ(ToInteger("0xBEBADA"), 0xBEBADA);
}

TEST(common, ToFloat) {
    ASSERT_TRUE(std::isnan(ToFloat("nan")));
    ASSERT_TRUE(std::isnan(ToFloat("NaN")));
    ASSERT_TRUE(std::isnan(ToFloat("NAN")));
    ASSERT_TRUE(std::isinf(ToFloat("inf")));
    ASSERT_TRUE(std::isinf(ToFloat("Inf")));
    ASSERT_TRUE(std::isinf(ToFloat("INF")));
    ASSERT_TRUE(std::isinf(ToFloat("-inf")));
    ASSERT_TRUE(std::isinf(ToFloat("-Inf")));
    ASSERT_TRUE(std::isinf(ToFloat("-INF")));

    ASSERT_EQ(ToFloat("0"), 0);

    ASSERT_EQ(ToFloat("12.3"), 12.3);
    ASSERT_EQ(ToFloat("-12.3"), -12.3);
    ASSERT_EQ(ToFloat("+12.3"), 12.3);

    ASSERT_EQ(ToFloat("12.3e45"), 12.3e45);
    ASSERT_EQ(ToFloat("12.3e+45"), 12.3e+45);
    ASSERT_EQ(ToFloat("-12.3e-45"), -12.3e-45);

    ASSERT_EQ(ToFloat("0x123"), 0x123);
    ASSERT_EQ(ToFloat("0X12.3"), 18.1875);
    ASSERT_EQ(ToFloat("0X12.3P45"), 18.1875 * std::pow(2, 45));
    ASSERT_EQ(ToFloat("0X12.3P+45"), 18.1875 * std::pow(2, 45));
    ASSERT_EQ(ToFloat("0X12.3P-45"), 18.1875 * std::pow(2, -45));
    ASSERT_EQ(ToFloat("0X12.3P+0"), 18.1875);

    ASSERT_EQ(ToFloat("+0x123"), 0x123);
    ASSERT_EQ(ToFloat("+0X12.3"), 18.1875);
    ASSERT_EQ(ToFloat("+0X12.3P45"), 18.1875 * std::pow(2, 45));
    ASSERT_EQ(ToFloat("+0X12.3P+45"), 18.1875 * std::pow(2, 45));
    ASSERT_EQ(ToFloat("+0X12.3P-45"), 18.1875 * std::pow(2, -45));
    ASSERT_EQ(ToFloat("+0X12.3P+0"), 18.1875);

    ASSERT_EQ(ToFloat("-0x123"), -0x123);
    ASSERT_EQ(ToFloat("-0X12.3"), -18.1875);
    ASSERT_EQ(ToFloat("-0X12.3P45"), -18.1875 * std::pow(2, 45));
    ASSERT_EQ(ToFloat("-0X12.3P+45"), -18.1875 * std::pow(2, 45));
    ASSERT_EQ(ToFloat("-0X12.3P-45"), -18.1875 * std::pow(2, -45));
    ASSERT_EQ(ToFloat("-0X12.3P+0"), -18.1875);

    ASSERT_EQ(ToFloat("1.7976931348623157e+308"), std::numeric_limits<double>::max());
    ASSERT_EQ(ToFloat("2.2250738585072014e-308"), std::numeric_limits<double>::min());

    ASSERT_EQ(ToFloat("3.1416"), 3.1416);
    ASSERT_EQ(ToFloat("314.16e-2"), 314.16e-2);
    ASSERT_EQ(ToFloat("0.31416E1"), 0.31416E1);
    ASSERT_EQ(ToFloat("34e1"), 34e1);
    ASSERT_EQ(ToFloat("0x0.1E"), 0.1171875);
    ASSERT_EQ(ToFloat("0xA23p-4"), 0xA23p-4);
    ASSERT_EQ(ToFloat("0X1.921FB54442D18P+1"), 0X1.921FB54442D18P+1);
}

// Bug 1: ToInteger must parse INT64_MIN in hex form without throwing.
TEST(common, ToInteger_hex_int64_min) {
    ASSERT_EQ(ToInteger("-0x8000000000000000"), INT64_MIN);
    ASSERT_EQ(ToInteger("-0X8000000000000000"), INT64_MIN);
    // Largest positive hex value (INT64_MAX)
    ASSERT_EQ(ToInteger("0x7FFFFFFFFFFFFFFF"), INT64_MAX);
    ASSERT_EQ(ToInteger("-0x7FFFFFFFFFFFFFFF"), -INT64_MAX);
}

// Bug 2: ToFloat must handle hex integers >= 0x8000000000000000 without overflowing.
TEST(common, ToFloat_hex_large) {
    // 0x8000000000000000 == 9223372036854775808.0
    ASSERT_EQ(ToFloat("0x8000000000000000"), static_cast<double>(0x8000000000000000ULL));
    // All-ones: 0xFFFFFFFFFFFFFFFF == 18446744073709551615.0
    ASSERT_EQ(ToFloat("0xFFFFFFFFFFFFFFFF"), static_cast<double>(0xFFFFFFFFFFFFFFFFULL));
    // Negative large hex
    ASSERT_EQ(ToFloat("-0x8000000000000000"), -static_cast<double>(0x8000000000000000ULL));
}

// Bug 3: JoinString must not crash / UB on an empty vector.
TEST(common, JoinString_empty) {
    ASSERT_EQ(JoinString({}, ","), "");
    ASSERT_EQ(JoinString({"a"}, ","), "a");
    ASSERT_EQ(JoinString({"a", "b", "c"}, ","), "a,b,c");
    ASSERT_EQ(JoinString({"x", "y"}, ""), "xy");
}

// Bug 7: SET_FLAG_BIT / GET_FLAG_BIT must not invoke UB for any valid bit position.
TEST(common, flag_bit_macros) {
    // Basic set / clear / get
    uint32_t f = 0U;
    SET_FLAG_BIT(f, 0, true);
    ASSERT_EQ(GET_FLAG_BIT(f, 0), 1U);
    SET_FLAG_BIT(f, 0, false);
    ASSERT_EQ(GET_FLAG_BIT(f, 0), 0U);

    // High bit positions (would overflow with signed int)
    SET_FLAG_BIT(f, 31, true);
    ASSERT_EQ(GET_FLAG_BIT(f, 31), 1U);
    SET_FLAG_BIT(f, 31, false);
    ASSERT_EQ(GET_FLAG_BIT(f, 31), 0U);

    // Multiple bits
    SET_FLAG_BIT(f, 7, true);
    SET_FLAG_BIT(f, 15, true);
    SET_FLAG_BIT(f, 23, true);
    ASSERT_EQ(GET_FLAG_BIT(f, 7), 1U);
    ASSERT_EQ(GET_FLAG_BIT(f, 15), 1U);
    ASSERT_EQ(GET_FLAG_BIT(f, 23), 1U);
    ASSERT_EQ(GET_FLAG_BIT(f, 8), 0U);
}

// ToInteger error paths: invalid chars and out-of-range for negative hex.
TEST(common, ToInteger_neg_hex_errors) {
    // Invalid hex digits after stripping the -0x prefix.
    EXPECT_THROW(ToInteger("-0xGGG"), std::exception);
    // Value exceeds INT64_MAX + 1, cannot be represented as negative int64.
    EXPECT_THROW(ToInteger("-0x9000000000000000"), std::exception);
}

// ToFloat error paths: invalid hex float and invalid hex integer.
TEST(common, ToFloat_hex_errors) {
    // Hex float with trailing invalid chars after the exponent.
    EXPECT_THROW(ToFloat("0x1.2P3xyz"), std::exception);
    // Hex integer with invalid digits.
    EXPECT_THROW(ToFloat("0xGGG"), std::exception);
}

#include "compile/c_runtime_header.h"
#include "compile/compile_common.h"
#include "compile/syntax_tree.h"
#include "test_jit.h"
#include <cmath>
#include <limits>
#include <string>

// Keep c_runtime_cov.h (Codecov view / #line target) identical to kCRuntimeHeader's
// raw-string body. The string begins with a leading '\n' that is not in the file.
TEST(common, c_runtime_cov_matches_kCRuntimeHeader) {
    std::vector<std::filesystem::path> candidates;
    if (const char *home = std::getenv("FAKELUA_HOME"); home && *home) {
        candidates.emplace_back(std::filesystem::path(home) / kCRuntimeCovPath);
    }
    // unit_tests usually run with cwd=test/lua; shards may use a temp cwd with symlinks.
    candidates.emplace_back(std::filesystem::path("../../") / kCRuntimeCovPath);
    candidates.emplace_back(std::filesystem::path("../..") / "src" / "compile" / "c_runtime_cov.h");
    candidates.emplace_back(std::filesystem::current_path() / ".." / ".." / kCRuntimeCovPath);
    // Walk up from cwd looking for the repo-rooted header.
    {
        auto cur = std::filesystem::current_path();
        for (int i = 0; i < 6; ++i) {
            candidates.emplace_back(cur / kCRuntimeCovPath);
            if (!cur.has_parent_path() || cur == cur.parent_path()) break;
            cur = cur.parent_path();
        }
    }
    std::filesystem::path cov_path;
    std::ifstream ifs;
    for (const auto &cand: candidates) {
        std::error_code ec;
        auto canon = std::filesystem::weakly_canonical(cand, ec);
        if (ec) continue;
        ifs.open(canon);
        if (ifs.good()) {
            cov_path = canon;
            break;
        }
        ifs.close();
    }
    ASSERT_TRUE(ifs.good()) << "missing view header; tried FAKELUA_HOME/cwd parents for " << kCRuntimeCovPath;
    std::ostringstream oss;
    oss << ifs.rdbuf();
    const std::string file_body = oss.str();
    const std::string expected = kCRuntimeHeader[0] == '\n' ? std::string(kCRuntimeHeader + 1) : std::string(kCRuntimeHeader);
    EXPECT_EQ(file_body, expected) << "Edit kCRuntimeHeader and sync " << kCRuntimeCovPath
                                   << " (or the reverse); Codecov #line mapping depends on it.";
}

namespace {

yy::location CovLoc() {
    return yy::location();
}

SyntaxTreeInterfacePtr CovMakeNumberExp(const std::string &v) {
    auto e = std::make_shared<SyntaxTreeExp>(CovLoc());
    e->SetExpKind(ExpKind::kNumber);
    e->SetValue(v);
    return e;
}

SyntaxTreeInterfacePtr CovMakeStringExp(const std::string &v) {
    auto e = std::make_shared<SyntaxTreeExp>(CovLoc());
    e->SetExpKind(ExpKind::kString);
    e->SetValue(v);
    return e;
}

SyntaxTreeInterfacePtr CovMakeSimpleVar(const std::string &name) {
    auto v = std::make_shared<SyntaxTreeVar>(CovLoc());
    v->SetVarKind(VarKind::kSimple);
    v->SetName(name);
    return v;
}

SyntaxTreeInterfacePtr CovMakePrefixVar(const std::string &name) {
    auto pe = std::make_shared<SyntaxTreePrefixexp>(CovLoc());
    pe->SetPrefixKind(PrefixExpKind::kVar);
    pe->SetValue(CovMakeSimpleVar(name));
    return pe;
}

SyntaxTreeInterfacePtr CovMakePrefixCall(const std::string &name) {
    auto pe = std::make_shared<SyntaxTreePrefixexp>(CovLoc());
    pe->SetPrefixKind(PrefixExpKind::kFunctionCall);
    pe->SetValue(CovMakeSimpleVar(name));
    return pe;
}

SyntaxTreeInterfacePtr CovMakeVarPrefixExp(const std::string &name) {
    auto e = std::make_shared<SyntaxTreeExp>(CovLoc());
    e->SetExpKind(ExpKind::kPrefixExp);
    e->SetRight(CovMakePrefixVar(name));
    return e;
}

SyntaxTreeInterfacePtr CovMakeCallPrefixExp(const std::string &name) {
    auto e = std::make_shared<SyntaxTreeExp>(CovLoc());
    e->SetExpKind(ExpKind::kPrefixExp);
    e->SetRight(CovMakePrefixCall(name));
    return e;
}

SyntaxTreeInterfacePtr CovMakeUnaryMinus(const SyntaxTreeInterfacePtr &right) {
    auto op = std::make_shared<SyntaxTreeUnop>(CovLoc());
    op->SetOpKind(UnOpKind::kMinus);
    auto e = std::make_shared<SyntaxTreeExp>(CovLoc());
    e->SetExpKind(ExpKind::kUnop);
    e->SetOp(op);
    e->SetRight(right);
    return e;
}

}// namespace

TEST(common, compile_math_param_helpers) {
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

TEST(common, compile_extract_args_and_vararg) {
    {
        auto args = std::make_shared<SyntaxTreeArgs>(CovLoc());
        args->SetArgsKind(ArgsKind::kExpList);
        auto el = std::make_shared<SyntaxTreeExplist>(CovLoc());
        el->AddExp(CovMakeNumberExp("1"));
        el->AddExp(CovMakeNumberExp("2"));
        args->SetExplist(el);
        auto raw = ExtractCallRawArgs(args);
        ASSERT_EQ(raw.size(), 2u);
    }
    {
        auto args = std::make_shared<SyntaxTreeArgs>(CovLoc());
        args->SetArgsKind(ArgsKind::kString);
        args->SetString(CovMakeStringExp("\"hi\""));
        auto raw = ExtractCallRawArgs(args);
        ASSERT_EQ(raw.size(), 1u);
    }
    {
        auto args = std::make_shared<SyntaxTreeArgs>(CovLoc());
        args->SetArgsKind(ArgsKind::kTableConstructor);
        auto tc = std::make_shared<SyntaxTreeTableconstructor>(CovLoc());
        args->SetTableconstructor(tc);
        auto raw = ExtractCallRawArgs(args);
        ASSERT_EQ(raw.size(), 1u);
    }

    EXPECT_FALSE(IsVarargExp(nullptr));
    EXPECT_FALSE(IsVarargExp(CovMakeNumberExp("1")));
    EXPECT_FALSE(IsVarargExp(CovMakeCallPrefixExp("f")));
    EXPECT_TRUE(IsVarargExp(CovMakeVarPrefixExp("__fakelua_vararg_0")));
    EXPECT_FALSE(IsVarargExp(CovMakeVarPrefixExp("x")));

    EXPECT_FALSE(IsFunctionCallExp(nullptr));
    EXPECT_FALSE(IsFunctionCallExp(CovMakeNumberExp("1")));
    EXPECT_TRUE(IsFunctionCallExp(CovMakeCallPrefixExp("f")));
    EXPECT_FALSE(IsFunctionCallExp(CovMakeVarPrefixExp("x")));
}

TEST(common, compile_package_and_strings) {
    EXPECT_EQ(StripLuaStringQuotes("\"abc\""), "abc");
    EXPECT_EQ(StripLuaStringQuotes("'xyz'"), "xyz");
    EXPECT_EQ(StripLuaStringQuotes("bare"), "bare");
    EXPECT_EQ(StripLuaStringQuotes("\""), "\"");

    {
        // package "mypkg"
        auto fc = std::make_shared<SyntaxTreeFunctioncall>(CovLoc());
        fc->SetPrefixexp(CovMakePrefixVar("package"));
        auto args = std::make_shared<SyntaxTreeArgs>(CovLoc());
        args->SetArgsKind(ArgsKind::kString);
        args->SetString(CovMakeStringExp("\"mypkg\""));
        fc->SetArgs(args);
        std::string out;
        EXPECT_TRUE(ExtractPackageName(fc, out));
        EXPECT_EQ(out, "mypkg");
    }
    {
        // package("mypkg") via explist
        auto fc = std::make_shared<SyntaxTreeFunctioncall>(CovLoc());
        fc->SetPrefixexp(CovMakePrefixVar("package"));
        auto args = std::make_shared<SyntaxTreeArgs>(CovLoc());
        args->SetArgsKind(ArgsKind::kExpList);
        auto el = std::make_shared<SyntaxTreeExplist>(CovLoc());
        el->AddExp(CovMakeStringExp("\"pkg2\""));
        args->SetExplist(el);
        fc->SetArgs(args);
        std::string out;
        EXPECT_TRUE(ExtractPackageName(fc, out));
        EXPECT_EQ(out, "pkg2");
    }
    {
        // package = "pkg3"
        auto assign = std::make_shared<SyntaxTreeAssign>(CovLoc());
        auto vl = std::make_shared<SyntaxTreeVarlist>(CovLoc());
        vl->AddVar(CovMakeSimpleVar("package"));
        auto el = std::make_shared<SyntaxTreeExplist>(CovLoc());
        el->AddExp(CovMakeStringExp("\"pkg3\""));
        assign->SetVarlist(vl);
        assign->SetExplist(el);
        std::string out;
        EXPECT_TRUE(ExtractPackageName(assign, out));
        EXPECT_EQ(out, "pkg3");
    }
    {
        auto empty = std::make_shared<SyntaxTreeEmpty>(CovLoc());
        std::string out;
        EXPECT_FALSE(ExtractPackageName(empty, out));
    }

    auto empty = std::make_shared<SyntaxTreeEmpty>(CovLoc());
    EXPECT_FALSE(SyntaxTreeLocationStr("f.lua", empty).empty());
}

TEST(common, compile_number_key_classify) {
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

    auto n = CovMakeNumberExp("7");
    EXPECT_TRUE(ClassifyConstNumberExp(n, kind, canon, iv, fv));
    EXPECT_EQ(iv, 7);

    auto neg = CovMakeUnaryMinus(CovMakeNumberExp("3"));
    EXPECT_TRUE(ClassifyConstNumberExp(neg, kind, canon, iv, fv));
    EXPECT_EQ(iv, -3);

    auto min_neg = CovMakeUnaryMinus(CovMakeNumberExp(std::to_string(std::numeric_limits<int64_t>::min())));
    // May classify as float overflow path; just exercise the branch.
    (void)ClassifyConstNumberExp(min_neg, kind, canon, iv, fv);

    auto two63 = CovMakeUnaryMinus(CovMakeNumberExp("9223372036854775808.0"));
    EXPECT_TRUE(ClassifyConstNumberExp(two63, kind, canon, iv, fv));

    EXPECT_FALSE(ClassifyConstNumberExp(CovMakeStringExp("\"x\""), kind, canon, iv, fv));
    EXPECT_FALSE(ConstNumberExpHasFloatSyntax(nullptr));
    EXPECT_TRUE(ConstNumberExpHasFloatSyntax(CovMakeNumberExp("1.0")));
    EXPECT_FALSE(ConstNumberExpHasFloatSyntax(CovMakeNumberExp("1")));
    EXPECT_TRUE(ConstNumberExpHasFloatSyntax(CovMakeUnaryMinus(CovMakeNumberExp("0.0"))));

    EXPECT_TRUE(ConstNumberExpIsIntValue(TableKeyKind::kInt, CovMakeNumberExp("1")));
    EXPECT_FALSE(ConstNumberExpIsIntValue(TableKeyKind::kInt, CovMakeNumberExp("1.0")));

    const double z = ConstNumberExpToDouble(TableKeyKind::kInt, 0, 0.0, CovMakeNumberExp("0.0"));
    EXPECT_TRUE(z == 0.0);
    const double nz = ConstNumberExpToDouble(TableKeyKind::kInt, 0, 0.0, CovMakeUnaryMinus(CovMakeNumberExp("0.0")));
    EXPECT_TRUE(std::signbit(nz));

    EXPECT_NE(FormatCDoubleLiteral(1.0).find('.'), std::string::npos);
    EXPECT_FALSE(FormatCDoubleLiteral(1.5).empty());

    EXPECT_EQ(EscapeCStringLiteral("a\"b\\c\n\r\td"), "a\\\"b\\\\c\\n\\r\\td");
    EXPECT_NE(EscapeCStringLiteral(std::string("\x01\x7f", 2)).find('\\'), std::string::npos);
}

TEST(common, compile_table_spec_helpers) {
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
    EXPECT_EQ(GetSimpleVarName(CovMakeSimpleVar("x")), "x");
    EXPECT_EQ(GetSimpleVarName(CovMakePrefixVar("y")), "y");
    EXPECT_EQ(GetSimpleVarName(CovMakeVarPrefixExp("z")), "z");
    EXPECT_EQ(GetSimpleVarName(CovMakeNumberExp("1")), "");
}

TEST(common, cov_gaps_lua_misc_paths) {
    State *s = FakeluaNewState();
    ASSERT_NE(s, nullptr);
    CompileConfig config;
    for (auto jit_type: AllJitTypes()) {
        CompileFile(s, "./basic/test_local_multi_assign.lua", config);
        CompileFile(s, "./table/test_table_seq_and_move.lua", config);
        CompileFile(s, "./basic/test_select_and_closure_args.lua", config);
        CompileFile(s, "./basic/test_file_level_literals.lua", config);
        CompileFile(s, "./basic/test_interp_errors.lua", config);
        double res = 0;
        Call(s, jit_type, "test_local_multi_assign", res);
        EXPECT_EQ(res, 0.0) << "jit=" << JitTypeName(jit_type);
        Call(s, jit_type, "test_table_seq_and_move", res);
        EXPECT_EQ(res, 0.0) << "jit=" << JitTypeName(jit_type);
        Call(s, jit_type, "test_select_and_closure_args", res);
        EXPECT_EQ(res, 0.0) << "jit=" << JitTypeName(jit_type);
        Call(s, jit_type, "test_file_level_literals", res);
        EXPECT_EQ(res, 0.0) << "jit=" << JitTypeName(jit_type);
        Call(s, jit_type, "test_bool_nil_locals", res);
        EXPECT_EQ(res, 0.0) << "jit=" << JitTypeName(jit_type);
        Call(s, jit_type, "test_empty_body", res);
        EXPECT_EQ(res, 0.0) << "jit=" << JitTypeName(jit_type);
        Call(s, jit_type, "test_interp_errors", res);
        EXPECT_EQ(res, 0.0) << "jit=" << JitTypeName(jit_type);
    }
    FakeluaDeleteState(s);
}
