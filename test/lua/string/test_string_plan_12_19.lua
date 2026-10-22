function test_plan_12_19(expected_int_size, expected_long_size)
    -- 12: bare i/I follows sizeof(int)
    if string.packsize("i") ~= expected_int_size or string.packsize("I") ~= expected_int_size then return 1 end
    if #string.pack("i", 0) ~= expected_int_size or #string.pack("I", 0) ~= expected_int_size then return 2 end
    local u12, p12 = string.unpack("i", string.pack("i", 12345))
    if u12 ~= 12345 or p12 ~= expected_int_size + 1 then return 3 end

    -- 13: l/L is sizeof(long)
    local l_sz = string.packsize("l")
    if l_sz ~= expected_long_size or string.packsize("L") ~= expected_long_size then return 4 end
    if #string.pack("l", 0) ~= expected_long_size or #string.pack("L", 0) ~= expected_long_size then return 5 end
    local ul13, pl13 = string.unpack("l", string.pack("l", -999999))
    if ul13 ~= -999999 or pl13 ~= expected_long_size + 1 then return 6 end

    -- 14 & 16: alignment is min(maxalign, item size), X aligns according to next item, x emits 1 zero byte
    -- Default alignment min(1, 4) is 1. With !4, min(4, 4) is 4.
    if string.packsize("!4 b i4") ~= 8 then return 7 end
    if #string.pack("!4 b i4", 1, 2) ~= 8 then return 8 end
    -- x emits one zero byte
    local px = string.pack("x")
    if #px ~= 1 or string.byte(px, 1) ~= 0 then return 9 end
    if string.packsize("x") ~= 1 then return 10 end
    -- X with next item
    if string.packsize("!4 b Xi4") ~= 4 then return 11 end
    if #string.pack("!4 b Xi4", 1) ~= 4 then return 12 end
    -- unpack x skips 1 byte, unpack X skips alignment padding to next item
    local ux_val, ux_pos = string.unpack("!4 b Xi4", string.pack("!4 b Xi4", 65))
    if ux_val ~= 65 or ux_pos ~= 5 then return 13 end
    local ux_b, ux_bpos = string.unpack("x b", "\0B")
    if ux_b ~= 0x42 or ux_bpos ~= 3 then return 14 end

    -- 15: pack integer range checking, signed and unsigned
    local ok_ov1 = pcall(function() string.pack("b", 128) end)
    if ok_ov1 then return 15 end
    local ok_ov2 = pcall(function() string.pack("b", -129) end)
    if ok_ov2 then return 16 end
    local ok_ov3 = pcall(function() string.pack("B", -1) end)
    if ok_ov3 then return 17 end
    local ok_ov4 = pcall(function() string.pack("B", 256) end)
    if ok_ov4 then return 18 end
    local ok_ov5 = pcall(function() string.pack("h", 32768) end)
    if ok_ov5 then return 19 end
    local ok_ov6 = pcall(function() string.pack("H", -1) end)
    if ok_ov6 then return 20 end
    local ok_ov7 = pcall(function() string.pack("i1", 128) end)
    if ok_ov7 then return 21 end
    local ok_ov8 = pcall(function() string.pack("I1", 256) end)
    if ok_ov8 then return 22 end

    -- 17: z pack rejects embedded NUL; z unpack errors if no terminator; empty unpack format returns next pos; short input errors
    local ok_z_null = pcall(function() string.pack("z", "hello\0world") end)
    if ok_z_null then return 23 end
    local ok_z_unterminated = pcall(function() string.unpack("z", "unterminated") end)
    if ok_z_unterminated then return 24 end
    -- empty unpack format returns next position
    local emp_pos = string.unpack("", "abc", 2)
    if emp_pos ~= 2 then return 25 end
    -- short input errors in unpack
    local ok_short1 = pcall(function() string.unpack("i4", "ab") end)
    if ok_short1 then return 26 end
    local ok_short2 = pcall(function() string.unpack("x", "") end)
    if ok_short2 then return 27 end

    -- 18: string.byte on empty string validates optional indexes then returns 0 values
    local b_cnt = select("#", string.byte(""))
    if b_cnt ~= 0 then return 28 end
    local b_cnt2 = select("#", string.byte("", 1, 1))
    if b_cnt2 ~= 0 then return 29 end
    -- validates invalid optional index type
    local ok_byte_bad = pcall(function() string.byte("", "not_a_num") end)
    if ok_byte_bad then return 30 end

    -- 19: string.format %q accepts nil/bool/number/string and emits valid Lua literals
    if string.format("%q", nil) ~= "nil" then return 31 end
    if string.format("%q", true) ~= "true" then return 32 end
    if string.format("%q", false) ~= "false" then return 33 end
    if string.format("%q", 12345) ~= "12345" then return 34 end
    if string.format("%q", 1.5) ~= "0x1.8p+0" then return 35 end
    -- control chars, CR, NUL and decimal padding if followed by digit
    if string.format("%q", "a\0b") ~= "\"a\\0b\"" then return 36 end
    if string.format("%q", "a\0" .. "1") ~= "\"a\\0001\"" then return 37 end
    if string.format("%q", "a\rb") ~= "\"a\\13b\"" then return 38 end
    if string.format("%q", "a\r" .. "1") ~= "\"a\\0131\"" then return 39 end
    if string.format("%q", "hello\nworld") ~= "\"hello\\\nworld\"" then return 40 end

    return 1900
end
