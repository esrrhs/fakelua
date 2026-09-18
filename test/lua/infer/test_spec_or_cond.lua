-- 复合 or 条件：if n < 0 or n > 10 then。
-- n 通过比较（IsNativeComparisonExpr: LESS, MORE）被识别为数学参数，
-- 同时也通过算术 n + 1 得到强化。
-- or 走 CompileBinop 短路，不再生成 C ||。
-- test(5) == 6, test(-1) == 0, test(11) == 0, test(0) == 1, test(10) == 11.
local function test(n)
    if n < 0 or n > 10 then
        return 0
    end
    return n + 1
end
