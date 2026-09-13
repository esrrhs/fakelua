-- Bug 3: or 运算符当左侧为数值类型时结果应为 left_type，无论右侧类型。
-- 数值（包括0）在 Lua 中始终为真值，因此 `a or b` 当 a 为数值时短路返回 a。
-- 修复前：仅当两侧均为数值类型时才推断 or 的结果为 left_type。
-- 修复后：左侧为数值类型即可推断结果为 left_type。
-- test(5) == 5 (n or get_default() 短路，不调用 get_default)
function test(n)
    local x = n or 0
    return x + 1
end
