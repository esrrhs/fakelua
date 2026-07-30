function test_string_match()
    local s = "hello world 123"

    -- 无捕获组：返回整个匹配
    local m1 = string.match(s, "[a-zA-Z]+")
    if m1 ~= "hello" then return 1 end

    -- 单个捕获组
    local m2 = string.match(s, "([a-zA-Z]+) ([a-zA-Z]+)")
    if m2 ~= "hello" then return 2 end

    -- 多个捕获组
    local a, b = string.match(s, "([a-zA-Z]+) ([a-zA-Z]+)")
    if a ~= "hello" or b ~= "world" then return 3 end

    -- 数字捕获 (ECMAScript 语法用 \d)
    local num = string.match(s, "\\d+")
    if num ~= "123" then return 4 end

    -- 带起始位置
    local m3 = string.match(s, "[a-zA-Z]+", 7)
    if m3 ~= "world" then return 5 end

    -- 找不到
    local m4 = string.match(s, "xyz")
    if m4 ~= nil then return 6 end

    return 2000
end
