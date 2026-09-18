-- 数学特化路径上 and 必须短路：n==0 时 bump() 不能执行。
local function test(n)
    local c = 0
    local function bump()
        c = c + 1
        return n
    end
    if n > 0 and bump() > 0 then
        return c + 100
    end
    return c
end
