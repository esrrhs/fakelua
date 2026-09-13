function test_basic_collectgarbage()
    -- "count" 返回内存使用量（KB，number 类型）
    local kb = collectgarbage("count")
    if type(kb) ~= "number" then return 0 end
    -- 分配器预分配了 1MB 块，所以应该 > 0
    if kb <= 0 then return 0 end

    -- 默认参数也是 "count"
    local kb2 = collectgarbage()
    if type(kb2) ~= "number" then return 0 end
    if kb2 <= 0 then return 0 end

    -- 其他选项为 no-op，返回 0
    local ret = collectgarbage("collect")
    if ret ~= 0 then return 0 end

    ret = collectgarbage("stop")
    if ret ~= 0 then return 0 end

    return 5000
end
