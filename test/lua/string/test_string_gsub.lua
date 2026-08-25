function test_string_gsub()
    -- 字符串替换
    local s1 = string.gsub("hello world", "world", "fakelua")
    if s1 ~= "hello fakelua" then return 0 end

    -- 替换次数
    local s2, cnt2 = string.gsub("a a a", " ", "_")
    if s2 ~= "a_a_a" or cnt2 ~= 2 then return 0 end

    -- 限制替换次数
    local s3, cnt3 = string.gsub("a a a a", " ", "_", "2")
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

    -- 函数替换 + 捕获组正确传递测试
    local s7 = string.gsub("hello 123 world", "(\\d+)", function(cap1)
        return "[" .. cap1 .. "]"
    end)
    if s7 ~= "hello [123] world" then return 0 end

    -- 表超过 8 个键会 rehash：以前只扫 quick_data_，k9 及之后替换失败
    local gsub_big = {}
    local gsub_i = 1
    while gsub_i <= 15 do
        gsub_big["k" .. gsub_i] = "v" .. gsub_i
        gsub_i = gsub_i + 1
    end
    local s8 = string.gsub("k1 k9 k15 kx", "k\\d+", gsub_big)
    if s8 ~= "v1 v9 v15 kx" then return 0 end

    -- 捕获组超过旧的 16 槽上限时，第 17 个参数会被丢掉
    local s9 = string.gsub("abcdefghijklmnopq", "(.)(.)(.)(.)(.)(.)(.)(.)(.)(.)(.)(.)(.)(.)(.)(.)(.)", function(g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11, g12, g13, g14, g15, g16, g17)
        if g17 ~= "q" then return "bad" end
        return "OK"
    end)
    if s9 ~= "OK" then return 0 end

    -- 捕获组少于替换函数形参时，缺的必须是 nil，不能读垃圾寄存器
    local s10 = string.gsub("ab", "(a)(b)", function(g1, g2, g3)
        if g3 ~= nil then return "bad" end
        if g1 ~= "a" or g2 ~= "b" then return "bad" end
        return "OK"
    end)
    if s10 ~= "OK" then return 0 end

    return 4000
end
