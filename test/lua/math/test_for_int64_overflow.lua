-- 整数 for 在 maxinteger/mininteger 处步进会溢出。
-- C 的 i++ 把 maxinteger 绕成 mininteger 后条件仍成立，会死循环。
-- 对齐 Lua 5.4：溢出即停止，不再进入下一轮。

function test_for_int64_overflow()
    local n = 0
    for i = math.maxinteger, math.maxinteger do
        n = n + 1
        if n > 3 then return 1 end
    end
    if n ~= 1 then return 2 end

    n = 0
    for i = math.maxinteger - 1, math.maxinteger do
        n = n + 1
        if n > 5 then return 3 end
    end
    if n ~= 2 then return 4 end

    n = 0
    for i = math.mininteger, math.mininteger, -1 do
        n = n + 1
        if n > 3 then return 5 end
    end
    if n ~= 1 then return 6 end

    n = 0
    for i = math.mininteger + 1, math.mininteger, -1 do
        n = n + 1
        if n > 5 then return 7 end
    end
    if n ~= 2 then return 8 end

    -- 动态边界（表取值 → T_DYNAMIC → OpAdd）
    local t = {math.maxinteger}
    n = 0
    for i = t[1], t[1] do
        n = n + 1
        if n > 3 then return 9 end
    end
    if n ~= 1 then return 10 end

    -- 普通循环回归
    local s = 0
    for i = 1, 3 do
        s = s + i
    end
    if s ~= 6 then return 11 end

    return 100
end
