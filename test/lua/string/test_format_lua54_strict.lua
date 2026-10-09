-- Lua 5.4 string.format 严格整数转换与非法转换符
function test_format_lua54_strict()
    -- %d/%i/%u/%x/%o：float 必须能精确表示为整数
    if string.format("%d", 3.0) ~= "3" then return 1 end
    local ok1 = pcall(function() return string.format("%d", 3.9) end)
    if ok1 then return 2 end
    local ok2 = pcall(function() return string.format("%d", 1e20) end)
    if ok2 then return 3 end
    local ok3 = pcall(function() return string.format("%d", math.huge) end)
    if ok3 then return 4 end
    local ok4 = pcall(function() return string.format("%d", 0 / 0) end)
    if ok4 then return 5 end
    local ok5 = pcall(function() return string.format("%x", 3.5) end)
    if ok5 then return 6 end
    -- %u 同样接受精确整数 float
    if string.format("%u", 2.0) ~= "2" then return 7 end
    -- %c 也走严格整数：整数 OK，小数报错
    if string.format("%c", 65) ~= "A" then return 20 end
    local ok7 = pcall(function() return string.format("%c", 65.9) end)
    if ok7 then return 21 end

    -- 未识别的转换符一律报错，绝不原样输出
    local bad = { "%z", "%", "%1", "%.*f", "%w", "%Z" }
    for i = 1, #bad do
        local ok = pcall(function() return string.format(bad[i], 1) end)
        if ok then return 20 + i end
    end

    -- 正常格式不受影响
    if string.format("%d|%5d|%-5d|%05d", 42, 42, 42, 42) ~= "42|   42|42   |00042" then return 40 end
    if string.format("%x|%X|%o|%u", 255, 255, 8, 10) ~= "ff|FF|10|10" then return 41 end
    if string.format("%.2f", 1.5) ~= "1.50" then return 42 end
    if string.format("%+d|% d|%+d", 42, 42, -42) ~= "+42| 42|-42" then return 43 end
    if string.format("100%%") ~= "100%" then return 44 end

    return 5000
end
