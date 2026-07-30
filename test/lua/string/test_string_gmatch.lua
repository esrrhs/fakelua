function test_string_gmatch()
    local s = "hello world from fakelua"
    local words = {}
    for w in string.gmatch(s, "[a-zA-Z]+") do
        words[#words + 1] = w
    end
    if #words ~= 4 then return 1 end
    if words[1] ~= "hello" then return 2 end
    if words[2] ~= "world" then return 3 end
    if words[3] ~= "from" then return 4 end
    if words[4] ~= "fakelua" then return 5 end

    -- 捕获组迭代 (ECMAScript 语法)
    local s2 = "a=1 b=2 c=3"
    local keys = {}
    local vals = {}
    for k, v in string.gmatch(s2, "([a-z])=(\\d)") do
        keys[#keys + 1] = k
        vals[#vals + 1] = v
    end
    if #keys ~= 3 then return 6 end
    if keys[1] ~= "a" or vals[1] ~= "1" then return 7 end
    if keys[2] ~= "b" or vals[2] ~= "2" then return 8 end
    if keys[3] ~= "c" or vals[3] ~= "3" then return 9 end

    -- 空匹配不应死循环
    local s3 = "abc"
    local count = 0
    for _ in string.gmatch(s3, "z*") do
        count = count + 1
        if count > 10 then return 10 end-- 安全阀
    end

    return 3000
end
