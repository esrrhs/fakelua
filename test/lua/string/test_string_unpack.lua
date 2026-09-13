package "StringUnpack"

-- 测试 string.unpack 基本（返回值顺序：值, 位置）
function test_unpack_basic()
    local s = string.pack("i4", 42)
    local val, pos = string.unpack("i4", s)
    if val ~= 42 then return 0 end
    if pos ~= 5 then return 0 end
    return 1
end

-- 测试 string.unpack 负数
function test_unpack_negative()
    local s = string.pack("i4", -100)
    local val, pos = string.unpack("i4", s)
    if val ~= -100 then return 0 end
    return 1
end

-- 测试 string.unpack 多值（返回值顺序：值1, 值2, 位置）
function test_unpack_multi()
    local s = string.pack("i4i4", 10, 20)
    local val1, val2, pos = string.unpack("i4i4", s)
    if val1 ~= 10 then return 0 end
    if val2 ~= 20 then return 0 end
    if pos ~= 9 then return 0 end
    return 1
end

-- 测试 string.unpack 带偏移
function test_unpack_offset()
    local s = string.pack("i4i4i4", 1, 2, 3)
    local val1, pos1 = string.unpack("i4", s, 1)
    if val1 ~= 1 then return 0 end
    local val2, pos2 = string.unpack("i4", s, 5)
    if val2 ~= 2 then return 0 end
    return 1
end
