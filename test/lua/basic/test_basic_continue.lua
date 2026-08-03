function test_basic_continue()
    -- 测试 while 循环中的 continue
    local sum = 0
    local i = 0
    while i < 10 do
        i = i + 1
        if i % 2 == 0 then continue end
        sum = sum + i
    end
    if sum ~= 25 then return 1 end  -- 1+3+5+7+9 = 25

    -- 测试 for 循环中的 continue
    local count = 0
    for j = 1, 10 do
        if j > 5 then continue end
        count = count + 1
    end
    if count ~= 5 then return 2 end

    -- 测试 repeat 循环中的 continue
    local n = 0
    local total = 0
    repeat
        n = n + 1
        if n % 3 == 0 then continue end
        total = total + n
    until n >= 6
    -- n=1,2,4,5 → total=12 (跳过 3 和 6)
    if total ~= 12 then return 3 end

    -- 测试嵌套循环中的 continue（只影响最内层）
    local outer_sum = 0
    for a = 1, 3 do
        for b = 1, 3 do
            if b == 2 then continue end
            outer_sum = outer_sum + b
        end
    end
    -- 每轮 a: b=1 + b=3 = 4, 共 3 轮 = 12
    if outer_sum ~= 12 then return 4 end

    -- 测试 for-in 循环中的 continue
    local t = {1, 2, 3, 4, 5}
    local t_sum = 0
    for _, v in pairs(t) do
        if v % 2 == 0 then continue end
        t_sum = t_sum + v
    end
    if t_sum ~= 9 then return 5 end  -- 1+3+5 = 9

    return 5000
end
