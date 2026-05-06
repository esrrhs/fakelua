-- repeat...until 循环内数学参数参与循环体算术运算。
-- n 被用于 sum = sum + n（整数算术），使 n 成为数学参数。
-- until 条件使用 TryCompileNativeBoolExpr 编译为原生布尔表达式。
-- 循环固定迭代 5 次（i 从 1 到 5），每次 sum += n。
-- test(3) == 3*5 = 15, test(2) == 2*5 = 10.
function test(n)
    local sum = 0
    local i = 1
    repeat
        sum = sum + n
        i = i + 1
    until i > 5
    return sum
end
