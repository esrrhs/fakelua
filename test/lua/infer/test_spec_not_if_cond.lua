-- not (comparison) 作为 if 条件，直接使用数学参数（无局部变量中间体）。
-- n 通过 n < 0（IsNativeComparisonExpr: LESS）被识别为数学参数。
-- if 条件 not (n < 0) 应生成原生 C 布尔 !((n) < (0))，不使用 IsTrue。
-- test(5) == 6, test(0) == 1, test(-1) == 0.
local function test(n)
    if not (n < 0) then
        return n + 1
    end
    return 0
end
