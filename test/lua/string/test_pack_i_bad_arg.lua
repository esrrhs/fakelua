function test_pack_i_bad_arg()
    -- string.pack "i" format requires a number argument; Table is invalid per standard Lua
    string.pack("i4", {})
    return 0
end

function test_pack_i16_throw()
    string.pack("i16", 1)
    return 0
end

function test_pack_c_huge_throw()
    string.pack("c9999999999", "x")
    return 0
end

function test_format_huge_width()
    string.format("%2147483647s", "x")
    return 0
end

function test_format_n_throw()
    string.format("%n", 1)
    return 0
end
