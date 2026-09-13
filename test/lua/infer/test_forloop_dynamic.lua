-- getVal() 返回 T_DYNAMIC；for 循环变量 i 在循环体内被赋值为 T_DYNAMIC，
-- 导致循环变量类型退化，使用动态 for 循环实现。
-- test 的 n*2 使 n 成为数学参数，test(5) == 10。
function getVal()
    return 0
end

function test(n)
    for i = 1, n do
        i = getVal()
    end
    return n * 2
end
