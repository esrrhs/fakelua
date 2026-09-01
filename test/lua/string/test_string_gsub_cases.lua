package "StringGsubCases"

-- 测试 string.gsub 函数替换
function test_gsub_function()
    local s = "hello world"
    local r = string.gsub(s, "\\w+", function(w)
        return string.upper(w)
    end)
    if r ~= "HELLO WORLD" then return 0 end
    return 1
end

-- 测试 string.gsub 表替换
function test_gsub_table()
    local s = "a b"
    local t = {a="x", b="y"}
    local r = string.gsub(s, "(\\w+)", t)
    if r ~= "x y" then return 0 end
    return 1
end

-- 测试 string.gsub 带计数限制
function test_gsub_limit()
    local s = "aaa aaa aaa"
    local r, n = string.gsub(s, "aaa", "b", 2)
    if n ~= 2 then return 0 end
    if r ~= "b b aaa" then return 0 end
    return 1
end
