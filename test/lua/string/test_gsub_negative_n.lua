-- Lua 5.4: gsub 第四参缺省/nil 时为 srcl+1（不限次数）；显式传入的负数或 0
-- 意味着一次都不替换（while (n < max_s)），而不是"不限次数"。
function test_gsub_negative_n()
    local r1, c1 = ("aaa"):gsub("a", "X", -1)
    if r1 ~= "aaa" or c1 ~= 0 then return 1 end

    local r2, c2 = ("hello"):gsub("l", "L", -2)
    if r2 ~= "hello" or c2 ~= 0 then return 2 end

    local r3, c3 = ("aaa"):gsub("a", "X", 0)
    if r3 ~= "aaa" or c3 ~= 0 then return 3 end

    local r4, c4 = ("aaa"):gsub("a", "X", 2)
    if r4 ~= "XXa" or c4 ~= 2 then return 4 end

    -- 缺省：全部替换
    local r5, c5 = ("aaa"):gsub("a", "X")
    if r5 ~= "XXX" or c5 ~= 3 then return 5 end

    -- 显式 nil 与缺省等价
    local r6, c6 = ("aaa"):gsub("a", "X", nil)
    if r6 ~= "XXX" or c6 ~= 3 then return 6 end

    -- 大于匹配总数的值等同于全部替换
    local r7, c7 = ("aaa"):gsub("a", "X", 100)
    if r7 ~= "XXX" or c7 ~= 3 then return 7 end

    return 5000
end
