-- 回归：函数体只有 return ... 时，调用点必须展开全部返回值。
-- 旧逻辑把这种函数的 max_returns 记成 1，local a, b = passthrough(10, 20) 只会留下第一个值。

local function passthrough(...)
    return ...
end

function test(n)
    local a, b = passthrough(n, n + 1)
    return a + b
end
