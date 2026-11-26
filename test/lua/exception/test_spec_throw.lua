package "SpecThrow"

-- 数值特化函数除零必须出现在 Lua 调用栈上。括号避免解释器尾调用吃掉调用方。
function plain_caller()
    return (divzero(1))
end

function caller(n)
    return (divzero(n))
end

function divzero(n)
    return n // 0
end
