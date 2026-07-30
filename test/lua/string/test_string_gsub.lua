function test_string_gsub()
    -- 字符串替换
    local s1 = string.gsub("hello world", "world", "fakelua")
    if s1 ~= "hello fakelua" then return 0 end

    -- 替换次数
    local s2, cnt2 = string.gsub("a a a", " ", "_")
    if s2 ~= "a_a_a" or cnt2 ~= 2 then return 0 end

    -- 限制替换次数
    local s3, cnt3 = string.gsub("a a a a", " ", "_", 2)
    if s3 ~= "a_a_a a" or cnt3 ~= 2 then return 0 end

    -- 无匹配
    local s4, cnt4 = string.gsub("hello", "xyz", "abc")
    if s4 ~= "hello" or cnt4 ~= 0 then return 0 end

    -- 正则捕获 + $1 引用 (ECMAScript 语法)
    local s5 = string.gsub("hello world", "([a-zA-Z]+) ([a-zA-Z]+)", "$2 $1")
    if s5 ~= "world hello" then return 0 end

    -- 表替换 (ECMAScript 语法)
    local t = {a = "A", b = "B"}
    local s6 = string.gsub("a b c", "[a-z]", t)
    if s6 ~= "A B c" then return 0 end

    -- 函数替换 (ECMAScript 语法)
    local s7 = string.gsub("hello 123 world", "\\d+", function(s)
        return "[" .. s .. "]"
    end)
    if s7 ~= "hello [123] world" then return 0 end

    return 4000
end
