package "StringMatchCases"

-- 测试 string.match 捕获
function test_match_capture()
    local s = "date: 2024-01-15"
    local y, m, d = string.match(s, "(\\d+)-(\\d+)-(\\d+)")
    if y ~= "2024" then return 0 end
    if m ~= "01" then return 0 end
    if d ~= "15" then return 0 end
    return 1
end

-- 测试 string.match 无捕获
function test_match_no_capture()
    local s = "hello world"
    local m = string.match(s, "world")
    if m ~= "world" then return 0 end
    return 1
end
