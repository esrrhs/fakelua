-- 仅用 <= 和 >= 比较触发数学参数特化：clamp_le(x, lo, hi) 不含算术运算。
-- LESS_EQUAL / MORE_EQUAL 与 LESS / MORE 一样可被 IsNativeComparisonExpr 识别，
-- 从而触发 HasComparisonOperandTypeChange → 数学参数检测。
-- x, lo, hi 均为数学参数 → 8 个特化版本（2^3）。
-- test(5, 1, 10) == 5, test(0, 1, 10) == 1, test(15, 1, 10) == 10.
function clamp_le(x, lo, hi)
    if x <= lo then
        return lo
    end
    if x >= hi then
        return hi
    end
    return x
end

function test(x, lo, hi)
    return clamp_le(x, lo, hi)
end
