function test_basic_version()
    -- _VERSION 应该是字符串
    if type(_VERSION) ~= "string" then return 0 end
    -- 应包含 "Fakelua" 或 "Lua" 关键字
    if string.find(_VERSION, "Fakelua") == nil and string.find(_VERSION, "Lua") == nil then return 0 end
    -- 非空
    if #_VERSION == 0 then return 0 end
    return 5000
end
