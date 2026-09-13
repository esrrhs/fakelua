-- 当位运算表达式（n & m）的操作数为数学参数时，
-- InferNumericBinopResultType(kBitAnd, T_INT, T_INT) 应返回 T_INT。
-- 这确保 InferNumericBinopResultType 中位运算符的分支被覆盖。
-- add(n) 通过 n + 0 被识别为数学参数函数。
-- test 通过 n + 0 也被识别为数学参数，内部用 n & 1（位运算数学参数）
-- 调用 add，由于位运算结果是 T_INT，走特化调用路径。
local function add(n)
    return n + 0
end

local function test(n)
    local x = n + 0
    return add(n & 1)
end
