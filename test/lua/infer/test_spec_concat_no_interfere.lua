-- .. 拼接运算符与算术运算共存时，不应阻止数学参数特化。
-- n 通过算术 n + 1 被识别为数学参数；
-- 拼接运算符 .. 的结果为 T_DYNAMIC，不在 IsArithmeticExpr 中，
-- 因此不应影响 n 的特化发现。
-- test(5) == 6, test(0) == 1, test(-1) == 0, test(3) == 4.
local function test(n)
    local s = "prefix" .. "suffix"
    return n + 1
end
