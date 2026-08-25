function test_os_time_boundary()
    -- 1. 无参数（当前时间）
    local t1 = os.time()
    if type(t1) ~= "number" then return 0 end
    if t1 <= 0 then return 0 end

    -- 2. 表参数
    local t2 = os.time({year = 2000, month = 1, day = 1, hour = 0, min = 0, sec = 0})
    if type(t2) ~= "number" then return 0 end
    if t2 <= 0 then return 0 end

    -- 3. 特殊时间值（epoch）
    local t3 = os.time({year = 1970, month = 1, day = 1, hour = 0, min = 0, sec = 0})
    if type(t3) ~= "number" then return 0 end

    -- 4. 不同日期产生不同时间
    local t4a = os.time({year = 2020, month = 6, day = 15})
    local t4b = os.time({year = 2020, month = 6, day = 16})
    if t4b <= t4a then return 0 end

    -- 5. 小时/分钟/秒影响时间
    local t5a = os.time({year = 2020, month = 6, day = 15, hour = 0})
    local t5b = os.time({year = 2020, month = 6, day = 15, hour = 1})
    if t5b <= t5a then return 0 end

    -- 6. 极端 year/month 不得在填 tm 时有符号溢出
    if os.time({year = math.mininteger, month = 1, day = 1}) ~= nil then return 0 end
    if os.time({year = 2020, month = math.mininteger, day = 1}) ~= nil then return 0 end

    -- rehash 后 quick_data_ 会过期，必须走完整表查找
    local trehash = {}
    trehash.year = 2020
    trehash.month = 1
    trehash.day = 1
    trehash.hour = 12
    trehash.min = 0
    trehash.sec = 0
    trehash.wday = 1
    trehash.yday = 1
    trehash.isdst = false
    trehash.year = 2024
    local told = os.time({year = 2020, month = 1, day = 1, hour = 12, min = 0, sec = 0, isdst = false})
    local tnew = os.time({year = 2024, month = 1, day = 1, hour = 12, min = 0, sec = 0, isdst = false})
    local tgot = os.time(trehash)
    if tgot == told then return 0 end
    if tgot ~= tnew then return 0 end

    return 5000
end
