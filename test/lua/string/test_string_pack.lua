package "StringPack"

-- 测试 string.pack i1/i2/i4/I1/I2/I4
function test_pack_various_sizes()
    local s1 = string.pack("i4", -1)
    if #s1 ~= 4 then return 0 end
    local s2 = string.pack("I4", 0xFFFFFFFF)
    if #s2 ~= 4 then return 0 end
    local s3 = string.pack("i1", 127)
    if #s3 ~= 1 then return 0 end
    local s4 = string.pack("i2", 256)
    if #s4 ~= 2 then return 0 end
    return 1
end

-- 测试 string.pack b/B/h/H/l/L
function test_pack_integers()
    local s1 = string.pack("b", -1)
    if #s1 ~= 1 then return 0 end
    local s2 = string.pack("B", 255)
    if #s2 ~= 1 then return 0 end
    local s3 = string.pack("h", -256)
    if #s3 ~= 2 then return 0 end
    local s4 = string.pack("H", 65535)
    if #s4 ~= 2 then return 0 end
    return 1
end

-- 测试 string.pack j/J (int64/uint64)
function test_pack_int64()
    local s1 = string.pack("j", -1)
    if #s1 ~= 8 then return 0 end
    local s2 = string.pack("J", 0)
    if #s2 ~= 8 then return 0 end
    return 1
end

-- 测试 string.pack f/d (float/double)
function test_pack_float_double()
    local s1 = string.pack("f", 1.5)
    if #s1 ~= 4 then return 0 end
    local s2 = string.pack("d", 1.5)
    if #s2 ~= 8 then return 0 end
    return 1
end

-- 测试 string.pack z (零结尾字符串)
function test_pack_z()
    local s = string.pack("z", "hello")
    if #s ~= 6 then return 0 end
    return 1
end

-- 测试 string.pack c (固定长度字符串)
function test_pack_c()
    local s = string.pack("c5", "hi")
    if #s ~= 5 then return 0 end
    return 1
end

-- 测试 string.pack X (填充字节)
function test_pack_X()
    local s = string.pack("Xi4", 42)
    if #s ~= 5 then return 0 end
    return 1
end

-- 测试 string.pack < > (字节序)
function test_pack_endian()
    local s1 = string.pack("<i4", 1)
    if #s1 ~= 4 then return 0 end
    local s2 = string.pack(">i4", 1)
    if #s2 ~= 4 then return 0 end
    return 1
end

-- 测试 string.pack ! (对齐)
function test_pack_align()
    local s = string.pack("!4Xi4", 42)
    if #s ~= 8 then return 0 end
    return 1
end

-- 测试 string.packsize 各种格式
function test_packsize_various()
    if string.packsize("i4") ~= 4 then return 0 end
    if string.packsize("I4") ~= 4 then return 0 end
    if string.packsize("j") ~= 8 then return 0 end
    if string.packsize("f") ~= 4 then return 0 end
    if string.packsize("d") ~= 8 then return 0 end
    return 1
end
