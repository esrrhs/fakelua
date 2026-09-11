#include "fk_test.h"

#include <cstring>

TEST_F(FakeEnv, IntegerArithmetic) {
    Parse("func main() return 1 + 2 * 3 - 4 end");
    EXPECT_EQ(Run<int>(), 3);
}

TEST_F(FakeEnv, ModuloAndUnary) {
    Parse("func main() return 0 - (10 % 3) end");
    EXPECT_EQ(Run<int>(), -1);
}

TEST_F(FakeEnv, Float) {
    Parse("func main() return 1.5 + 2.5 end");
    EXPECT_DOUBLE_EQ(Run<double>(), 4.0);
}

TEST_F(FakeEnv, ComparisonsAndOrNot) {
    Parse(
            "func main()\n"
            "	if (1 < 2 and 3 != 4) or not 0 then\n"
            "		return 1\n"
            "	end\n"
            "	return 0\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 1);
}

TEST_F(FakeEnv, IfElse) {
    Parse(
            "func main()\n"
            "	if 1 > 2 then\n"
            "		return 9\n"
            "	else\n"
            "		return 8\n"
            "	end\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 8);
}

TEST_F(FakeEnv, Elseif) {
    Parse(
            "func main()\n"
            "	var a = 2\n"
            "	if a < 1 then\n"
            "		return 1\n"
            "	elseif a == 2 then\n"
            "		return 2\n"
            "	else\n"
            "		return 3\n"
            "	end\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 2);
}

TEST_F(FakeEnv, WhileBreak) {
    Parse(
            "func main()\n"
            "	var i = 0\n"
            "	while true then\n"
            "		i += 1\n"
            "		if i >= 4 then\n"
            "			break\n"
            "		end\n"
            "	end\n"
            "	return i\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 4);
}

TEST_F(FakeEnv, ForContinue) {
    Parse(
            "func main()\n"
            "	var s = 0\n"
            "	for var i = 0, i < 5, i++ then\n"
            "		if i == 2 then\n"
            "			continue\n"
            "		end\n"
            "		s += i\n"
            "	end\n"
            "	return s\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 0 + 1 + 3 + 4);
}

TEST_F(FakeEnv, ForRange) {
    Parse(
            "func main()\n"
            "	var s = 0\n"
            "	for var i = 1 -> 5, 1 then\n"
            "		s += i\n"
            "	end\n"
            "	return s\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 10);
}

TEST_F(FakeEnv, Recursion) {
    Parse(
            "func sum(n)\n"
            "	if n <= 0 then\n"
            "		return 0\n"
            "	end\n"
            "	return n + sum(n - 1)\n"
            "end\n"
            "func main() return sum(5) end\n");
    EXPECT_EQ(Run<int>(), 15);
}

TEST_F(FakeEnv, MultipleReturn) {
    Parse(
            "func pair()\n"
            "	return 3, 4\n"
            "end\n"
            "func main()\n"
            "	var a, var b = pair()\n"
            "	return a * b\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 12);
}

TEST_F(FakeEnv, CallByStringName) {
    Parse(
            "func add(a, b) return a + b end\n"
            "func main()\n"
            "	var f = \"add\"\n"
            "	return f(2, 5)\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 7);
}

TEST_F(FakeEnv, ArrayMapSize) {
    Parse(
            "func main()\n"
            "	var a = array()\n"
            "	a[0] = 1\n"
            "	a[1] = 2\n"
            "	var m = map()\n"
            "	m[\"x\"] = 9\n"
            "	return size(a) + m[\"x\"]\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 11);
}

TEST_F(FakeEnv, NestedArrayMap) {
    Parse(
            "func main()\n"
            "	var m = map()\n"
            "	var a = array()\n"
            "	a[0] = 7\n"
            "	m[\"a\"] = a\n"
            "	var inner = m[\"a\"]\n"
            "	return inner[0]\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 7);
}

TEST_F(FakeEnv, StringConcat) {
    Parse(
            "func main()\n"
            "	return \"a\"..\"b\"..1\n"
            "end\n");
    EXPECT_STREQ(Run<const char *>(), "ab1");
}

TEST_F(FakeEnv, Format) {
    Parse("func main() return format(\"u%\", 3) end");
    EXPECT_STREQ(Run<const char *>(), "u3");
}

TEST_F(FakeEnv, Typeof) {
    Parse("func main() return typeof(1) end");
    EXPECT_STREQ(Run<const char *>(), "REAL");
}

TEST_F(FakeEnv, NullCompare) {
    Parse(
            "func main()\n"
            "	if 1 != null then\n"
            "		return 1\n"
            "	end\n"
            "	return 0\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 1);
}

TEST_F(FakeEnv, SwitchCase) {
    Parse(
            "func main()\n"
            "	var x = 2\n"
            "	switch x\n"
            "		case 1 then\n"
            "			return 10\n"
            "		case 2 then\n"
            "			return 20\n"
            "		default\n"
            "			return 30\n"
            "	end\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 20);
}

TEST_F(FakeEnv, SwitchDefault) {
    Parse(
            "func main()\n"
            "	switch 9\n"
            "		case 1 then\n"
            "			return 10\n"
            "		default\n"
            "			return 30\n"
            "	end\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 30);
}

TEST_F(FakeEnv, Const) {
    Parse(
            "const c = 5\n"
            "func main() return c + 1 end\n");
    EXPECT_EQ(Run<int>(), 6);
}

TEST_F(FakeEnv, ConstMapLiteral) {
    Parse(
            "const c = {\n"
            "	\"a\" : 1\n"
            "}\n"
            "func main()\n"
            "	return c[\"a\"]\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 1);
}

TEST_F(FakeEnv, StructField) {
    Parse(
            "struct point\n"
            "	x\n"
            "	y\n"
            "end\n"
            "func main()\n"
            "	var p = point()\n"
            "	p->x = 3\n"
            "	p->y = 4\n"
            "	return p->x + p->y\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 7);
}

TEST_F(FakeEnv, DeclareAssign) {
    Parse(
            "func pair() return 1, 2 end\n"
            "func main()\n"
            "	a, var b := pair()\n"
            "	return a + b\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 3);
}

TEST_F(FakeEnv, DostringDefinesFunc) {
    Parse(
            "func main()\n"
            "	dostring(\"func extra() return 11 end\")\n"
            "	return extra()\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 11);
}

TEST_F(FakeEnv, Isinteger) {
    Parse(
            "func main()\n"
            "	if is isinteger(3) then\n"
            "		if is isinteger(1.5) then\n"
            "			return 0\n"
            "		end\n"
            "		return 1\n"
            "	end\n"
            "	return 0\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 1);
}

TEST_F(FakeEnv, Int64) {
    Parse(
            "func main()\n"
            "	var a = 7u\n"
            "	return tolong(a)\n"
            "end\n");
    EXPECT_EQ(Run<int64_t>(), 7);
}

TEST_F(FakeEnv, CFunction) {
    Parse("func main() return test_cfunc1(10, 3) end");
    EXPECT_EQ(Run<int>(), 7);
}

TEST_F(FakeEnv, DivByZero) {
    Parse("func main() return 1 / 0 end");
    fkrun<int>(fk, "main");
    EXPECT_NE(fkerror(fk), efk_ok);
}

TEST_F(FakeEnv, StringPlusIsNotConcat) {
    Parse("func main() return \"a\" + \"b\" end");
    fkrun<int>(fk, "main");
    EXPECT_NE(fkerror(fk), efk_ok);
}

TEST_F(FakeEnv, UnusedDivByZeroStillErrors) {
    Parse(
            "func main()\n"
            "	var x = 1 / 0\n"
            "	return 1\n"
            "end\n");
    fkrun<int>(fk, "main");
    EXPECT_NE(fkerror(fk), efk_ok);
}

TEST_F(FakeEnv, UnusedStringPlusStillErrors) {
    Parse(
            "func main()\n"
            "	var x = \"a\" + \"b\"\n"
            "	return 1\n"
            "end\n");
    fkrun<int>(fk, "main");
    EXPECT_NE(fkerror(fk), efk_ok);
}

TEST_F(FakeEnv, UnusedAddIsFoldedAway) {
    Parse(
            "func main()\n"
            "	var x = 1 + 2\n"
            "	return 7\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 7);
    const char *d = fkdumpfunc(fk, "main");
    ASSERT_NE(d, nullptr);
    EXPECT_EQ(strstr(d, "[PLUS]"), nullptr);
}

TEST_F(FakeEnv, DeadFalseBranchIsRemoved) {
    Parse(
            "func main()\n"
            "	if false then\n"
            "		return 1 / 0\n"
            "	end\n"
            "	return 2\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 2);
}

TEST_F(FakeEnv, SccpZeroCopyGuardsDiv) {
    Parse(
            "func main()\n"
            "	var a = 0\n"
            "	var b = a\n"
            "	if is b then\n"
            "		return 1 / 0\n"
            "	end\n"
            "	return 9\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 9);
}

TEST_F(FakeEnv, SccpJoinSameConstant) {
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
            "	return f(3)\n"
            "end\n");
    EXPECT_EQ(Run<int>(), 3);
}
