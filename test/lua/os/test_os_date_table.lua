function test_os_date_table()
    -- 测试 os.date("*t") 返回表
    local t = os.date("*t")
    if type(t) ~= "table" then return 0 end

    -- 验证必要字段存在且为数值
    if type(t.year) ~= "number" then return 0 end
    if type(t.month) ~= "number" then return 0 end
    if type(t.day) ~= "number" then return 0 end
    if type(t.hour) ~= "number" then return 0 end
    if type(t.min) ~= "number" then return 0 end
    if type(t.sec) ~= "number" then return 0 end
    if type(t.wday) ~= "number" then return 0 end
    if type(t.yday) ~= "number" then return 0 end
    if type(t.isdst) ~= "boolean" then return 0 end

    -- 验证范围合理性
    if t.month < 1 or t.month > 12 then return 0 end
    if t.day < 1 or t.day > 31 then return 0 end
    if t.hour < 0 or t.hour > 23 then return 0 end
    if t.min < 0 or t.min > 59 then return 0 end
    if t.sec < 0 or t.sec > 61 then return 0 end
    if t.wday < 1 or t.wday > 7 then return 0 end
    if t.yday < 1 or t.yday > 366 then return 0 end

    -- 测试带时间参数的 "*t"
    local t2 = os.date("*t", 0)
    if type(t2) ~= "table" then return 0 end
    -- Unix epoch: 1970-01-01 00:00:00 UTC
    if t2.year ~= 1970 then return 0 end
    if t2.month ~= 1 then return 0 end
    if t2.day ~= 1 then return 0 end

    -- 测试 os.date 字符串格式仍然正常工作
    local s = os.date("%Y-%m-%d")
    if type(s) ~= "string" then return 0 end
    if #s < 8 then return 0 end

    return 5000
end
