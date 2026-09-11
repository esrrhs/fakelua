#include "fk_test.h"

#include <cstring>
#include <string>

namespace {

bool has_op(const char *dump, const char *op) {
    if (!dump || !op) {
        return false;
    }
    std::string tok = std::string("[") + op + "]";
    return strstr(dump, tok.c_str()) != nullptr;
}

}  // namespace

class OptimizerEnv : public FakeEnv {
protected:
    std::string dump_buf;

    const char *Dump(const char *func = "main") {
        const char *d = fkdumpfunc(fk, func);
        EXPECT_NE(d, nullptr) << func;
        dump_buf = d ? d : "";
        return dump_buf.c_str();
    }

    void ExpectNoOp(const char *op, const char *func = "main") {
        EXPECT_FALSE(has_op(Dump(func), op)) << op << " still in " << func << ":\n" << Dump(func);
    }

    void ExpectOp(const char *op, const char *func = "main") {
        EXPECT_TRUE(has_op(Dump(func), op)) << op << " missing in " << func << ":\n" << Dump(func);
    }

    void ExpectError(const char *src) {
        Parse(src);
        fkrun<int>(fk, "main");
        EXPECT_NE(fkerror(fk), efk_ok) << fkerrorstr(fk);
    }
};

TEST_F(OptimizerEnv, FoldConstAddRemovedFromDump) {
    Parse("func main() return 1 + 2 end");
    EXPECT_EQ(Run<int>(), 3);
    ExpectNoOp("PLUS");
}

TEST_F(OptimizerEnv, FoldConstMulDivMod) {
    Parse("func main() return (6 * 7) + (8 / 2) + (10 % 3) end");
    EXPECT_EQ(Run<int>(), 47);
    ExpectNoOp("MULTIPLY");
    ExpectNoOp("DIVIDE");
    ExpectNoOp("DIVIDE_MOD");
    ExpectNoOp("PLUS");
}

TEST_F(OptimizerEnv, FoldConstSubAndUnary) {
    Parse("func main() return 0 - (10 % 3) end");
    EXPECT_EQ(Run<int>(), -1);
    ExpectNoOp("MINUS");
    ExpectNoOp("DIVIDE_MOD");
}

TEST_F(OptimizerEnv, FoldMixedExprDump) {
    Parse("func main() return 1 + 2 * 3 - 4 end");
    EXPECT_EQ(Run<int>(), 3);
    ExpectNoOp("PLUS");
    ExpectNoOp("MULTIPLY");
    ExpectNoOp("MINUS");
}

TEST_F(OptimizerEnv, FoldFloatMul) {
    Parse("func main() return 1.5 * 2 end");
    EXPECT_DOUBLE_EQ(Run<double>(), 3.0);
    ExpectNoOp("MULTIPLY");
}

TEST_F(OptimizerEnv, FoldStringCat) {
    Parse("func main() return \"a\"..\"b\"..\"c\" end");
    EXPECT_STREQ(Run<const char *>(), "abc");
    ExpectNoOp("STRING_CAT");
}

TEST_F(OptimizerEnv, FoldConstKeyword) {
    Parse(
            "const c = 5\n"
            "func main() return c + 1 end\n");
    EXPECT_EQ(Run<int>(), 6);
    ExpectNoOp("PLUS");
}

