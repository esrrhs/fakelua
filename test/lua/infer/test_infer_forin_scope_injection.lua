-- 回归：for-in 循环变量必须注入循环体作用域，
-- 不能污染外层同名局部变量类型。
function test()
    local k = 100
    local sum = 0
    for k, v in pairs({1, 2, 3}) do
        k = k + v
        sum = sum + k
    end
    return k + sum
end
