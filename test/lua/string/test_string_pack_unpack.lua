function test_string_pack_unpack()
    -- === string.pack ===

    -- 基本整数打包 (小端)
    local p1 = string.pack("<i4", 42)
    if #p1 ~= 4 then return 1 end

    -- 大端 vs 小端
    local le = string.pack("<i4", 0x01020304)
    local be = string.pack(">i4", 0x01020304)
    if le == be then return 2 end
    if string.byte(le, 1) ~= 0x04 then return 3 end
    if string.byte(be, 1) ~= 0x01 then return 4 end

    -- 多值打包 (i 需要指定字节数，用 i4/I4 代替裸 i)
    local p2 = string.pack("bBi4I4jJ", -1, 255, -1, 65535, -1, 4294967295, -1, 9223372036854775807)
    if #p2 ~= 1 + 1 + 4 + 4 + 8 + 8 then return 5 end

    -- 浮点打包
    local p3 = string.pack("f", 1.5)
    if #p3 ~= 4 then return 6 end
    local p4 = string.pack("d", 3.14)
    if #p4 ~= 8 then return 7 end

    -- 字符串打包 (c[n] 固定长度)
    local p5 = string.pack("c5", "hello")
    if #p5 ~= 5 then return 8 end
    if p5 ~= "hello" then return 9 end

    -- c[n] 短字符串补零
    local p6 = string.pack("c5", "hi")
    if #p6 ~= 5 then return 10 end
    if string.byte(p6, 3) ~= 0 then return 11 end

    -- 零终止字符串
    local p7 = string.pack("z", "abc")
    if #p7 ~= 4 then return 12 end
    if string.byte(p7, 4) ~= 0 then return 13 end

    -- 多个值组合
    local p8 = string.pack("bd", 7, 2.718)
    if #p8 ~= 1 + 8 then return 14 end

    -- === string.packsize ===

    if string.packsize("i4") ~= 4 then return 20 end
    if string.packsize("bd") ~= 9 then return 21 end
    if string.packsize("c10") ~= 10 then return 22 end
    if string.packsize("z", "hello") ~= 6 then return 23 end  -- 5 chars + null
    if string.packsize("BH") ~= 3 then return 24 end
    if string.packsize("j") ~= 8 then return 25 end

    -- === string.unpack ===

    -- 基本整数解包
    local data1 = string.pack(">i4", 0x01020304)
    local a = string.unpack(">i4", data1)
    if a ~= 0x01020304 then return 30 end

    -- 小端解包
    local b = string.unpack("<i4", data1)
    if b ~= 0x04030201 then return 31 end

    -- 多值解包
    local packed2 = string.pack(">i4i4", 100, 200)
    local x, y = string.unpack(">i4i4", packed2)
    if x ~= 100 or y ~= 200 then return 32 end

    -- 带位置参数解包
    local packed3 = string.pack(">i4i4i4", 10, 20, 30)
    local m = string.unpack(">i4", packed3, 5)
    if m ~= 20 then return 33 end

    local m2, m3 = string.unpack(">i4i4", packed3, 5)
    if m2 ~= 20 or m3 ~= 30 then return 34 end

    -- 浮点解包
    local packed_f = string.pack(">f", 1.5)
    local fv = string.unpack(">f", packed_f)
    if math.abs(fv - 1.5) > 0.001 then return 35 end

    local packed_d = string.pack(">d", 3.14)
    local dv, next_pos = string.unpack(">d", packed_d)
    if math.abs(dv - 3.14) > 0.0001 then return 36 end
    if next_pos ~= 9 then return 37 end

    -- 零终止字符串解包
    local packed_z = string.pack(">z", "hello")
    local sz, after_z = string.unpack(">z", packed_z)
    if sz ~= "hello" then return 38 end
    if after_z ~= 7 then return 39 end  -- 5 chars + null + 1

    -- 返回值包含位置信息
    local packed4 = string.pack(">i4i4", 111, 222)
    local v1, v2, pos = string.unpack(">i4i4", packed4)
    if v1 ~= 111 or v2 ~= 222 then return 40 end
    if pos ~= 9 then return 41 end  -- 4+4+1

    -- 有符号 char 解包
    local packed_b = string.pack(">b", -5)
    local bv = string.unpack(">b", packed_b)
    if bv ~= -5 then return 42 end

    -- 无符号 short 解包
    local packed_H = string.pack(">H", 60000)
    local Hv = string.unpack(">H", packed_H)
    if Hv ~= 60000 then return 43 end

    -- i[n] 变长整数打包解包
    local packed_i2 = string.pack(">i2", -128)
    local i2v = string.unpack(">i2", packed_i2)
    if i2v ~= -128 then return 44 end

    -- c[n] 固定长度字符串解包
    local packed_c = string.pack(">c6", "hello!")
    local cs, cpos = string.unpack(">c6", packed_c)
    if cs ~= "hello!" then return 45 end
    if cpos ~= 7 then return 46 end

    return 5000
end
