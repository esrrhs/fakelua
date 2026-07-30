function test_basic_ipairs()
    -- 遍历数组
    local arr = {10, 20, 30}
    local sum = 0
    for i, v in ipairs(arr) do
        sum = sum + v
    end
    if sum ~= 60 then return 1 end

    -- 空表不执行循环
    local empty_count = 0
    for i, v in ipairs({}) do
        empty_count = empty_count + 1
    end
    if empty_count ~= 0 then return 2 end

    -- 验证索引和值
    local result = {}
    for i, v in ipairs({100, 200, 300}) do
        result[i] = v
    end
    if result[1] ~= 100 or result[2] ~= 200 or result[3] ~= 300 then return 3 end

    return 5000
end
