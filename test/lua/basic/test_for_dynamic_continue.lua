-- 动态 numeric for（while(1)+后置步进）里 continue 必须落到步进处。
-- helper() 返回 T_DYNAMIC，避免走 typed int/float for。
-- 若 continue 被编成 C continue，会跳过 OpAdd，安全阀会返回 1。

function helper()
    return 1
end

function test_for_dynamic_continue()
    local n = 0
    local s = 0
    for i = 1, 6, helper() do
        n = n + 1
        if n > 20 then return 1 end
        if i % 2 == 0 then continue end
        s = s + i
    end
    if n ~= 6 then return 2 end
    if s ~= 9 then return 3 end

    -- inner while continue 不应跳到外层动态 for 的步进
    n = 0
    local inner = 0
    for i = 1, 3, helper() do
        n = n + 1
        if n > 20 then return 4 end
        local w = 0
        while w < 3 do
            w = w + 1
            if w == 2 then continue end
            inner = inner + 1
        end
    end
    if n ~= 3 then return 5 end
    if inner ~= 6 then return 6 end

    -- 动态 for 内的 repeat continue 应走 until，而不是 for 步进
    local total = 0
    for i = 1, 2, helper() do
        local k = 0
        repeat
            k = k + 1
            if k % 2 == 0 then continue end
            total = total + 1
        until k >= 4
    end
    if total ~= 4 then return 7 end

    -- 表取值边界同样走动态 for
    local t = {1, 5}
    n = 0
    s = 0
    for i = t[1], t[2] do
        n = n + 1
        if n > 20 then return 8 end
        if i == 3 then continue end
        s = s + i
    end
    if n ~= 5 then return 9 end
    if s ~= 12 then return 10 end

    return 100
end