TEST_F(OptimizerEnv, UnusedAddIsDead) {
    Parse(
            "func main()\n"
            "	var x = 1 + 2\n"
            "	var y = 3 * 4\n"
            "	return 7\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 7);
    ExpectNoOp("PLUS");
    ExpectNoOp("MULTIPLY");
}

TEST_F(OptimizerEnv, UnusedDivByZeroStillErrors) {
    ExpectError(
            "func main()\n"
            "	var x = 1 / 0\n"
            "	return 1\n"
            "end\n");
}

TEST_F(OptimizerEnv, UnusedModuloZeroStillErrors) {
    ExpectError(
            "func main()\n"
            "	var x = 1 % 0\n"
            "	return 1\n"
            "end\n");
}

TEST_F(OptimizerEnv, UnusedStringPlusStillErrors) {
    ExpectError(
            "func main()\n"
            "	var x = \"a\" + \"b\"\n"
            "	return 1\n"
            "end\n");
}

TEST_F(OptimizerEnv, UnusedCallIsKept) {
    Parse(
            "func main()\n"
            "	test_cfunc1(9, 3)\n"
            "	return 4\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 4);
    ExpectOp("CALL");
}

TEST_F(OptimizerEnv, ArrayStoreNotDeleted) {
    Parse(
            "func main()\n"
            "	var a = array()\n"
            "	a[0] = 1\n"
            "	a[1] = 2\n"
            "	return size(a)\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 2);
}

TEST_F(OptimizerEnv, DeadFalseBranchRemovesTrap) {
    Parse(
            "func main()\n"
            "	if false then\n"
            "		return 1 / 0\n"
            "	end\n"
            "	return 2\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 2);
    ExpectNoOp("DIVIDE");
}

TEST_F(OptimizerEnv, DeadElseOfTrueRemovesTrap) {
    Parse(
            "func main()\n"
            "	if true then\n"
            "		return 4\n"
            "	else\n"
            "		return 1 / 0\n"
            "	end\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 4);
    ExpectNoOp("DIVIDE");
}

TEST_F(OptimizerEnv, DeadElseifTrapsRemoved) {
    Parse(
            "func main()\n"
            "	if false then\n"
            "		return 1 / 0\n"
            "	elseif true then\n"
            "		return 5\n"
            "	else\n"
            "		return 2 / 0\n"
            "	end\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 5);
    ExpectNoOp("DIVIDE");
}

TEST_F(OptimizerEnv, ConstOrAndBranch) {
    Parse(
            "func main()\n"
            "	if false or true then\n"
            "		return 1\n"
            "	else\n"
            "		return 1 / 0\n"
            "	end\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 1);
    ExpectNoOp("DIVIDE");
}

TEST_F(OptimizerEnv, ConstAndFalseBranch) {
    Parse(
            "func main()\n"
            "	if true and false then\n"
            "		return 1 / 0\n"
            "	end\n"
            "	return 8\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 8);
    ExpectNoOp("DIVIDE");
}

TEST_F(OptimizerEnv, NotFalseTaken) {
    Parse(
            "func main()\n"
            "	if not false then\n"
            "		return 8\n"
            "	end\n"
            "	return 1 / 0\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 8);
    ExpectNoOp("DIVIDE");
}

TEST_F(OptimizerEnv, WhileFalseBodyRemoved) {
    Parse(
            "func main()\n"
            "	while false then\n"
            "		return 1 / 0\n"
            "	end\n"
            "	return 4\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 4);
    ExpectNoOp("DIVIDE");
}

TEST_F(OptimizerEnv, UnreachableAfterReturn) {
    Parse(
            "func main()\n"
            "	return 3\n"
            "	return 1 / 0\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 3);
    ExpectNoOp("DIVIDE");
}

TEST_F(OptimizerEnv, SccpCopyChainGuardsDiv) {
    Parse(
            "func main()\n"
            "	var a = 0\n"
            "	var b = a\n"
            "	var c = b\n"
            "	if is c then\n"
            "		return 1 / 0\n"
            "	end\n"
            "	return 9\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 9);
    ExpectNoOp("DIVIDE");
}

TEST_F(OptimizerEnv, SccpArithThenCmp) {
    Parse(
            "func main()\n"
            "	var a = 2\n"
            "	var b = a * 3\n"
            "	var c = b - 1\n"
            "	if c == 5 then\n"
            "		return 11\n"
            "	end\n"
            "	return 1 / 0\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 11);
    ExpectNoOp("MULTIPLY");
    ExpectNoOp("MINUS");
    ExpectNoOp("DIVIDE");
}

TEST_F(OptimizerEnv, SccpJoinSameConstantFoldsAdd) {
    Parse(
            "func f(n)\n"
            "	var x = 1\n"
            "	if n != 0 then\n"
            "		x = 1\n"
            "	else\n"
            "		x = 1\n"
            "	end\n"
            "	return x + 2\n"
            "end\n"
            "func main()\n"
            "	return f(0) + f(3)\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 6);
    ExpectNoOp("PLUS", "f");
}

TEST_F(OptimizerEnv, SccpJoinDifferentKeepsBoth) {
    Parse(
            "func f(n)\n"
            "	var x = 0\n"
            "	if n != 0 then\n"
            "		x = 1\n"
            "	else\n"
            "		x = 2\n"
            "	end\n"
            "	return x\n"
            "end\n"
            "func main()\n"
            "	return f(1) * 10 + f(0)\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 12);
}

TEST_F(OptimizerEnv, CopyPropIntoAdd) {
    Parse(
            "func main()\n"
            "	var a = 5\n"
            "	var b = a\n"
            "	return b + 1\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 6);
    ExpectNoOp("PLUS");
}

TEST_F(OptimizerEnv, MathAssignFolded) {
    Parse(
            "func main()\n"
            "	var x = 1\n"
            "	x += 2\n"
            "	x *= 3\n"
            "	return x\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 9);
}

TEST_F(OptimizerEnv, GvnCommonSubexpr) {
    Parse(
            "func f(a, b)\n"
            "	var x = a + b\n"
            "	var y = a + b\n"
            "	return x + y\n"
            "end\n"
            "func main()\n"
            "	return f(3, 4)\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 14);
}

TEST_F(OptimizerEnv, GvnSubtractSame) {
    Parse(
            "func f(a, b)\n"
            "	var x = a * b\n"
            "	var y = a * b\n"
            "	return x - y\n"
            "end\n"
            "func main()\n"
            "	return f(9, 8)\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 0);
}

TEST_F(OptimizerEnv, ParamAddNotFolded) {
    Parse(
            "func f(n)\n"
            "	return n + 1\n"
            "end\n"
            "func main()\n"
            "	return f(4)\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 5);
    ExpectOp("PLUS", "f");
}

TEST_F(OptimizerEnv, ForRangeNotDeleted) {
    Parse(
            "func main()\n"
            "	var s = 0\n"
            "	for var i = 1 -> 5, 1 then\n"
            "		s += i\n"
            "	end\n"
            "	return s\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 10);
    ExpectOp("FOR");
}

TEST_F(OptimizerEnv, ForCStyleNotDeleted) {
    Parse(
            "func main()\n"
            "	var s = 0\n"
            "	for var i = 0, i < 4, i++ then\n"
            "		s += i\n"
            "	end\n"
            "	return s\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 6);
}

TEST_F(OptimizerEnv, WhileLiveNotDeleted) {
    Parse(
            "func main()\n"
            "	var i = 0\n"
            "	while i < 3 then\n"
            "		i += 1\n"
            "	end\n"
            "	return i\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 3);
}

TEST_F(OptimizerEnv, SwitchConstPicksCase) {
    Parse(
            "func main()\n"
            "	switch 2\n"
            "		case 1 then\n"
            "			return 1 / 0\n"
            "		case 2 then\n"
            "			return 20\n"
            "		default\n"
            "			return 2 / 0\n"
            "	end\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 20);
}

TEST_F(OptimizerEnv, NestedIfConstants) {
    Parse(
            "func main()\n"
            "	if true then\n"
            "		if false then\n"
            "			return 1 / 0\n"
            "		else\n"
            "			return 6\n"
            "		end\n"
            "	end\n"
            "	return 0\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 6);
    ExpectNoOp("DIVIDE");
}

TEST_F(OptimizerEnv, IsConstTaken) {
    Parse(
            "func main()\n"
            "	if is 1 then\n"
            "		return 1\n"
            "	end\n"
            "	return 0\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 1);
}

TEST_F(OptimizerEnv, CompareConstBranch) {
    Parse(
            "func main()\n"
            "	if 3 > 2 then\n"
            "		return 9\n"
            "	end\n"
            "	return 1 / 0\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 9);
    ExpectNoOp("DIVIDE");
    ExpectNoOp("MORE_JNE");
    ExpectNoOp("MORE");
}
