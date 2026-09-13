-- 当比较表达式的结果作为数学函数的参数传递时，
-- InferNumericBinopResultType 应返回 T_DYNAMIC（比较结果是布尔值）。
-- 这确保 InferNumericBinopResultType 中比较运算符的分支被覆盖。
-- add(n) 通过 n + 0 被识别为数学参数函数。
-- test 用 a < b（比较结果）调用 add，由于结果是 T_DYNAMIC，
-- 走动态调用路径而非特化路径。
local function add(n)
    return n + 0
end

local function test(a, b)
    return add(a < b)
end
