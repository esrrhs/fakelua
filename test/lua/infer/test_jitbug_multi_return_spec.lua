-- 回归：多返回值 / 变参返回 / 尾位置透传多返回值的函数，即使形参命中数学参数，
-- 也不得生成标量返回值特化（否则特化函数会在 int64_t/double 返回值函数里
-- return FlMakeMulti(...)，产生非法 C：cannot convert 'struct CVar' to 'long long'）。

-- 固定返回 3 个值
local function mr(x)
    local y = x + 1
    return y, y + 1, y + 2
end

-- 尾表达式为 vararg 展开，返回值数量不确定；t 让 x 命中数学参数
local function va(x, ...)
    local t = x + 1
    return t, ...
end

-- 返回 2 个值
local function mr2(x)
    return x, x + 1
end

-- 尾位置透传 mr2 的多返回值；z 让 x 命中数学参数
local function tailmr(x)
    local z = x + 1 - 1
    return mr2(z)
end

local function sum3(a, b, c)
    return a + b + c
end

-- 带单值基线的尾递归：return factacc(...) 的被调链指向自身，定点求解靠
-- concrete 锚点（return acc）解析为有效返回 1，必须仍然生成标量特化。
local function factacc(n, acc)
    if n <= 0 then
        return acc
    end
    return factacc(n - 1, acc * n)
end

function test(n)
    local a, b, c = mr(n)
    local s = a + b + c
    local d, e = va(n, n + 1, n + 2)
    local s2 = d + e
    local f, g = tailmr(n)
    local s3 = f + g
    -- 尾位置把多返回值直接展开进另一个调用
    local s4 = sum3(mr(n))
    -- 尾递归单值函数仍可特化，factacc(5,1)=120
    local s5 = factacc(5, 1)
    return s + s2 + s3 + s4 + s5
end
