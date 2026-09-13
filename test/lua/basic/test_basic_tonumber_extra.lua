package "BasicTonumberExtra"

-- 测试 tonumber 十六进制
function test_tonumber_hex()
    local n = tonumber("0xFF", 16)
    if n ~= 255 then return 0 end
    return 1
end

-- 测试 tonumber 二进制
function test_tonumber_binary()
    local n = tonumber("1010", 2)
    if n ~= 10 then return 0 end
    return 1
end

-- 测试 tonumber 八进制
function test_tonumber_octal()
    local n = tonumber("77", 8)
    if n ~= 63 then return 0 end
    return 1
end

-- 测试 tonumber 自动检测十六进制
function test_tonumber_auto_hex()
    local n = tonumber("0x1A")
    if n ~= 26 then return 0 end
    return 1
end

-- 测试 tonumber 带符号
function test_tonumber_negative()
    local n = tonumber("-100")
    if n ~= -100 then return 0 end
    return 1
end

-- 测试 tonumber 浮点数
function test_tonumber_float()
    local n = tonumber("3.14")
    if n == nil then return 0 end
    return 1
end

-- 测试 tonumber 无效输入
function test_tonumber_invalid()
    local n = tonumber("abc")
    if n ~= nil then return 0 end
    return 1
end

-- 测试 tonumber 空字符串
function test_tonumber_empty()
    local n = tonumber("")
    if n ~= nil then return 0 end
    return 1
end

-- 测试 tonumber 空白字符串
function test_tonumber_whitespace()
    local n = tonumber("   ")
    if n ~= nil then return 0 end
    return 1
end

-- 测试 tonumber 越界 base
function test_tonumber_bad_base()
    local n = tonumber("101", 37)
    if n ~= nil then return 0 end
    return 1
end

-- 测试 tonumber base < 2
function test_tonumber_base_too_small()
    local n = tonumber("101", 1)
    if n ~= nil then return 0 end
    return 1
end
