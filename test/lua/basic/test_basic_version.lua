function test_basic_version()
    -- _VERSION 应该是字符串
    if type(_VERSION) ~= "string" then return 0 end
    -- 应以 "Fakelua " 开头
    if string.sub(_VERSION, 1, 8) ~= "Fakelua " then return 0 end
    -- 版本号非空
    if #_VERSION <= 8 then return 0 end
    return 5000
end
