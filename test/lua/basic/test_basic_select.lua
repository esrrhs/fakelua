function test_basic_select()
    -- 选择从第 n 个开始的参数
    local a, b, c = select(2, 10, 20, 30, 40)
    if a ~= 20 or b ~= 30 or c ~= 40 then return 1 end

    -- select("#", ...) 返回参数总数
    local count = select("#", 10, 20, 30)
    if count ~= 3 then return 2 end

    -- 选择第一个
    local first = select(1, 100, 200)
    if first ~= 100 then return 3 end

    -- 超出范围返回 nil
    local x = select(5, 1, 2, 3)
    if x ~= nil then return 4 end

    return 5000
end
