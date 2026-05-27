-- 当 or 表达式的两侧操作数均为数学参数时，
-- InferNumericBinopResultType(kOr, T_INT, T_INT) 应返回 left_type (T_INT)。
-- 这确保 InferNumericBinopResultType 中 kOr 的分支被覆盖。
-- add(n) 通过 n + 0 被识别为数学参数函数。
-- test 通过 a + 0, b + 0 使 a, b 被识别为数学参数，
-- 内部用 a or b 调用 add，由于 or 结果是 T_INT，走特化调用路径。
local function add(n)
    return n + 0
end

local function test(a, b)
    local x = a + 0
    local y = b + 0
    return add(a or b)
end
