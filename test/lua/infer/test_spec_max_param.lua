-- 比较运算符触发数学参数特化：max(a, b) 仅用比较，不含算术运算。
-- 特化后 int 版 max_0(int64_t a, int64_t b) 用原生 C 比较，无 CVar 装拆箱。
-- test(3, 5) == 5, test(7, 2) == 7, test(4, 4) == 4.
function max(a, b)
    if a > b then
        return a
    end
    return b
end

function test(a, b)
    return max(a, b)
end
