package "StringFormatCases"

-- 测试 string.format %d 整数
function test_format_d()
    local s = string.format("%d", 42)
    if s ~= "42" then return 0 end
    return 1
end

-- 测试 string.format %d 负整数
function test_format_d_negative()
    local s = string.format("%d", -123)
    if s ~= "-123" then return 0 end
    return 1
end

-- 测试 string.format %u 无符号整数
function test_format_u()
    local s = string.format("%u", 255)
    if s ~= "255" then return 0 end
    return 1
end

-- 测试 string.format %x 十六进制
function test_format_x()
    local s = string.format("%x", 255)
    if s ~= "ff" then return 0 end
    return 1
end

-- 测试 string.format %X 大写十六进制
function test_format_X()
    local s = string.format("%X", 255)
    if s ~= "FF" then return 0 end
    return 1
end

-- 测试 string.format %o 八进制
function test_format_o()
    local s = string.format("%o", 64)
    if s ~= "100" then return 0 end
    return 1
end

-- 测试 string.format %f 浮点
function test_format_f()
    local s = string.format("%.2f", 3.14159)
    if s ~= "3.14" then return 0 end
    return 1
end

-- 测试 string.format %e 科学计数法
function test_format_e()
    local s = string.format("%.2e", 12345.0)
    if not string.find(s, "e") then return 0 end
    return 1
end

-- 测试 string.format %s 字符串
function test_format_s()
    local s = string.format("%s", "hello")
    if s ~= "hello" then return 0 end
    return 1
end

-- 测试 string.format %s 带宽度
function test_format_s_width()
    local s = string.format("%10s", "hi")
    if #s ~= 10 then return 0 end
    return 1
end

-- 测试 string.format %c 字符
function test_format_c()
    local s = string.format("%c", 65)
    if s ~= "A" then return 0 end
    return 1
end

-- 测试 string.format %q 引号
function test_format_q()
    local s = string.format("%q", 'hello')
    if s ~= '"hello"' then return 0 end
    return 1
end

-- 测试 string.format 多参数
function test_format_multi()
    local s = string.format("%s=%d", "x", 10)
    if s ~= "x=10" then return 0 end
    return 1
end

-- 测试 string.format 转义 %%
function test_format_percent()
    local s = string.format("100%%")
    if s ~= "100%" then return 0 end
    return 1
end

-- 测试 string.format %p 指针
function test_format_p()
    local s = string.format("%p", 0)
    if s ~= "0x0" then return 0 end
    return 1
end
