-- 尾调用传递 ... / 多返回值
function ident(...)
    return ...
end

function wrap(...)
    return ident(...)
end

function add3(a, b, c)
    return a + b + c
end

function via_multi(...)
    return add3(...)
end

function test()
    local a, b, c = wrap(10, 20, 30)
    local s = via_multi(1, 2, 3)
    return a + b + c + s
end
