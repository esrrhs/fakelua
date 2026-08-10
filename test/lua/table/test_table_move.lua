function test_table_move()
    -- 1. 基本移动（不同表）
    local a = {10, 20, 30, 40, 50}
    local b = {0, 0, 0, 0, 0}
    table.move(a, 1, 3, 2, b)
    if b[2] ~= 10 or b[3] ~= 20 or b[4] ~= 30 then return 0 end

    -- 2. 同表正向移动（t <= f，无需倒序）
    local c = {1, 2, 3, 4, 5}
    table.move(c, 2, 4, 1, c)
    if c[1] ~= 2 or c[2] ~= 3 or c[3] ~= 4 then return 0 end

    -- 3. 同表反向移动（t > e，需倒序避免覆盖）
    local d = {1, 2, 3, 4, 5}
    table.move(d, 1, 3, 3, d)
    if d[3] ~= 1 or d[4] ~= 2 or d[5] ~= 3 then return 0 end

    -- 4. 重叠区域移动
    local e = {1, 2, 3, 4, 5}
    table.move(e, 1, 4, 2, e)
    if e[2] ~= 1 or e[3] ~= 2 or e[4] ~= 3 or e[5] ~= 4 then return 0 end

    -- 5. 空范围（e < f），不应有任何移动
    local f = {1, 2, 3}
    local g = {}
    local result = table.move(f, 3, 1, 1, g)
    if #g ~= 0 then return 0 end

    -- 6. 第 5 参数为 nil 时回退到 a1
    local h = {1, 2, 3}
    table.move(h, 1, 2, 2, nil)
    if h[1] ~= 1 or h[2] ~= 1 or h[3] ~= 2 then return 0 end

    -- 7. 返回目标表
    local src = {10, 20}
    local dst = {}
    local ret = table.move(src, 1, 2, 1, dst)
    if ret ~= dst then return 0 end

    return 5000
end
