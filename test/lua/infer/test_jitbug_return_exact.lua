-- 回归：有效返回数是每条路径的精确值，不是 max()。
-- exactly_one 的 return x + 1, zero() 在 zero 返回 0 个值时实际只有 1 个值，caller_one 仍可特化。
-- mixed 一条路径返回 1 个值、另一条返回 0 个值，caller_mixed 不得特化。

local function zero()
    return
end

local function exactly_one(x)
    return x + 1, zero()
end

local function mixed(x)
    if x > 0 then
        return x
    end
    return zero()
end

function caller_one(n)
    local y = n + 1 - 1
    return exactly_one(y)
end

function caller_mixed(n)
    local y = n + 1 - 1
    return mixed(y)
end

function test(n)
    return caller_one(n) + caller_mixed(n)
end
