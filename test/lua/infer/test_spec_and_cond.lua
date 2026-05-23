-- 复合 and 条件：if a > 0 and b > 0 then。
-- a 和 b 均通过比较（IsNativeComparisonExpr: MORE）被识别为数学参数，
-- 同时也通过算术 a + b 得到强化。
-- if 条件应生成原生 C 布尔 ((a) > (0)) && ((b) > (0))，不使用 IsTrue。
-- test(3, 4) == 7, test(-1, 4) == 0, test(3, -1) == 0, test(0, 0) == 0.
local function test(a, b)
    if a > 0 and b > 0 then
        return a + b
    end
    return 0
end
