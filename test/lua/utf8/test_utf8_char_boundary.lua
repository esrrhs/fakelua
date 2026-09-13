function test_utf8_char_boundary()
    -- 1. 码点 0（NULL 字符）
    local s0 = utf8.char(0)
    if #s0 ~= 1 or string.byte(s0, 1) ~= 0 then return 0 end

    -- 2. 边界码点：0x7F（1 字节最大）
    local s1 = utf8.char(0x7F)
    if #s1 ~= 1 then return 0 end

    -- 3. 边界码点：0x80（2 字节最小）
    local s2 = utf8.char(0x80)
    if #s2 ~= 2 then return 0 end

    -- 4. 边界码点：0x7FF（2 字节最大）
    local s3 = utf8.char(0x7FF)
    if #s3 ~= 2 then return 0 end

    -- 5. 边界码点：0x800（3 字节最小）
    local s4 = utf8.char(0x800)
    if #s4 ~= 3 then return 0 end

    -- 6. 边界码点：0xFFFF（3 字节最大）
    local s5 = utf8.char(0xFFFF)
    if #s5 ~= 3 then return 0 end

    -- 7. 边界码点：0x10000（4 字节最小）
    local s6 = utf8.char(0x10000)
    if #s6 ~= 4 then return 0 end

    -- 8. 边界码点：0x10FFFF（4 字节最大，Unicode 最大有效码点）
    local s7 = utf8.char(0x10FFFF)
    if #s7 ~= 4 then return 0 end

    -- 9. 无效码点：负数
    if utf8.char(-1) ~= nil then return 0 end

    -- 10. 无效码点：> 0x10FFFF
    if utf8.char(0x110000) ~= nil then return 0 end

    -- 11. 无效码点：代理区 0xD800-0xDFFF
    if utf8.char(0xD800) ~= nil then return 0 end
    if utf8.char(0xDFFF) ~= nil then return 0 end

    -- 12. 混合有效和无效码点（第一个无效即返回 nil）
    if utf8.char(65, 0x110000, 66) ~= nil then return 0 end

    return 5000
end
