function test_string_find()
    -- 基本查找
    local s = "hello world"
    local a, b = string.find(s, "world")
    if a ~= 7 or b ~= 11 then return 0 end

    -- 找不到
    local c = string.find(s, "xyz")
    if c ~= nil then return 0 end

    -- 带起始位置（从位置5开始找 'o'，第一个 'o' 在位置5）
    local d, e = string.find(s, "o", 5)
    if d ~= 5 or e ~= 5 then return 1 end

    -- 负起始位置（从位置 11-5+1 = 7 开始，'o' 在位置 8）
    local f, g = string.find(s, "o", -5)
    if f ~= 8 or g ~= 8 then return 2 end

    -- plain 模式（纯子串查找）
    local h, i = string.find(s, "o", 1, true)
    if h ~= 5 or i ~= 5 then return 3 end

    -- 正则捕获组 (ECMAScript 语法)
    local j, k, cap = string.find(s, "([a-zA-Z]+) ([a-zA-Z]+)")
    if j ~= 1 or k ~= 11 or cap ~= "hello" then return 4 end

    -- 多个捕获组
    local l, m, c1, c2 = string.find(s, "([a-zA-Z]+) ([a-zA-Z]+)")
    if l ~= 1 or m ~= 11 or c1 ~= "hello" or c2 ~= "world" then return 5 end

    return 1000
end
