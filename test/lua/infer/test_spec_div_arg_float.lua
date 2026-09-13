-- 当除法表达式（a / b）的操作数为数学参数时，
-- InferNumericBinopResultType(kSlash, T_INT, T_INT) 应返回 T_FLOAT。
-- 这确保 InferNumericBinopResultType 中 kSlash/kPow 的分支被覆盖。
-- add(n) 通过 n + 0 被识别为数学参数函数。
-- test 通过 a + 0, b + 0 使 a, b 被识别为数学参数，
-- 内部用 a / b 调用 add，由于除法结果是 T_FLOAT，走 add_1（double 特化）。
local function add(n)
    return n + 0
end

local function test(a, b)
    local x = a + 0
    local y = b + 0
    return add(a / b)
end
