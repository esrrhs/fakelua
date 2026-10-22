function test_string_byte_empty_count()
    return select("#", string.byte(""))
end

function test_string_pack_lua54(expected_int_size, expected_long_size)
    if #string.pack("i", 42) ~= expected_int_size then return 1 end
    if string.packsize("I") ~= expected_int_size then return 2 end
    if #string.pack("l", 42) ~= expected_long_size then return 3 end
    if string.packsize("L") ~= expected_long_size then return 4 end
    if #string.pack("x") ~= 1 then return 5 end
    if string.packsize("Xb") ~= 0 then return 6 end
    if string.unpack("", "hi") ~= 1 then return 7 end
    return 0
end

function test_string_byte_empty_bad_index()
    return string.byte("", true)
end

function test_string_pack_signed_overflow()
    return string.pack("b", 200)
end

function test_string_pack_unsigned_overflow()
    return string.pack("I1", 256)
end

function test_string_pack_z_with_zero()
    return string.pack("z", "a\0b")
end

function test_string_unpack_unfinished_z()
    return string.unpack("z", "abc")
end

function test_string_packsize_variable()
    return string.packsize("z")
end

function test_string_packsize_s_variable()
    return string.packsize("s")
end

function test_string_pack_bad_option()
    return string.pack("y", 1)
end

function test_string_pack_bad_align()
    return string.pack("!17i4", 1)
end

function test_string_packsize_padding()
    -- "!4" 对齐: b(1) + 填充(3) + i4(4) = 8；X 按下一个 i4 补齐到 4 的倍数
    if string.packsize("!4 b i4") ~= 8 then return 1 end
    if string.packsize("!4 b Xi4") ~= 4 then return 2 end
    if string.packsize("<i2 >i2 =i2") ~= 6 then return 3 end
    return 0
end
