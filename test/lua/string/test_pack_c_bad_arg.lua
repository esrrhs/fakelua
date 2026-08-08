function test_pack_c_bad_arg()
    -- string.pack "c" format requires a string argument; Bool is invalid per standard Lua
    string.pack("c5", true)
    return 0
end
