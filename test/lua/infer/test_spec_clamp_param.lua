-- 比较运算符触发数学参数特化：clamp(x, lo, hi) 仅用比较，不含算术运算。
-- 三个数学参数 → 8 个特化版本（2^3）。
-- int 全特化 clamp_0_0_0(int64_t x, int64_t lo, int64_t hi) 全部用原生 C 比较。
-- test(5, 1, 10) == 5, test(0, 1, 10) == 1, test(15, 1, 10) == 10.
function clamp(x, lo, hi)
    if x < lo then
        return lo
    end
    if x > hi then
        return hi
    end
    return x
end

function test(x, lo, hi)
    return clamp(x, lo, hi)
end
