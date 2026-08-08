function test_pack_i_bad_arg()
    -- string.pack "i" format requires a number argument; Table is invalid per standard Lua
    string.pack("i4", {})
    return 0
end
