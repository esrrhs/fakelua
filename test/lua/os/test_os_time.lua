function test_os_time()
    -- os.time() returns a number (unix timestamp)
    local t1 = os.time()
    if type(t1) ~= "number" then return 0 end
    -- reasonable range: after year 2000 and before year 2100
    if t1 < 946684800 then return 0 end   -- 2000-01-01
    if t1 > 4102444800 then return 0 end  -- 2100-01-01

    -- os.time(nil) returns current time
    local t2 = os.time(nil)
    if type(t2) ~= "number" then return 0 end

    -- os.time with table: 2024-01-15 10:30:00
    local t3 = os.time({year = 2024, month = 1, day = 15, hour = 10, min = 30, sec = 0})
    if type(t3) ~= "number" then return 0 end
    if t3 < 1700000000 then return 0 end

    -- 动态 Key (VarType::String) 传入 os.time 验证
    local y_key = "ye" .. "ar"
    local m_key = "mon" .. "th"
    local d_key = "d" .. "ay"
    local dyn_tbl = {}
    dyn_tbl[y_key] = 2024
    dyn_tbl[m_key] = 1
    dyn_tbl[d_key] = 15
    dyn_tbl["hour"] = 10
    dyn_tbl["min"] = 30
    dyn_tbl["sec"] = 0
    local t4 = os.time(dyn_tbl)
    if t4 ~= t3 then return 0 end

    return 6000
end
