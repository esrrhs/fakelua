function test_string_format()
    local s1 = string.format("hello %s %d", "world", 100)
    if s1 ~= "hello world 100" then return 0 end

    local s2 = string.format("%.2f", 3.14159)
    if s2 ~= "3.14" then return 0 end

    local s3 = string.format("%q", "foo\"bar")
    if s3 ~= "\"foo\\\"bar\"" then return 0 end

    -- 超长字符串(>1024 字节)与大 Padding 宽度格式化验证，确保无缓冲截断
    local long_str = string.rep("a", 2000)
    local s4 = string.format("prefix_%s_suffix", long_str)
    if string.len(s4) ~= 2014 or string.sub(s4, 1, 7) ~= "prefix_" or string.sub(s4, -7) ~= "_suffix" then
        return 0
    end

    local s5 = string.format("%3000s", "hello")
    if string.len(s5) ~= 3000 then
        return 0
    end

    -- 验证 %c, %x, %f 等的数字字符串与浮点数隐式转换
    if string.format("%c", "65") ~= "A" then return 0 end
    if string.format("%c", 65.0) ~= "A" then return 0 end
    if string.format("%x", "255") ~= "ff" then return 0 end
    if string.format("%.2f", "3.14") ~= "3.14" then return 0 end

    return 600
end
