-- 当连接表达式的操作数为数学参数时，
-- InferNumericBinopResultType(kConcat, T_INT, T_INT) 应返回 T_DYNAMIC。
-- 这确保 InferNumericBinopResultType 中 kConcat 走到最终 return T_DYNAMIC 的分支。
-- add(n) 通过 n + 0 被识别为数学参数函数。
-- test 通过 n + 0 也被识别为数学参数，内部用 n .. n（连接数学参数）
-- 调用 add，由于连接结果是 T_DYNAMIC，走动态调用路径。
local function add(n)
    return n + 0
end

local function test(n)
    local x = n + 0
    return add(n .. n)
end
