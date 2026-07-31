function test_os_date()
    -- os.date() with no args returns a string
    local d1 = os.date()
    if type(d1) ~= "string" then return 0 end
    if #d1 == 0 then return 0 end

    -- os.date("%Y-%m-%d") returns formatted date
    local d2 = os.date("%Y-%m-%d")
    if type(d2) ~= "string" then return 0 end
    -- format: YYYY-MM-DD (10 chars)
    if #d2 ~= 10 then return 0 end

    -- os.date("%Y") returns 4-digit year
    local year = os.date("%Y")
    if #year ~= 4 then return 0 end

    -- os.date("!%Y-%m-%d") UTC format
    local d3 = os.date("!%Y-%m-%d")
    if type(d3) ~= "string" then return 0 end
    if #d3 ~= 10 then return 0 end

    -- os.date(nil) should return default format
    local d4 = os.date(nil)
    if type(d4) ~= "string" then return 0 end

    return 6000
end
