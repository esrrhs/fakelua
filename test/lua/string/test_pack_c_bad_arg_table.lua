function test_pack_c_bad_tbl()
    -- string.pack "c" format requires a string argument; Table is invalid per standard Lua
    local t = {1, 2, 3}
    string.pack("c5", t)
    return 0
end
