function test_os_date_format()
    -- 1. %Y-%m-%d 格式
    local s1 = os.date("%Y-%m-%d")
    if type(s1) ~= "string" then return 1 end
    if #s1 ~= 10 then return 2 end  -- YYYY-MM-DD = 10 chars

    -- 2. %H:%M:%S 格式
    local s2 = os.date("%H:%M:%S")
    if type(s2) ~= "string" then return 3 end
    if #s2 ~= 8 then return 4 end  -- HH:MM:SS = 8 chars

    -- 3. 组合格式
    local s3 = os.date("%Y-%m-%d %H:%M:%S")
    if type(s3) ~= "string" then return 5 end
    if #s3 ~= 19 then return 6 end  -- YYYY-MM-DD HH:MM:SS = 19 chars

    -- 4. 带时间参数
    local s4 = os.date("%Y-%m-%d", 0)
    if type(s4) ~= "string" then return 7 end

    -- 5. 其他常用格式
    local s5 = os.date("%A")  -- 星期几
    if type(s5) ~= "string" then return 8 end

    local s6 = os.date("%B")  -- 月份名
    if type(s6) ~= "string" then return 9 end

    return 5000
end
