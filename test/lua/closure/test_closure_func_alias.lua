function my_add(x, y, z)
    return x + y + z
end

function my_mul(x, y)
    return x * y
end

function test()
    -- 1. 将已定义的函数赋值给局部变量
    local alias_add = my_add
    local res1 = alias_add(10, 20, 30)

    -- 2. 将已定义的函数赋值给 table 字段
    local ops = {}
    ops.calc = my_mul
    local res2 = ops.calc(5, 6)

    return res1 + res2
end
