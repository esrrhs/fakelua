-- 回归：return g() 按词法作用域解析被调，不能用文件级同名函数的返回数。
-- shadowed 里的 local g 返回两个值，即使文件级 g 只返回一个值，也不得特化。
-- 形参 g 同样遮蔽文件级 g。
-- 没有同名文件级函数的嵌套 inner 精确返回 1 个值时，外层仍可特化。

function g(x)
    return x + 1
end

function shadowed(n)
    local function g(x)
        local y = x + 0
        return y, y + 1
    end
    local z = n + 1 - 1
    return g(z)
end

function use_nested(n)
    local function inner(x)
        return x + 1
    end
    local y = n + 1 - 1
    return inner(y)
end

function apply(n, g)
    local y = n + 1 - 1
    return g(y)
end

function test(n)
    local a, b = shadowed(n)
    local c = use_nested(n)
    local d, e = apply(n, function(x)
        return x, x + 10
    end)
    return a + b + c + d + e
end
