-- Test os.date with UTC prefix (!) and numeric timestamp format
-- Also covers os.date with number timestamp as first arg

function test_os_date_utc()
    -- UTC format with "!" prefix
    local d = os.date("!%Y-%m-%d", 0)  -- epoch
    if d == nil then return 1 end
    if type(d) ~= "string" then return 2 end
    -- 1970-01-01
    if d ~= "1970-01-01" then return 3 end
    return 5000
end

function test_os_date_numeric_ts()
    -- os.date with a numeric timestamp as first arg (fakelua extension)
    local d = os.date(0)  -- epoch -> use default format
    if d == nil then return 1 end
    if type(d) ~= "string" then return 2 end
    return 5000
end

function test_os_date_string_ts()
    -- os.date with string-encoded numeric timestamp (fakelua extension)
    local d = os.date("1000000000")  -- 2001-09-09 in UTC
    if d == nil then return 1 end
    if type(d) ~= "string" then return 2 end
    return 5000
end

function test_os_date_table_utc()
    -- os.date("!*t", ts) returns a table in UTC
    local t = os.date("!*t", 0)  -- epoch
    if type(t) ~= "table" then return 1 end
    if t.year ~= 1970 then return 2 end
    if t.month ~= 1 then return 3 end
    if t.day ~= 1 then return 4 end
    if t.hour ~= 0 then return 5 end
    return 5000
end

function test_os_setlocale_query()
    -- Query current locale (empty string = query)
    local loc = os.setlocale("")
    -- Should return a string
    if type(loc) ~= "string" and loc ~= nil then return 1 end
    return 5000
end

function test_os_setlocale_category()
    -- os.setlocale with specific category
    local r = os.setlocale("C", "ctype")
    if type(r) ~= "string" and r ~= nil then return 1 end

    local r2 = os.setlocale("C", "numeric")
    if type(r2) ~= "string" and r2 ~= nil then return 2 end

    local r3 = os.setlocale("C", "time")
    if type(r3) ~= "string" and r3 ~= nil then return 3 end

    local r4 = os.setlocale("C", "monetary")
    if type(r4) ~= "string" and r4 ~= nil then return 4 end

    local r5 = os.setlocale("C", "collate")
    if type(r5) ~= "string" and r5 ~= nil then return 5 end

    return 5000
end
