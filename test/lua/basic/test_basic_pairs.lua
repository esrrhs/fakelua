function test_basic_pairs()
    -- 遍历所有键值对
    local t = {a = 1, b = 2, c = 3}
    local count = 0
    for k, v in pairs(t) do
        count = count + 1
    end
    if count ~= 3 then return 1 end

    -- 空表不执行循环
    local empty_count = 0
    for k, v in pairs({}) do
        empty_count = empty_count + 1
    end
    if empty_count ~= 0 then return 2 end

    -- 混合 key 类型
    local t2 = {x = 10, y = 20}
    local keys = {}
    for k, v in pairs(t2) do
        keys[k] = v
    end
    if keys.x ~= 10 or keys.y ~= 20 then return 3 end

    -- 超过 8 个键会 rehash：以前同时扫 quick_data_ 和桶，会重复遍历
    local t3 = {}
    for i = 1, 15 do t3[i] = i * 10 end
    local n3 = 0
    local sum3 = 0
    local seen3 = {}
    for k, v in pairs(t3) do
        if seen3[k] then return 4 end
        seen3[k] = true
        n3 = n3 + 1
        sum3 = sum3 + v
    end
    if n3 ~= 15 then return 5 end
    if sum3 ~= 10 * (15 * 16 / 2) then return 6 end

    return 5000
end
