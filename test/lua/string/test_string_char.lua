package "StringChar"

-- 测试 string.char 多参数
function test_char_multi()
    local s = string.char(72, 101, 108, 108, 111)
    if s ~= "Hello" then return 0 end
    return 1
end

-- 测试 string.char 单参数
function test_char_single()
    local s = string.char(65)
    if s ~= "A" then return 0 end
    return 1
end

-- 测试 string.char 边界值
function test_char_boundary()
    local s1 = string.char(0)
    if #s1 ~= 1 then return 0 end
    local s2 = string.char(255)
    if #s2 ~= 1 then return 0 end
    return 1
end

-- 测试 string.char 越界（应报错）
function test_char_out_of_range()
    local ok, err = pcall(function() string.char(256) end)
    if ok then return 0 end
    return 1
end

-- 测试 string.char 负数（应报错）
function test_char_negative()
    local ok, err = pcall(function() string.char(-1) end)
    if ok then return 0 end
    return 1
end
