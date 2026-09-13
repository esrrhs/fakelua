-- while 循环中 not (comparison) 作为条件，直接使用数学参数。
-- n 通过比较（IsNativeComparisonExpr: LESS）和算术（n - 1）被识别为数学参数。
-- while 条件 not (n < 0) 应生成原生 C 布尔 !((n) < (0))，不使用 IsTrue。
-- 等价于 while n >= 0 do s = s + n; n = n - 1 end。
-- test(0) == 0, test(1) == 1, test(5) == 15 (= 0+1+2+3+4+5).
local function test(n)
    local s = 0
    while not (n < 0) do
        s = s + n
        n = n - 1
    end
    return s
end
