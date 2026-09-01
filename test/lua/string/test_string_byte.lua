package "StringByte"

-- 测试 string.byte 指定范围
function test_byte_range()
    local s = "Hello"
    local b1, b2, b3 = string.byte(s, 1, 3)
    if b1 ~= 72 or b2 ~= 101 or b3 ~= 108 then return 0 end
    return 1
end

-- 测试 string.byte 负数索引
function test_byte_negative_index()
    local s = "Hello"
    local b = string.byte(s, -1)
    if b ~= 111 then return 0 end
    return 1
end

-- 测试 string.byte 单字节
function test_byte_single()
    local s = "ABC"
    local b = string.byte(s, 2)
    if b ~= 66 then return 0 end
    return 1
end
